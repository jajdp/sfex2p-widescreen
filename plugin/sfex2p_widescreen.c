/* Street Fighter EX2 Plus — Widescreen (16:9), the game-owned plugin.
 * Recompilaciones, 2026-10-03 (stage backdrop chain fix: 2026-10-07).
 * https://github.com/jajdp/sfex2p-widescreen — PolyForm Noncommercial 1.0.0.
 *
 * psxrecomp makes widescreen mod-owned: the generic [video] aspect_ratio is
 * clamped to 4:3 unless a trusted plugin asks for a wider display aspect
 * before the renderer starts. This game-owned plugin does that when its mod
 * feature (mods/packages/sfex2p.widescreen) is enabled; the [widescreen] block
 * of game.toml tunes the native-wide mode for this game.
 *
 * Real 16:9 stage backdrop (Recompilaciones, 2026-10-03). The stage's 2D
 * backdrop layer (func_80137AD0, called once per frame) draws exactly 17
 * columns of 32x32 tiles (POLY_FT4, 16x32 texels each) from
 * x = -(scrollX & 31) across the 512-px screen, reading a 64-column x
 * 14-row tile map, into two pre-linked packet buffers of 153 entries at
 * 0x800D0000 (other game data follows them). In 16:9 the revealed margins
 * (85 px per side) were never drawn, so they kept stale pixels from earlier
 * frames. This plugin widens that loop by
 * BG_EXTRA_COLS columns per side (the map index wraps at 64 columns, as the
 * game's own scroll does) and moves the packets to two larger buffers
 * (9 rows x 23 columns) in the GPU-DMA mod aperture. The patched function
 * runs through the runtime's executable-RAM path. The extra tiles land
 * outside the 4:3 draw area, so the canonical image is unchanged. */
#include "mod_plugins.h"

#define BG_EXTRA_COLS 3u                          /* per side: ceil(85 / 32) */
#define BG_ROWS       9u                          /* the game's row limit */
#define BG_COLS       (17u + 2u * BG_EXTRA_COLS)
#define BG_PRIMS      (BG_ROWS * BG_COLS)         /* 207, against the 153 the game links by itself */
#define BG_PRIM_BYTES 40u                         /* tag + 9 words */
#define BG_STRIDE     0x4000u                     /* per buffer: a power of two >= 207 * 40 */
#define BG_BASE       0x80F00000u                 /* the aperture's first allocation (lui s7, 0x80F0) */
#define BG_LAST_TABLE 0x801E4CE8u                 /* the game's own table: last packet used, one slot per buffer */

typedef struct { uint32_t addr, orig, patched; } BgCodeWord;

/* func_80137AD0, SLUS-01105 (USA). Every original word is checked before any is written. */
static const BgCodeWord k_bg_code[] = {
    { 0x80137AF0u, 0x3C17800Du, 0x3C1780F0u },  /* lui s7, 0x800D -> 0x80F0: the new buffers */
    { 0x80137B14u, 0x00041040u, 0x00041B80u },  /* v1 = idx * 6120 -> sll v1, a0, 14 */
    { 0x80137B18u, 0x00441021u, 0x00000000u },
    { 0x80137B1Cu, 0x00021A00u, 0x00000000u },
    { 0x80137B20u, 0x00621823u, 0x00000000u },
    { 0x80137B24u, 0x000318C0u, 0x00000000u },
    { 0x80137BE8u, 0x00000000u, 0x2401FFA0u },  /* delay-slot nop -> addiu at, zero, -96 */
    { 0x80137BF8u, 0x001E4023u, 0x003E4023u },  /* t0 = -fp -> subu t0, at, fp (3 columns left) */
    { 0x80137C00u, 0x001E4023u, 0x003E4023u },
    { 0x80137C04u, 0x02807021u, 0x268EFFFDu },  /* t6 = s4 -> addiu t6, s4, -3 (map column) */
    { 0x80137E88u, 0x29A20011u, 0x29A20017u },  /* slti v0, t5, 17 -> 23 columns */
    { 0x80137EB4u, 0x00041840u, 0x00041380u },  /* v0 = idx * 6120 -> sll v0, a0, 14 */
    { 0x80137EB8u, 0x00641821u, 0x00000000u },
    { 0x80137EBCu, 0x00031200u, 0x00000000u },
    { 0x80137EC0u, 0x00431023u, 0x00000000u },
    { 0x80137EC4u, 0x000210C0u, 0x00000000u },
    { 0x80137ED4u, 0x24421950u, 0x24422058u },  /* "last == start + 162*40" (never true) -> + 207*40 */
    { 0x80137EF8u, 0x00021840u, 0x00021380u },  /* v0 = idx * 6120 -> sll v0, v0, 14 */
    { 0x80137EFCu, 0x00621821u, 0x00000000u },
    { 0x80137F00u, 0x00031200u, 0x00000000u },
    { 0x80137F04u, 0x00431023u, 0x00000000u },
    { 0x80137F08u, 0x000210C0u, 0x00000000u },
};
#define BG_CODE_WORDS (sizeof(k_bg_code) / sizeof(k_bg_code[0]))

