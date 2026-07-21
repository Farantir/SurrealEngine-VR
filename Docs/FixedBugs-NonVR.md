# Fixed bugs that do not require VR

This is a split-out of `Bugtracker.md` for PR purposes: everything below is a bug that has
already been **fixed** on the `vr_renderer` branch and whose fix does not touch `VR/` or
depend on VR being enabled. It exists so this work can be reviewed/merged as its own PR,
separate from the VR feature work. See `Bugtracker.md` for full context, open items, and
the VR-specific fixes (BUG-058 and the WP-4 entries) that are intentionally *not* listed
here.

## WP-1 — Save/load and level travel persistence

| ID | Sev | Defect | Fix commit |
| --- | --- | --- | --- |
| BUG-001 | S1 | Loading a saved game crashes the engine after a few seconds — `VisibleMesh::DrawDebugInfo` dereferenced `pawn->StateFrame->LatentState` unguarded. | c86874b6 (part of WP-1 phase 1) |
| BUG-002 | S1 | On a loaded level it is impossible to destroy glass or activate movers — root cause was `UObject::Save` writing a zeroed state frame. | c86874b6 (part of WP-1 phase 1) |
| BUG-003 | S1 | Inventory from loaded saves does not transfer to the next map. | 1a673676 |
| BUG-004 | S1 | The Translator is lost on the first→second level transition. | 1a673676 |
| BUG-006 | S2 | `Engine::GameInfo` is only assigned in `LoadMap`, so it dangles at the previous level after `LoadFromSaveFile`. | c86874b6 (part of WP-1 phase 2) |
| BUG-007 | S1 | Saving intermittently crashes the engine — `UObject::Save` dereferenced `StateFrame->Func` unguarded. | c86874b6 |
| BUG-008 | S1 | Loading any save crashes on UT99 (and any package version > 61) — `UModel` zone count written with `WriteIndex`, read with `ReadInt32`. | 893030fd |
| BUG-009 | S3 | The travel map is keyed inconsistently between save and login, so travel transfer silently no-ops. | cd1c61a1 |
| — | S1 | Mid-mover restore crash and double-pawn bug on load (BasedActor relinking, event suppression during restore, latent-action index lookup). | 8cebb915 |
| — | S1 | Stack overflow when saving — `UProperty::CompareArray`/`CompareLessArray` recursed into themselves instead of delegating to `CompareElement`/`CompareLessElement`. | a0f912b5 |
| — | S2 | `globalconfig` array properties not loading from ini — `UClass::LoadProperties` read the un-indexed name instead of the array-indexed key. | 6416331b |
| — | S2 | `globalconfig` changes not propagated to sibling classes, only to derived ones. | 51ae2993 |

Still open in this area (not part of this split, no fix yet): BUG-005 (package/save writer gaps).

## WP-3 — Movers, collision and physics

| ID | Sev | Defect | Fix commit(s) |
| --- | --- | --- | --- |
| BUG-020 | S2 | Player-to-decoration and player-to-pawn collision not handled — pawn got stuck, then took phantom fall damage on breaking free. | fe65ac54 |
| BUG-022 | S2 | Possible to get stuck on some movers when approached from certain angles. | d5647ed8, f82def33 |
| BUG-023 | S2 | Projectiles pass through some movers — blocking required the block flag on both sides, but `Projectile` defaults to `bBlockActors=0`. | f82def33 |
| BUG-025 | S3 | Underwater collisions buggy (Klingon Honor Guard) — `isMoving` used `&&` across velocity components. | d5647ed8 |
| BUG-062 | S2 | Standing on another actor's collision cylinder makes the player crawl (physics flips walking/falling every frame) — cylinder hit normals wrong on flat end caps. | 7da5b74b |

Supporting fixes/hardening in the same area, not separately numbered: pickup/trigger
regressions from the phase-3 depenetration change (11169bca), sliding along a mover instead
of stopping dead (4469203a), stuck-detector false positives and physics-mode independence
(f108282b, f829e59f, abd342e2).

Still open in this area (not part of this split, no fix yet): BUG-021 (semisolid brushes,
no repro), BUG-024 (some mover buttons too easy to trigger), BUG-026 (partly addressed by
BUG-062; re-test before acting further).

## Not included here

- **BUG-058** (explosion/impact sprites face the wrong way) — fixed in VR phase 8, and the
  fix (`VisibleFrame::HeadLocalToWorld()`) is specifically about combining headset pose with
  body yaw. VR-dependent, kept out of this split.
- **WP-4** (VR presentation polish) — all four entries are VR-specific by definition.
- **WP-9** (VM correctness), **WP-6** (rendering fidelity), **WP-7** (AI behaviour) — these
  are documented findings from the structural review, not yet fixed; nothing to split out.
