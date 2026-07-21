# Garbage collector: status and handover

Branch `gc-lifetime`, based on `origin/master`. The engine's garbage collector was never
finished: it allocated and tracked objects but never reclaimed any. This branch makes the
mark phase correct and verifiable, and leaves the sweep switched off behind a mark-only mode.

Nothing on this branch frees memory yet. `GC::Collect` defaults to `Mode::Full`, but the only
caller is the instrumentation, which always passes `Mode::MarkOnly`.

## What was wrong

Three defects, all in the core machinery:

1. `GCRootNode`'s constructor never linked the node into the root list, so `Collect` walked an
   empty list and every object traced as unreachable.
2. The mark phase threw away the worklist returned by `Mark`, so tracing never went past the
   first level of the object graph.
3. `PropertyDataBlock::Reset` was commented out with a note that it crashed because the class
   might already have been destroyed, so property data was never released.

The third one was the real blocker: destruction order. It is solved by sweeping in two phases.
`PreDestruct` runs on every unreachable object while all of them are still constructed, so an
object's teardown can safely reach into other objects that the same sweep is about to destroy.
Only after that pass does the sweep destruct and free anything.

## Current state

The mark phase traces properties (including struct, array and fixed-array elements), class and
field graphs, the level graph, actors, meshes, fonts, and playing sounds. Subsystems that hold
objects in plain pointers rather than `GCRoot` report them through `GC::AddRootMarker`;
`MarkEngineRoots` in `Engine.cpp` covers the engine's own fields. Registration happens at the
top of `Engine::Run`, before any map loads.

Actors take themselves out of the collision and light spatial hashes in `PreDestruct`. Those
hashes hold raw `UActor*` outside the object graph, so a sweep would otherwise free actors the
collision system still walks. `UActor::Destroy` already removes them on the normal path, and it
is the only path by which an actor leaves `Level->Actors`, but a swept actor is not guaranteed
to have gone through it. Both removals are guarded by an `Inserted` flag, so doing it twice is
a no-op.

## Instrumentation

`LogGCStats` in `Engine.cpp` runs a mark-only collect at every map transition and every ten
seconds of play, and writes to the engine log:

- total objects and memory, and how many objects and how much memory are unreachable
- the fifteen classes with the most unreachable instances
- stale spatial hash entries, if any; this line is absent when the count is zero, which is the
  expected case

A mark-only collect leaves the marks in place so reachability can be inspected afterwards
(`GC::IsUnreachable`). The caller must then call `GC::ClearMarks`, or the next collect will see
everything as reachable and report nothing.

`SURREAL_SHOW_STATS=1` turns on the same overlay as the `timedemo 1` console command.

## How to run a measurement session

    cd "build/Unreal Tournament GOTY/System"
    SURREAL_SHOW_STATS=1 ./SurrealEngine "--url=DM-Deck16][?Game=BotPack.DeathMatchPlus?MinPlayers=8?Difficulty=2" "$(pwd)/.."

Quit through the console `exit` command. The log is only flushed in `~Engine`, so a killed
process leaves no log. It lands in `~/.config/SurrealEngine/SE-Log-LastRun.txt`.

Get into the match and play for a few minutes. The garbage is spawned by combat, so a session
spent in the menus shows nothing regardless of how often it samples.

## What the measurements show

Three sessions on DM-Deck16 with eight bots. Live set means total minus unreachable.

The live set is flat. Over 3.3 minutes and eighteen samples it stayed between 20373 and 20517
objects, with no trend. A session of 7.7 minutes gave the same result. If the mark phase were
failing to reach live objects, that number would decline steadily; it does not.

Everything allocated during play is garbage that is never reclaimed. Over 462 seconds the heap
grew by 6949 objects and 9 mb, and the unreachable count grew from 2 to 6426. Roughly half the
heap was garbage after eight minutes, growing about 1.2 mb per minute without bound.

The garbage is entirely transient combat effects. At the last sample of one session the top
classes were `UT_SpriteSmokePuff` 376, `UT_BloodPuff` 254, `BloodSplat` 232, `UT_BloodBurst`
208, `ChunkTrail` 146, then shell cases, sparks, bullet pocks, gibs and respawn effects.
Nothing structural appears: no `UClass`, `UTexture`, `UFont` or `UWindow`. This is the
population a working collector is supposed to reclaim.

Fixing the root registration order dropped the unreachable count right after a map load from
1187 to 2. The 1187 was an artifact: `PackageManager::LoadMap` returns an unrooted package and
`Engine` held it in a plain pointer, so the level and every actor traced as unreachable whether
alive or not. Any measurement taken before commit `5b857d8e` is worthless for that reason.

## Not yet verified

The stale spatial hash line has not been seen in a session yet, because the fix and the audit
landed together and the build has only been smoke-tested for thirty seconds. The next session
should confirm the line stays absent. If it ever appears, something removes an actor from the
level without going through `Destroy`.

## Open work

Reference holders that are still untraced. None of them affect the numbers above, but each is a
way for a real sweep to free something that is still in use:

- `UWindow` texture and item references
- bytecode literals; the `ExpressionVisitor` has 91 methods and about 13 need bodies.
  `Bytecode::Allocations` allows a linear walk instead
- `NativeFunctions::FuncByIndex`
- `PackageManager::saveInfos`, `delayLoads` and `openStreams`
- `RenderSubsystem::MainFrame` and the mesh and light caches

`Frame::Callstack` and the render frame's actor lists are a different case. The callstack is
pushed and popped by a scoped guard so it is only non-empty during script execution, and
`VisibleFrame::Actors` and `FogBalls` are rebuilt every frame. They do not need marking as long
as collection only ever runs at a safe point between ticks. Choosing that collection point is
the open design decision, and it matters more than several of the remaining mark overrides.

Two `Package` objects trace as unreachable right after a map load, reported as `(non-object)`
because `Package` is the only GC type that is not a `UObject`. Two objects is not a leak, but a
package tracing as garbage should be understood before a sweep is allowed to free one.

Texture, mesh and bytecode payloads are still outside the memory accounting. `ExternalMemorySize`
currently only covers property data.

Enabling the real sweep should go behind a poison or quarantine mode first, so that a dangling
pointer fails loudly instead of reading recycled memory.

## Commits

    6190fa54  Fix garbage collector root list and mark phase
    d46b0c43  Destruct property data when sweeping, and add a mark-only mode
    5d8f66fc  Initialize the level package pointer
    248b1a4f  Log object counts at every map transition
    9e16aa91  Add an env var to show the stats overlay from startup
    98d217df  Count property data in the reported memory usage
    726e17cd  Trace object references when marking
    6bd490c1  Trace the level graph when marking
    a22e6c25  Sample object counts during play, not just at map transitions
    ea07f935  Report memory owned outside the allocation block, and trace meshes, fonts and playing sounds
    2ebb462a  Let subsystems report roots they hold in plain pointers
    5b857d8e  Register the engine roots before the first map loads
    f0d44f4b  Log which classes the unreachable objects belong to
    11b18d6b  Remove swept actors from the collision and light hashes