static uint32_t s_bg_base = 0u;                   /* 0 = backdrop widen unavailable */

/* Pre-link one buffer like the game links its own: each 9-word packet's tag points to the
 * next; AddPrims (0x801357A4) rewrites the last used one every frame, and the function's
 * entry (0x801C54B8) relinks it on the buffer's next use. */
static void bg_link_buffer(uint32_t buf) {
    for (uint32_t k = 0; k < BG_PRIMS; k++) {
        const uint32_t p = buf + k * BG_PRIM_BYTES;
        const uint32_t next = (k + 1u < BG_PRIMS) ? (p + BG_PRIM_BYTES) : 0x00FFFFFFu;
        psx_mod_write_word(p, (9u << 24) | (next & 0x00FFFFFFu));
    }
}

static void sfex2p_widescreen_activate(void) {
    (void)psx_mod_set_fixed_display_aspect(16u, 9u);
    const uint32_t base = psx_mod_alloc_gpu_dma_memory(2u * BG_STRIDE, 4096u);
    if (base != BG_BASE) return;                  /* not where the patched lui points: keep 17 columns */
    s_bg_base = base;
    bg_link_buffer(base);
    bg_link_buffer(base + BG_STRIDE);
}

/* The game's executable is loaded (and may be reloaded or restored from a state) after
 * activation: once per VBlank, patch the backdrop function when its original code is in
 * RAM, and relink a buffer whose tags were cleared. Boot loads the executable long before
 * the first stage, so the function is never mid-run when it is patched. */
static void sfex2p_widescreen_vblank(void) {
    if (!s_bg_base) return;
    /* Exactly one link breaks per frame, in no fixed place: AddPrims rewrites the tag of the LAST packet the
     * backdrop used, so the block chains into the rest of the ordering table (characters, HUD), and the game
     * repairs that link on the next frame — but only while its own table still remembers it. On a frame that
     * draws no backdrop at all (the transitions around a match) the table is cleared to -1 and the break is
     * forgotten: a stale jump into an ordering table of a past frame stays in the middle of the chain, and the
     * GPU stops there, leaving the lower part of the stage black.
     * ⚠ Relinking the whole buffer is NOT the fix. The link the table points at is what joins the backdrop to
     * everything else, so rewriting it leaves the game with the stage and nothing else: no characters, no title
     * letters. Repair only the stale jumps BEFORE that packet, which belong to frames already gone. */
    for (uint32_t b = 0; b < 2u; b++) {
        const uint32_t buf = s_bg_base + b * BG_STRIDE;
        const uint32_t fin = buf + BG_PRIMS * BG_PRIM_BYTES;
        if (psx_mod_read_word(buf) == 0u) { bg_link_buffer(buf); continue; }
        const uint32_t last = psx_mod_read_word(BG_LAST_TABLE + b * 4u);
        if (last < buf || last >= fin || ((last - buf) % BG_PRIM_BYTES) != 0u) continue;
        const uint32_t upto = (last - buf) / BG_PRIM_BYTES;
        for (uint32_t k = 0; k < upto; k++) {
            const uint32_t p = buf + k * BG_PRIM_BYTES;
            const uint32_t next = p + BG_PRIM_BYTES;
            if ((psx_mod_read_word(p) & 0x00FFFFFFu) != (next & 0x00FFFFFFu))
                psx_mod_write_word(p, (9u << 24) | (next & 0x00FFFFFFu));
        }
    }
    /* Every original word has to be in RAM before any is written: a reload or a restored state
     * leaves the function half patched otherwise. */
    for (uint32_t i = 0; i < BG_CODE_WORDS; i++)
        if (psx_mod_read_word(k_bg_code[i].addr) != k_bg_code[i].orig) return;
    for (uint32_t i = 0; i < BG_CODE_WORDS; i++)
        psx_mod_write_code_word(k_bg_code[i].addr, k_bg_code[i].patched);
}

PSX_MOD_CONSTRUCTOR(sfex2p_widescreen_register) {
    (void)psx_mod_register_activation_plugin("sfex2p.widescreen",
                                             sfex2p_widescreen_activate);
    (void)psx_mod_register_vblank_plugin("sfex2p.widescreen",
                                         sfex2p_widescreen_vblank);
}
