# -*- coding: utf-8 -*-
"""Install the sfex2p.widescreen mod into a Street Fighter EX2 Plus (PSXRecomp) game project.

A PSXRecomp package carries no native code, so a mod that changes the display aspect has to
ship a game-owned plugin that is compiled into the game executable. This script drops that
plugin into the project, registers it in CMakeLists.txt, adds the [widescreen] block to
game.toml and installs the mod package. Then you rebuild the game.

Usage:
    python tools/apply_widescreen.py <path-to-game-project-root> [--dry-run]

The project root is the folder that holds CMakeLists.txt and game.toml.

Every change is guarded by a signature: running the script twice changes nothing. Writes are
atomic and keep each file's existing line endings. Nothing is deleted, and the stock disc
image is never touched.

Recompilaciones — https://github.com/jajdp/sfex2p-widescreen
PolyForm Noncommercial License 1.0.0
"""
import argparse
import os
import shutil
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLUGIN_SRC = os.path.join(REPO, 'plugin', 'sfex2p_widescreen.c')
PACKAGE_SRC = os.path.join(REPO, 'mods', 'packages', 'sfex2p.widescreen', '1.0.0', 'manifest.toml')
PACKAGE_REL = os.path.join('mods', 'packages', 'sfex2p.widescreen', '1.0.0', 'manifest.toml')

MARK = 'sfex2p.widescreen (Recompilaciones)'
# Markers written by earlier revisions of this installer, recognised so that a project patched
# before the public release is not reported as carrying a foreign [widescreen] block.
KNOWN_MARKS = (MARK, 'Recompilaciones (2026-10-03): 16:9')

# Where the plugin is registered. The anchor is the framework's own parameter name, not a copy
# of the project's CMakeLists.txt: the plugin entry goes on the line after it, inside the list.
CMAKE_ANCHOR = 'CODEGEN_SETUP_SOURCES'
CMAKE_PLUGIN_LINES = (
    '# ' + MARK + ': the game-owned widescreen plugin.',
    '"${CMAKE_CURRENT_SOURCE_DIR}/sfex2p_widescreen.c"',
)

# Appended at the end of the file, which is also what makes it correct (see the comment it
# writes). Appending needs no anchor, so this installer copies nothing out of the project's
# own CMakeLists.txt.
CMAKE_STAGE_BLOCK = '''
# ''' + MARK + ''': the game's own mod packages (the widescreen
# feature). Kept last on purpose: POST_BUILD commands run in the order they are
# declared, and the framework stages its builtin catalog by clearing mods/packages
# first, so this has to come after it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mods/packages")
    add_custom_command(TARGET psx-runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/mods/packages"
            "$<TARGET_FILE_DIR:psx-runtime>/mods/packages"
        COMMENT "Staging the game's mod packages"
        VERBATIM)
endif()
'''

GAME_BLOCK = '''
# ''' + MARK + ''' — read when the sfex2p.widescreen mod selects 16:9 (widescreen is
# mod-owned): the fights are 3D (GTE gameplay detector).
# nw_phase_backdrop stretches the 2D stage backdrop over the wide frame, as in SF EX Plus Alpha (same Arika
# engine). It is OFF here: the plugin already widens the game's own backdrop loop to 23 columns, so
# stretching it on top duplicated the edge and left vertical seams in the margins. Set it to true only if
# the widen plugin is unavailable.
[widescreen]
gte_game_mode = true
nw_phase_backdrop = false
'''


def read_text(path):
    """Return (line ending, text normalised to \\n)."""
    with open(path, 'rb') as f:
        text = f.read().decode('utf-8')
    return ('\r\n' if '\r\n' in text else '\n'), text.replace('\r\n', '\n')


def write_atomic(path, text):
    os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
    tmp = path + '.tmp'
    with open(tmp, 'w', encoding='utf-8', newline='') as f:
        f.write(text)
    os.replace(tmp, path)


def main():
    parser = argparse.ArgumentParser(description='Install sfex2p.widescreen into a game project.')
    parser.add_argument('project_root', help='the game project folder (holds CMakeLists.txt and game.toml)')
    parser.add_argument('--dry-run', action='store_true', help='report what would change and write nothing')
    args = parser.parse_args()

    root = os.path.abspath(args.project_root)
    for required in ('CMakeLists.txt', 'game.toml'):
        if not os.path.isfile(os.path.join(root, required)):
            sys.exit('not a game project root (no %s): %s' % (required, root))
    for required in (PLUGIN_SRC, PACKAGE_SRC):
        if not os.path.isfile(required):
            sys.exit('missing repository file: %s' % required)

    pending, report = [], []

    cmake = os.path.join(root, 'CMakeLists.txt')
    eol, text = read_text(cmake)
    patched = text
    if 'sfex2p_widescreen.c' not in patched:
        lines = patched.split('\n')
        hits = [i for i, line in enumerate(lines) if CMAKE_ANCHOR in line]
        if len(hits) != 1:
            sys.exit('CMakeLists.txt: expected exactly one %s line, found %d '
                     '(unsupported project layout)' % (CMAKE_ANCHOR, len(hits)))
        at = hits[0]
        pad = ' ' * (len(lines[at]) - len(lines[at].lstrip()) + 4)
        lines[at + 1:at + 1] = [pad + line for line in CMAKE_PLUGIN_LINES]
        patched = '\n'.join(lines)
        report.append('CMakeLists.txt: plugin added to %s' % CMAKE_ANCHOR)
    if "the game's own mod packages" not in patched:
        patched = patched.rstrip('\n') + '\n' + CMAKE_STAGE_BLOCK
        report.append('CMakeLists.txt: mods/packages staged next to the executable')
    if patched != text:
        pending.append(('text', cmake, patched.replace('\n', eol)))

    game_toml = os.path.join(root, 'game.toml')
    eol, text = read_text(game_toml)
    if '[widescreen]' not in text:
        pending.append(('text', game_toml, (text.rstrip('\n') + '\n' + GAME_BLOCK).replace('\n', eol)))
        report.append('game.toml: [widescreen] block added')
    elif not any(mark in text for mark in KNOWN_MARKS):
        sys.exit('game.toml already has a different [widescreen] block: merge it by hand')

    for src, dst_rel in ((PLUGIN_SRC, 'sfex2p_widescreen.c'), (PACKAGE_SRC, PACKAGE_REL)):
        dst = os.path.join(root, dst_rel)
        with open(src, 'rb') as f:
            payload = f.read()
        if os.path.isfile(dst):
            with open(dst, 'rb') as f:
                if f.read() == payload:
                    continue
        pending.append(('copy', dst, src))
        report.append(dst_rel.replace(os.sep, '/'))

    if args.dry_run:
        print('\n'.join('would change: ' + line for line in report) if report
              else 'nothing to do: the mod is already installed')
        return

    for kind, dst, payload in pending:
        if kind == 'copy':
            os.makedirs(os.path.dirname(dst) or '.', exist_ok=True)
            shutil.copyfile(payload, dst)
        else:
            write_atomic(dst, payload)

    if report:
        print('\n'.join(report))
        print('\nNow rebuild the game, then enable "Widescreen (16:9)" under Mods in the launcher.')
    else:
        print('nothing to do: the mod is already installed')


if __name__ == '__main__':
    main()
