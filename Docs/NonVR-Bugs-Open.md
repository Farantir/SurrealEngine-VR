# Non-VR bugs: still open

Split out of `Bugtracker.md`: every non-VR bug with no fix yet, on either `master` or `origin/master`.
See `Bugtracker.md` for full defect write-ups (root-cause analysis, repro notes, ruled-out hypotheses)
and `Docs/VR-Bugs.md` / `Docs/NonVR-Bugs-Fixed-Unmerged.md` for the other two splits.

Ordered by severity (S1 highest); S5 (parked/out-of-scope) items are feature gaps rather than defects
and are kept at the bottom for completeness.

| ID | Sev | Defect |
| --- | --- | --- |
| BUG-040 | S1 | Opening a map sometimes crashes with "Failed to spawn the player actor". |
| BUG-005 | S2 | Saving packages (`.u*`, game saves) is not fully implemented — `PackageWriter` gaps include `GenerationCount = 0` and no generations written, and `Save` renames the previous file to `.old` with the failure swallowed. Scope is what UT99 v436 / Unreal Gold v226 need read back, not general-purpose `.u` authoring. |
| BUG-021 | S2 | Semisolid brushes are finicky — usually the player falls through as if they weren't there. **No repro**; the leading hypothesis (extent sweeps can't hit semisolid geometry) was refuted by measurement. Do not act on it without a repro. |
| BUG-041 | S2 | The `viewclass` console command crashes with a null deref. |
| BUG-045 | S2 | Every `UObject` of every level leaks for the process lifetime — the garbage collector is dead three separate ways (`GC::Collect()` has no call sites, `GC::Mark` discards its marklist, `UObject::Mark` is a stub) and nothing calls it. `PackageManager::UnloadPackage` also frees no objects. Measure with `GC::GetStats()` at load/unload boundaries before fixing. |
| BUG-046 | S2 | Unreal Gold's `Maps/UPak/Crashsite2.unr` fails to load with `ObjectStream::ReadString: Invalid size in Crashsite2`; other UPak maps may be affected too (unswept). Leading hypothesis: a negative length means UTF-16 in UE's string serializer, not an error. |
| BUG-050 | S2 | Third-person weapon meshes are never rendered. |
| BUG-051 | S2 | No dynamic lighting: Dispersion Pistol projectiles and Flares don't illuminate their surroundings. |
| BUG-070 | S2 | String `>` is unregistered on both target games — `Greater_StrStr` is registered at a Deus Ex/227-range native index (1186) instead of both games' actual index (116). Any script evaluating `StrA > StrB` throws. One-line fix, not yet applied. |
| BUG-073 | S2 | The VM's runaway-instruction guard neither aborts the frame nor exits the loop — a runaway script logs the same line every iteration forever instead of aborting, and without a debugger attached the intended `Break()` is a no-op here. |
| BUG-074 | S2 | `Frame::ThrowException` returns to its caller when a debugger is attached, and every call site is written as if it doesn't — sharpest case is `Frame::Run`, which then indexes `Statements` with the very out-of-range index it just diagnosed. Fixing this first (making it `[[noreturn]]`) is what makes BUG-073/075 simple. |
| BUG-075 | S2 | Unbounded script recursion overflows the C++ stack with no diagnostic — `Frame::Callstack` has no depth limit and `ExpressionEvaluator::Eval` recurses on the C++ stack per nested expression. UE1's own limit was 250 frames with a script error. |
| BUG-024 | S3 | Some mover buttons are too easy to push (e.g. the ceiling button in the Kevlar Suit room, Vortex Rikers, triggers by walking under it). User-confirmed. Leading hypothesis: `TryMove` dispatches `Touch` from every step-up sub-move, so a pawn touches anything up to a step-height above its real head on every step. Needs a before/after repro before fixing — the same class of change regressed pickups previously. |
| BUG-042 | S3 | Screen-tinting power-ups (Invisibility, Energy Amplifier) leave the tint applied — and accumulate it on re-pickup — until the map changes. |
| BUG-043 | S3 | Some sounds are far too loud (Pulse Rifle secondary, minigun firing). |
| BUG-044 | S3 | `VulkanRenderDevice::~VulkanRenderDevice` segfaults on every clean shutdown (seen on AMD RADV), after the game has otherwise exited normally. Cosmetic in effect but drops a core on every quit, masking real crashes and poisoning bisects. Pre-existing. |
| BUG-052 | S3 | Portals mostly work but push players/projectiles in unexpected directions. |
| BUG-053 | S3 | Nali Fruit Seeds and ASMDs placed in a map don't render, though they can be picked up. |
| BUG-054 | S3 | Shock Rifle beams render glitchy (UT). |
| BUG-055 | S3 | Waving water textures at the ends of waterfalls render broken (NyLeve's Falls, DM-ArcaneTemple). |
| BUG-060 | S3 | Bot and ScriptedPawn AI is largely non-functional — the gap is ~38 native handlers that register successfully and then call `LogUnimplemented` (StatLog, WebRequest/WebResponse, cache entries, skeletal anim), not missing registrations. Stubs to fill in, not natives to wire up. |
| BUG-072 | S3 | `Level.Year` is off by 1900 and `Level.Month` is 0-based — breaks `UnrealSaveMenu.uc`'s `MonthNames[Level.Month - 1]` (an out-of-bounds index in January) and date rendering in `TournamentGameInfo.uc`/`StatLog.uc`. `Millisecond` is also hardcoded 0, and the underlying `std::localtime` call is not thread-safe. |
| BUG-076 | S3 | `string(vector)` and `string(rotator)` don't round-trip — the parser expects a bare `"1,2,3"` but UE1 formats as `X=1.0,Y=2.0,Z=3.0`, so every component parses as 0. |
| BUG-078 | S3 | `DynamicCast` compares class **names**, not identity, so two same-named classes in different packages cast into each other successfully. |
| BUG-056 | S4 | Mirrors/reflections are buggy, especially at the edges of world geometry. |
| BUG-057 | S4 | ASMD tertiary fire rings render wrong (Unreal Gold). |
| BUG-061 | S4 | Bots rotate their whole body (feet off the ground) to look up and down. |
| BUG-077 | S4 | Three conversion operators disagree with UE1: `string(bool)` yields `"1"`/`"0"` instead of `"True"`/`"False"`; `string(object)` yields `Package.Name` instead of the object name alone; `string(rotator)` masks each component to 16 bits so negative angles print as large positives. `IntToByte`/`FloatToByte` also narrow with no clamp (UB on a negative float). |
| BUG-079 | S4 | `export scripts` is unusable in its two most useful forms: two-or-more-package arguments loop forever (`sep` never updated), and with no arguments one unreadable package aborts the whole run instead of being isolated and reported. |
| BUG-081 | S4 | No commandlet re-runs the native-coverage check that found BUG-070 without hand-scripting an export and a diff — `SurrealDebugger` already has everything needed for a `natives check` commandlet. |

**Unverified/unclassified** (no severity assigned; reachability on the two target games unverified — a
disassembly sweep is needed before implementing or documenting): four `ExpressionEvaluator` cases that
throw unconditionally — `LabelTableExpression`, `NativeParmExpression`, `ConstructExpression`,
`Unknown0x46Expression` — plus `Unknown0x15Expression`, which silently returns `Stop` on an unverified
guess. `LabelTableExpression` matters most: label tables are the last statement of every state's
bytecode, so a state body that falls off its end without a `Stop` would execute the table and throw.

## S5 — Parked (out of scope for the playable targets)

Not targeted while UT99 v436 and Unreal Gold v226 are the playable goals. Full detail in `Docs/Status.md`.

- **Other engine versions** — Unreal Gold 227* and UT 469* have many unimplemented natives/features;
  227k_14 crashes immediately; 227i Translator Scale option does nothing; some UPak natives are missing.
- **Other games** — Deus Ex (partially playable), Tactical-Ops (crashes on startup), Klingon Honor Guard
  (keybinds, botmatch crash), and every other detected UE1 game (crash on startup).
- **Missing subsystems** — no networking, no OpenGL renderer, no native-mod support (by design), VM
  arrays and network conditional execution unimplemented.
- **[Linux/ZWidget] Wayland backend** — menu positioning, persistent menus, no client-side decorations
  on GNOME. Tracked upstream in `SurrealWidgets`.
