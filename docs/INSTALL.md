# Installing the widescreen mod

*(Español: [INSTALL.es.md](INSTALL.es.md))*

## Before you start

You need a PSXRecomp project for Street Fighter EX2 Plus that already builds and runs on your
machine — the folder that contains `CMakeLists.txt`, `game.toml` and your `disc/`. This
repository does not provide the game, the disc image or the framework.

That project is
[strider973/Street-Fighter-EX2-Plus-Recompiled](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled),
built on the [PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp) framework (clone it
with `--recurse-submodules`). Two things it needs that this mod cannot provide: **your own disc
dump**, and **a retail SCPH-1001 BIOS** — that project sets `openbios = false`, so the bundled
OpenBIOS will not do. Build it and get it booting first; this mod is a change to that build.

The mod is tied to the NTSC-U release, `SLUS-01105`. The plugin verifies every original
instruction word before writing anything, so on a different build it simply does nothing
rather than corrupting the game.

## 1. Install the files

Get the kit: download `sfex2p.widescreen-1.0.0.zip` from
[the releases page](https://github.com/jajdp/sfex2p-widescreen/releases/latest) and extract it,
or clone this repository — the contents are the same. Keep the layout as it is: the installer
reads the plugin and the package from its own folder, relative to itself. Then, from that
folder:

```sh
python tools/apply_widescreen.py <path-to-game-project-root>
```

Add `--dry-run` first if you want to see the changes without writing them.

The script makes four changes, each guarded by a signature so that running it twice does
nothing:

| What | Where |
|---|---|
| The plugin source | `sfex2p_widescreen.c`, copied to the project root |
| The mod package | `mods/packages/sfex2p.widescreen/1.0.0/manifest.toml` |
| Build registration | `CMakeLists.txt`: the plugin added to `CODEGEN_SETUP_SOURCES`, and a post-build step that stages `mods/packages/` next to the executable |
| Game configuration | `game.toml`: a `[widescreen]` block with `gte_game_mode = true` and `nw_phase_backdrop = false` |

Writes are atomic and keep each file's existing line endings. Nothing is deleted.

### Why `nw_phase_backdrop = false`

That framework option stretches the 2D backdrop over the wide frame. This mod widens the
game's own backdrop loop instead, so leaving both on duplicates the edge and leaves vertical
seams in the margins. Set it to `true` only if you remove the plugin and want the cheap
approximation.

## 2. Rebuild the game

Build the project as you normally do. The plugin is compiled into the executable — that is
the whole reason this step exists.

## 3. Turn it on

Launch the game. In the launcher, under **Mods**, the group **Visual** now lists
**Widescreen (16:9)**, enabled by default. Press **PLAY**.

![The launcher's Mods list with the feature enabled](images/pc-lanzador-mods.png)

You should see:

- menus, logos and character select in 4:3 with side bars;
- the fight itself filling the width, with stage visible all the way to both edges;
- no vertical strips, no frozen pixels and no seams in the margins — not even in the attract
  demo that plays right after boot.

## 4. Turning it off

Uncheck **Widescreen (16:9)** in the launcher: the game runs in its original 4:3 and the
plugin stays dormant. Saves are unaffected (`save_compatibility = "shared"`).

To remove it completely, delete `mods/packages/sfex2p.widescreen/` and `sfex2p_widescreen.c`
from the project, undo the two `CMakeLists.txt` additions and the `[widescreen]` block in
`game.toml` (each is marked with the comment `sfex2p.widescreen (Recompilaciones)`), and
rebuild.

## Consoles and other UWP packaging

On a UWP build (for example an Xbox in Developer Mode) the mod package is not installed
separately: it is compiled and staged into the application package together with the
executable, so it travels inside it. Installing a new package version is what updates the mod.

## If something looks wrong

| Symptom | Cause |
|---|---|
| The game is still 4:3 with the feature on | The plugin was not compiled in. Check that `sfex2p_widescreen.c` is in `CODEGEN_SETUP_SOURCES` and that you rebuilt. |
| The feature does not appear in the launcher | The package was not staged next to the executable. Check the post-build step, and that `mods/packages/sfex2p.widescreen/1.0.0/manifest.toml` exists. |
| 16:9, but the margins show repeated strips | The backdrop widen is not active: the plugin's GPU-DMA allocation did not land where the patched instruction expects, or the game build is not `SLUS-01105`. The mod falls back to 17 columns on purpose rather than drawing garbage. |
| Launcher refuses to start the game | Two features claiming the same plugin id, usually a leftover copy of an older package. Keep one `sfex2p.widescreen` package only. |
