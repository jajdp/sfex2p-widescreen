# Street Fighter EX2 Plus — Widescreen (16:9)

A real 16:9 mod for the [PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp) static recompilation of
*Street Fighter EX2 Plus* (PlayStation, 1999, `SLUS-01105`). The fights fill a wide screen
with **stage backdrop actually drawn out to the edges** — no stretching, no mirrored strips,
no stale pixels in the margins.

*(Español: [README.es.md](README.es.md))*

What the mod changes, measured frame by frame, is in
[`docs/HOW-IT-WORKS.md`](docs/HOW-IT-WORKS.md).

## What it does

- **16:9 during the fights.** The game's own 3D gameplay is rendered native-wide: a wider
  frustum, not a stretched 4:3 image.
- **The 2D backdrop is widened, not stretched.** The stage's tile layer draws 17 columns of
  32×32 tiles in the stock game — exactly the 4:3 screen. This mod patches that loop to draw
  **23 columns** (3 extra per side) and relocates its packet buffers, so the revealed margins
  contain real stage, scrolling in step with the rest.
- **2D screens stay 4:3.** Logos, menus, character select and loading screens keep their
  original framing with side bars; only the gameplay goes wide.
- **It is a toggle.** The feature appears in the launcher under **Mods → Visual → Widescreen
  (16:9)**. Turn it off and the game is 4:3 again, with no reinstall.

## What this repository is not

It contains **no game code, no disc image, no BIOS and no built executable**. You need your
own legally dumped copy of the game and a working PSXRecomp project for it. This repository
holds the mod: a plugin source file, a mod package and the script that installs both.

## Requirements

| | |
|---|---|
| [A PSXRecomp project for Street Fighter EX2 Plus](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled) | the folder with `CMakeLists.txt` and `game.toml` |
| Your own disc dump | NTSC-U, `SLUS-01105` |
| A retail SCPH-1001 BIOS | required by that project (`openbios = false`), not by this mod |
| A C toolchain able to build that project | the plugin is compiled into the executable |
| Python 3.8+ | only to run the installer script |

## Install

Download the **[latest release](https://github.com/jajdp/sfex2p-widescreen/releases/latest)** —
a source kit, not a binary — extract it, and from that folder run:

```sh
python tools/apply_widescreen.py <path-to-game-project-root>
```

Then rebuild the game. (A clone of this repository works just the same: the kit is these files.)
Full steps, including what the script changes and how to undo it, are in
**[docs/INSTALL.md](docs/INSTALL.md)**.

Why a build step? PSXRecomp mod packages carry no native code by design: a feature that
changes the display aspect has to be a *game-owned trusted plugin*, statically linked into
the game executable. The package in `mods/` is what makes it appear — and switch off — in the
launcher; `plugin/sfex2p_widescreen.c` is what does the work.

## How it works

The short version: the plugin asks the runtime for a fixed 16:9 display aspect before the
renderer starts, allocates two larger packet buffers in the GPU-DMA mod aperture, and patches
22 instruction words of the game's backdrop function so its loop covers 23 columns instead of
17. Once per emulated VBlank it repairs the stale links that the game's own ordering-table
bookkeeping leaves behind in the longer chain.

That last part is the subtle one, and it is written up — with the measurements and the two
failed attempts — in **[docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md)**.

## Layout

```
mods/packages/sfex2p.widescreen/1.0.0/manifest.toml   the mod package (the launcher toggle)
plugin/sfex2p_widescreen.c                            the game-owned plugin (compiled in)
tools/apply_widescreen.py                             installs both into a game project
docs/INSTALL.md            how to install, verify and remove it
docs/HOW-IT-WORKS.md       the backdrop widening and the chain repair, in detail
```

## Status

Built and played on Windows and on an Xbox Series in Developer Mode: 60 FPS, complete
backdrop from the first frame of the attract demo, no seams in the margins.

Verified from scratch on 2026-10-08: installer applied to a freshly cloned game tree (exactly
the five documented changes, nothing else), rebuilt, and active at boot —
`psxrecomp: mod selected fixed display aspect 16:9`.

## Credits and license

Mod by **Recompilaciones**. Released under the
[PolyForm Noncommercial License 1.0.0](LICENSE), matching the license of the PSXRecomp
framework it plugs into.

*Street Fighter EX2 Plus* is © Capcom / Arika. This project is not affiliated with them, with
Sony, or with the PSXRecomp author, and it distributes nothing that belongs to them. The
details — exactly what is and is not included here, and how to ask for a takedown — are in
**[NOTICE.md](NOTICE.md)**. Rights holders can write to **jajdpmail@gmail.com**.
