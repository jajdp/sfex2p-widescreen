# How the widescreen mod works

Everything here applies to Street Fighter EX2 Plus NTSC-U (`SLUS-01105`), whose executable
loads at `0x80130000` (file offset `0x800`). The same engine powers *Street Fighter EX Plus
Alpha*, so the shapes — if not the addresses — carry over.

## 1. Widescreen is mod-owned

PSXRecomp supports a native-wide display, but it does not let a generic setting turn it on:
the `[video] aspect_ratio` option is clamped to 4:3 unless a **trusted, game-owned plugin**
asks for a different display aspect *before the renderer is created*. The reasoning is sound —
on a PS1 game, whether a wider frustum is correct at all is a per-game question, often a
per-screen one.

So the mod has two halves:

- `mods/packages/sfex2p.widescreen/…/manifest.toml` — a `format_version = 5` package whose
  single feature claims the plugin id `sfex2p.widescreen`. It carries no code: that is what
  makes the feature appear in the launcher and lets the player switch it off.
- `plugin/sfex2p_widescreen.c` — compiled into the game executable, registering an
  **activation** callback and a **VBlank** callback under that same id. Activation runs after
  the launcher commits the mod plan and before the renderer starts, which is exactly where
  `psx_mod_set_fixed_display_aspect(16, 9)` has to be called.

`game.toml` then gets a `[widescreen]` block with `gte_game_mode = true`: the fights are 3D,
so the runtime's GTE-based gameplay detector decides when the wide frustum applies, leaving
the 2D screens (logos, menus, character select, loading) in 4:3.

## 2. The problem with the margins

Going wide reveals about 85 px on each side of the 512-px screen. The 3D fills them by itself.
The **2D stage backdrop does not**: nobody draws there, so those columns keep whatever was in
the frame buffer from earlier frames — in practice, repeated vertical strips that look like a
mirrored edge.

Stretching the backdrop over the wide frame is the cheap fix (`nw_phase_backdrop = true` in
the framework), and it is visibly a stretch. The honest fix is to make the game draw more
stage.

## 3. The backdrop loop, disassembled

`func_80137AD0`, called once per frame with `s1 = 0x801E7F00`:

- `[s1+4]` points at the stage's tile map: **64 columns × 14 rows**, 8 bytes per cell, indexed
  `row + column × 14`;
- the row count is `6 − (scrollY >> 5)`, capped at 9, and the first row is
  `clamp((scrollY >> 5) + 8, 0, 13)` — the row index **saturates**;
- the column is `(scrollX >> 5 + c) & 63` — the column index **wraps**;
- the loop runs **17 columns** starting at `x = −(scrollX & 31)`, drawing `POLY_FT4` tiles of
  32×32 pixels (16×32 texels) into two pre-linked packet buffers of 153 entries at
  `0x800D0000`;
- at `0x80137C40`: **a cell whose value is 0 is not drawn at all**, and it does not consume a
  packet either. The sky is not in the map — a healthy backdrop emits far fewer than the
  maximum number of tiles. (Counting emitted packets is therefore *not* a measure of health,
  which cost one wrong diagnosis.)

## 4. Widening it

The plugin patches **22 instruction words** of that function — each one verified against its
original value before any write, all-or-nothing:

- the base register of the packet buffers (`lui s7, 0x800D` → `0x80F0`), pointing at two
  larger buffers of 9 rows × 23 columns allocated with `psx_mod_alloc_gpu_dma_memory`. The
  stock buffers cannot be grown in place: other game data follows them immediately;
- the per-buffer index arithmetic, replaced by a shift (`sll …, 14`) matching the new stride;
- the start of the loop, moved 3 columns (96 px) left, and the map column index biased by −3;
- the loop bound, `slti v0, t5, 17` → `23`;
- the buffer-full check, `start + 162×40` → `start + 207×40`.

The extra tiles land outside the 4:3 draw area, so in 4:3 the image is unchanged. If the
GPU-DMA allocation does not land at the address the patched `lui` expects, the plugin leaves
the game at 17 columns rather than scribbling.

The patched function is reached through the runtime's executable-RAM path, so the patch is
applied from the VBlank callback whenever the original code is seen in RAM — the game loads
its executable long after the plugin activates, and may reload it or restore it from a state.

## 5. The hard part: the orphan link

With 23 columns the backdrop block became long enough to expose a bug in the game's own
ordering-table bookkeeping. The symptom: in the very first attract-mode match after boot, the
stage appeared **half drawn** for a couple of seconds and then flickered. Turning the mod off
made it whole from the first frame.

The mechanism, which generalises to any game on this engine:

1. `AddPrims` (`0x801357A4`) chains the backdrop block into the rest of the draw list
   (characters, HUD) by writing, **into the last packet the backdrop actually used**, the link
   to the remainder of the list.
2. The game repairs that link when it next enters the function (`jal 0x801C54B8`,
   `SetNextPrim`) — but it reads which packet to repair from **its own table at
   `0x801E4CE8`**, one slot per buffer.
3. That table is cleared to −1 as soon as a frame draws no backdrop at all, which happens in
   the transitions around a match. The break is then forgotten: a stale jump into an old
   ordering table sits in the middle of the chain, and from there on the GPU walks out of the
   backdrop.

With 17 columns the loop almost never reached that packet. With 23 it did.

**The fix:** once per VBlank, walk the buffer and restore only the links *before* the packet
the game's table points at. Those belong to frames already gone.

### Two traps worth knowing

- **Do not relink the whole buffer.** The link the table points at is precisely what joins the
  backdrop to everything else. Rewriting it leaves the game showing the stage and nothing
  else — no characters, not even the title letters. This was tried twice and reverted twice.
- **Do not measure chain health by counting emitted tiles**, for the reason in §3: empty cells
  are skipped, so a healthy frame legitimately emits far fewer packets than the loop's
  maximum.

## 6. How it was measured

PSXRecomp's debug server provides `read_ram`, `set_snapshot` + `read_frame_ram` (four 128-byte
windows per frame), `gpu_frame_dump` and the `wtrace` / `rtrace` / `fntrace` tracers. One
caveat that cost time: **the tracers do not see RAM accesses made by recompiled game code**,
only I/O.

What actually solved it was reading the whole packet buffer with `read_ram` and **walking the
chain the way the GPU does**. That printed, in one line: the chain breaks at packet 68 and
jumps to `0x801FC2EC`, with 138 packets written — in the Dhalsim vs Blanka demo, with only 69
tiles reaching the screen and the lower half of the stage black.
