![SEBANNER](Resources/surreal-engine-banner.png)

# What is this?

this is a fully working (Roomscale) VR Implementation for the SurrealEngine Project

VR is only implemented in engine, so this Implementation will not get further feature whise (for theat the games Unrealscript Would need to be changed)
It does come with a giant benefit tho: All games that are supported by SurrealEngine should also be playable wiht this build in VR, since no game specific canges are made.

I only tested the GOG versions of Unreal and Unreal Tournement GOTY, because i don't own another UE1 game

I also only tested with the Valve Index HMD

Consider the whole thing Experimental. SurrealEngine is not in a state yet,
where a VR Implementation is really sensible.
The SurrealEngine team is also not affiliated with this project.

In the future, when SurrealEngine is more finalized, there might be an official VR implementaion

## Features

- Stereo rendering and 6DOF Headtracking
- Main/Pause menu controllable with "laserpointer" controls
- Main/Pause menu can be opend with the contoller
- handtracked Weapons and items - booth are trigger aktivated on there respective hand
- Head or hand based locomotion direction
- Halfe Live Alex style Item and Weapon quickselect Wheel
- UI decoupled from stereo view - lives on your fore arm instead
- almost everything is configurable in launcher settings
- triggers, movers and items can be picked up/activated by touching them with your hand (no need to be rude and bump into them)

## Future Development

SurrealEngine is still under active Development and not feature complete.
This implementation will be keept up to date with the original SurrealEngine.
I will also add some polish to the current VR implementaion, like Improving the Visuals of the weapon/item wheel and updating the contrller models


# Oringinal SurrealEngine Readme

Surreal Engine is a project that aims to reimplement Unreal Engine 1; currently focused on making Unreal (Gold) and Unreal Tournament (UT99) playable. The scope of this project might expand to cover more UE1 games in the future.

## Current Status

Please refer to [Status.md](Docs/Status.md) for the current status of Surreal Engine!

## System Requirements

* Original copies of the UE1 games you want to run
* Windows 10+ or a modern Linux distro
* A Direct3D 11 or Vulkan capable graphics card

## Building Surreal Engine

Please refer to [Building.md](Docs/Building.md) for details!

## Downloads

[Nightly builds are available on the Releases section](https://github.com/dpjudas/SurrealEngine/releases/tag/nightly).

Additionally, Surreal Engine is available on following Linux distributions:

* Arch: [AUR](https://aur.archlinux.org/packages/surrealengine-git)
* Nix: [Package Search](https://search.nixos.org/packages?channel=unstable&show=surreal-engine) | [Quickstart](https://github.com/NixOS/nixpkgs/pull/337069)

## How to Play

* Run the `SurrealEngine` executable.
* Add the UE1 games you want in the Folders tab.
* Select the game you want to play in Games tab.
* Click "Play"!

## Discord Server

Visit us on Discord at https://discord.gg/5AEry4s

## Command Line Parameters

`SurrealEngine [--url=<mapname>] [--engineversion=X] [Path to game folder]`

If no game folder is specified, and the executable isn't in a System folder, the engine will search the registry (Windows only) for the registry keys Epic originally set.

If no URL is specified it will use the default URL in the ini file (per default the intro map).

The `--engineversion` argument overrides the internal version detected by the engine and should only be used for debugging purposes.
