# Non-VR bugs: fixed locally, not yet upstream

Split out of `Bugtracker.md`. Everything below is a non-VR bug that is **fixed on this branch's
`master`** but whose fix is **not present on `origin/master`** (`github.com/dpjudas/SurrealEngine`) as
of 2026-07-21. See `Bugtracker.md` for full context and open items, and `Docs/VR-Bugs.md` for the
VR-specific split.

This supersedes the earlier `Docs/FixedBugs-NonVR.md`: that file was written assuming none of these
fixes had reached upstream yet. Checking `origin/master` directly found that an earlier PR from this
fork was since merged there (under rewritten commit messages), and it already carries the fixes for
BUG-001, BUG-002, BUG-003, BUG-004, BUG-006, BUG-007, BUG-062, both `globalconfig` fixes, and the
`CompareArray` stack-overflow fix — all confirmed by diffing the actual source, not just commit
messages. Those are **not** repeated here since there is nothing left to upstream for them.

Ordered by severity (S1 highest).

| ID | Sev | Defect | Fix commit(s) |
| --- | --- | --- | --- |
| BUG-008 | S1 | Loading any save crashes on UT99 (and any package version > 61) with "Could not cast object Class (class Class) to UActor" — `UModel::Save` wrote the zone count with `WriteIndex` while `UModel::Load` reads it with `ReadInt32`, desyncing the rest of the stream. Masked on Unreal Gold, whose maps take the `<= 61` OldFormat branch that has no zone count at all. | 893030fd |
| BUG-020 | S2 | Player-to-decoration and player-to-pawn collision not handled — pawn got stuck, then took phantom fall damage on breaking free. | fe65ac54 |
| BUG-022 | S2 | Possible to get stuck on some movers when approached from certain angles — `EncroachingActors` keyed its visited-marker on the mover instead of the candidate, and searched by the mover's collision cylinder instead of its brush bounding box. | d5647ed8, f82def33 |
| BUG-023 | S2 | Projectiles pass through some movers — blocking against a brush required the block flag on *both* sides, but `Projectile`'s class defaults are `bBlockActors=0`/`bBlockPlayers=0`. | f82def33 |
| BUG-009 | S3 | The travel map is keyed inconsistently between save and login (`PlayerReplicationInfo().PlayerName()` vs the destination URL's `Name` option), so travel transfer silently no-ops. Never user-visible on UT99 since UT doesn't carry inventory between maps anyway; would bite any UT-based mod/game that travels with items. | cd1c61a1 |
| BUG-025 | S3 | Underwater and flying collisions buggy (most visible in Klingon Honor Guard) — `isMoving` tested velocity components with `&&`, so movement on only one axis (straight up/down) counted as stationary and never moved. | d5647ed8 |
| — | S2 (unrated — not a tracked bug ID) | `GCRootNode`'s intrusive linked list linked in the wrong direction: `roots` pointed at the newest node while that node's own `next` was null, so `GC::Collect()` only ever marked one root, and destroying a short-lived root (e.g. a per-frame `GCRoot`) left the global `roots` head dangling at freed memory — the next `GCRootNode()` construction then wrote into that freed block, corrupting the heap. Found via a VR startup crash, but the bug itself is general-purpose GC code, not VR-specific. | 3f29a618 |

Supporting fixes/hardening in the same areas, not separately numbered: pickup/trigger regressions from
the BUG-020 depenetration change (11169bca), sliding along a mover instead of stopping dead (4469203a),
and stuck-detector false positives / physics-mode independence found while chasing BUG-022/023
(f108282b, f829e59f, abd342e2).

Still open in these same areas (tracked in `Docs/NonVR-Bugs-Open.md`, not here): BUG-005, BUG-021,
BUG-024.

BUG-026 (slide-through catwalks, DM-Conveyor) is also resolved — same root cause as BUG-062, whose fix
is already on `origin/master` — so it isn't listed above as its own row and needs no separate upstreaming.
