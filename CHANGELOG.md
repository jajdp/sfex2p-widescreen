# Changelog

All notable changes to this mod. Dates are `YYYY-MM-DD`.

## [1.0.0] — 2026-10-08

First public release. Developed and tested on Windows and on an Xbox Series in Developer
Mode (60 FPS, complete backdrop, no seams).

### Added

- Native-wide 16:9 for the fights, requested by a game-owned trusted plugin before renderer
  start; 2D screens stay 4:3 with side bars.
- Real backdrop in the revealed margins: the game's own tile loop is widened from 17 to 23
  columns, with its packet buffers relocated to the GPU-DMA mod aperture. 22 instruction
  words are patched, each verified against its original value first.
- A launcher toggle (**Mods → Visual → Widescreen (16:9)**), on by default, with shared save
  compatibility.
- `tools/apply_widescreen.py`: idempotent, atomic installer for a game project, with
  `--dry-run`.

### Fixed

- **The backdrop drawn only half way** in the first attract-mode match after boot, which then
  flickered. The game's ordering-table bookkeeping forgets a link when a frame draws no
  backdrop, leaving a stale jump in the middle of the longer chain. The plugin now repairs,
  once per VBlank, only the links before the packet the game's own table points at — relinking
  the whole buffer leaves the game with the stage and nothing else. See
  [docs/HOW-IT-WORKS.md](docs/HOW-IT-WORKS.md) §5.
- Vertical seams in the margins caused by combining the widened loop with the framework's
  `nw_phase_backdrop` stretch. The installer now sets that option to `false`.

### Documentation, same day

Verified from scratch: the game cloned fresh from its own repository, the installer applied to
that untouched tree (exactly the five promised changes, nothing else), the game rebuilt, and the
feature confirmed active at boot — `mod selected fixed display aspect 16:9`. The install guide
now also says where the game project comes from and that it needs a retail SCPH-1001 BIOS.

Published as a release: `sfex2p.widescreen-1.0.0.zip`, a source kit with the plugin, the
installer and the package. There is no binary to ship — the plugin is compiled into the game
executable, which this project does not distribute.

### Installer, same day

`tools/apply_widescreen.py` no longer carries any lines copied out of the game project's own
`CMakeLists.txt`. It used to locate its two insertion points by matching those lines
literally; it now anchors on the framework's parameter name (`CODEGEN_SETUP_SOURCES`) for the
plugin entry, and simply appends the staging block at the end of the file — which is also
where it belongs, since `POST_BUILD` commands run in declaration order and the framework's own
staging clears `mods/packages` first. The result written into a game project is unchanged, and
the release archive was replaced with this version.
