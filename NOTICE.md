# Notice

*(Español: [NOTICE.es.md](NOTICE.es.md))*

## Not affiliated

This is an unofficial, noncommercial fan project. It is **not affiliated with, endorsed by or
connected to Capcom or Arika**, or any of their subsidiaries, nor to the authors of the
[PSXRecomp](https://github.com/RetroPortingToolKit/psxrecomp) framework or of the
[game project](https://github.com/strider973/Street-Fighter-EX2-Plus-Recompiled) this mod runs
on. *Street Fighter*, *Street Fighter EX2 Plus* and all related names, characters and marks are
the property of their respective owners, and are used here only to identify the game this mod
is compatible with.

Nothing here is for sale, and nothing here is monetized.

## What this repository does not distribute

No ROM. No disc image. No BIOS. No built executable. No decompiled or recompiled game code. No
art, audio, music or font data from the game. This repository is a **source kit**: the plugin
is compiled into the game executable by whoever builds it, and that executable never leaves
their machine. Playing requires **your own legally obtained copy** of the game and a working
build of the game project, neither of which this repository provides or can provide.

## What it does contain of the original work

The plugin needs to know which instructions it is replacing, so it carries a table of
**22 instruction words — 88 bytes** — of the game's own backdrop-drawing function, each one
paired with the value the plugin writes in its place. The plugin **verifies every one of those
original words before it writes anything**: on a different edition of the game the check fails
and the plugin does nothing at all, rather than corrupting it. Those 88 bytes are the entirety
of the original program's code quoted anywhere in this repository, and they are quoted for that
verification.

`docs/HOW-IT-WORKS.md` describes the game's behaviour in prose — how its tile loop and its
ordering table work — as any mod's documentation has to. It contains no game code beyond the
words above.

The screenshots under `docs/images/` show the game running, in order to document what the mod
does. They remain the property of their respective owners.

## Takedown and contact

If you hold rights in this material and want something here removed or changed, write to
**jajdpmail@gmail.com**, saying what you object to and in what capacity you are writing.

Requests from rights holders are honoured: the disputed part is removed, or the repository is
taken down, without argument, and you will get a reply confirming it. No notice or legal
process beyond that email is needed to reach the person who maintains this.
