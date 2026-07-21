# VR bugs

Split out of `Bugtracker.md`: every defect that is specific to the VR feature (hand aiming, wrist
tablet, weapon/item wheel, controller presentation) or that only reproduces in VR. See `Bugtracker.md`
for the full merged list and `Docs/VR.md` for the VR architecture and known-limitations section these
were folded in from.

One bug (BUG-010) is cross-cutting — its root cause is in general weapon-fire semantics, not VR code,
but it is also listed in `Docs/VR.md`'s known limitations because held-trigger charging is how VR fires
these weapons. It is kept here for visibility; the fix belongs to `Docs/NonVR-Bugs-Open.md`'s WP-2 scope.

Ordered by severity (S1 highest). Fixed entries are struck through with the fixing phase/commit noted.

## Open

| ID | Sev | Defect |
| --- | --- | --- |
| BUG-010 | S2 | Charging weapons (Dispersion Pistol, Impact Hammer, Rocket Launcher) mishandle the held trigger: the Rocket Launcher fires one rocket immediately and only then starts charging. Same on alt-fire. **Not VR-exclusive** — this is general weapon-fire semantics (`WP-2` in the main tracker) that VR also inherits because a held controller trigger is how these weapons are fired in VR. |
| BUG-012 | S3 | Firing with the hand against a wall can spawn the projectile clipped, because `FireOffset` puts the shot origin at the hand. |
| BUG-030 | S3 | The translator model is fully black in the item wheel and in hand when selected (the flare model renders correctly). |
| BUG-031 | S3 | Swimming and flying orient movement by `ViewRotation`, so the player swims/flies toward the aimed hand instead of the body. |
| BUG-032 | S4 | Controller models are far too large — should be ~20% of current size, and a circle with a line is enough (no sphere). |
| BUG-033 | S4 | The HUD tablet is always on the forearm; it cannot be dismissed or hidden. |

## Fixed

| ID | Sev | Defect | Fix |
| --- | --- | --- | --- |
| BUG-058 | — (unrated in the source list) | ~~Explosion/impact sprites face the wrong way instead of the camera.~~ Sprites billboarded off `ViewRotation` alone, which in VR is only the body-anchor yaw from the phase-5 aim/view split, never the headset pose. | VR phase 8 — new `VisibleFrame::HeadLocalToWorld()` combines the two; applied to `VisibleSprite::Draw` and the wheel's icon fallback. VR-only fix, not present on `origin/master` (VR is not upstream). |
