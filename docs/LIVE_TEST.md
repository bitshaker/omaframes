# Native live-test report

Date: 2026-08-29

The first native compositor test used the locally installed Hyprland 0.56.2.
The running compositor and development headers both identified the exact commit
`efb50993780079460b0cbed1363e2166a2de1d9f` before the plugin was loaded.

The OmaFrames/Vines pack refactor was smoke-tested against the same compositor.
The renamed plugin registered as `omaframes` version `0.2.0-prototype`, and the
active window reported `OmaFrames: Vines` at priority 9980. The nested
`plugin:omaframes:vines:enabled` value toggled false and true through
`hl.config`, proving the pack namespace is live. Unload removed both the plugin
and decoration, with no Hyprland config errors before or after cleanup.

The animated, theme-aware prototype was then tested as version
`0.3.0-prototype`. A centered floating Foot window retained Hyprland's standard
border while the Vines stem advanced clockwise from the top edge through the
right, bottom, and left edges. Leaves and buds appeared only after the stem
reached their perimeter positions. The default 1800 ms duration and a slowed
12000 ms diagnostic run both completed without stalling the compositor.

![Native growth at the beginning, midpoint, and completed state](native-growth-test.png)

With animation disabled to isolate color behavior, the same live Foot window
was captured under Solitude, Hackerman, and Solitude again. Vines followed the
resolved gray and green window-border palettes immediately, and the restored
Solitude frame matched the first capture.

![OmaFrames Vines rendered around a controlled Foot window](native-foot-test.png)

The natural-leaf prototype was then tested as version `0.4.0-prototype`.
Heart-shaped leaves are rasterized from in-memory Cairo curves at three tilt
angles for each edge. Two 720×480 Foot windows received visibly different,
stable counts and positions. A 15000 ms run confirmed that the randomized
leaves and their small individual delays still remain behind the clockwise stem
reveal.

The texture palette was exercised through Retro 82 → Hackerman → Retro 82.
Leaves, veins, stem, and buds changed from orange to green and back without a
plugin reload, layout shift, config error, or compositor error.

![Theme-aware natural leaves under Retro 82, Hackerman, and restored Retro 82](native-theme-test.png)

A growth-stall regression was then isolated and tested as version
`0.4.1-prototype`. The original renderer listened to Hyprland's generic
animation tick, which goes idle when the compositor has no active native
animation. A vine could therefore freeze near the end of a window-opening
animation and jump to its time-based completed state only after focus or a
screenshot caused fresh damage.

The replacement uses a short-lived Hyprland event-loop timer to damage the
window throughout growth and once at completion. An existing focused ChatGPT
window completed an 8000 ms diagnostic run without a focus change. A Foot
window launched through Omarchy's actual `omarchy-launch-terminal` path then
completed a 3000 ms run without the old workaround; the result was also
confirmed directly in the live compositor. No persistent config was changed.

## Passed

| Check | Result |
|---|---|
| Qt 6 lint and deterministic QML render | Pass |
| Clean C++23 native build | Pass |
| Required Hyprland plugin exports | Pass |
| Runtime ABI/hash guard | Pass |
| Plugin load and registration | Pass |
| OmaFrames host and Vines pack namespace | Pass |
| Attachment to a window that existed before load | Pass |
| Attachment to a Foot window opened after load | Pass |
| Runtime option registration | Pass |
| Stem, leaves, and buds visible in the compositor | Pass |
| Heart-shaped leaf silhouette and visible tilt variants | Pass |
| Variable stable layout across same-sized Foot windows | Pass |
| Per-window clockwise stem growth | Pass |
| Growth continues after Hyprland's opening animation becomes idle | Pass |
| Final completed frame without focus or screenshot damage | Pass |
| Exact Omarchy Foot launcher regression path | Pass |
| Randomized leaves and buds staged behind stem progress | Pass |
| Standard Hyprland border retained | Pass |
| Runtime animation disable | Pass |
| Live Solitude → Hackerman → Solitude inheritance | Pass |
| Live Retro 82 → Hackerman → Retro 82 texture regeneration | Pass |
| HiDPI scaling | Pass |
| Tiled to floating transition | Pass |
| Floating move and resize to 640×480 | Pass |
| Maximized transition and restoration | Pass |
| True fullscreen transition and restoration | Pass |
| Workspace 1 → 3 → 1 transition | Pass |
| Runtime disable and re-enable | Pass |
| Live thickness, extent, and leaf-size changes | Pass |
| Five simultaneous decorated Foot windows | Pass |
| Sequential client removal and repeated relayout | Pass |
| Decoration removal on unload | Pass |
| Three consecutive load/unload cycles | Pass |
| Relevant Hyprland rolling-log errors | None |
| Hyprland config errors after cleanup | None |
| Compositor responsive after cleanup | Pass |

The Foot window was captured as a 725×874 logical-pixel region and produced a
1450×1748 image, confirming correct rendering on the monitor's 2× scale. The
window's decoration list reported the Vines decoration at priority 9980 while
loaded and no longer reported it after unload.

The expanded state test moved a floating Foot window, resized it to 640×480,
toggled maximized and true fullscreen states, moved it from workspace 1 to 3
and back, then disabled and re-enabled rendering through `hl.config`. Runtime
geometry was increased from the then-current defaults `(3, 16, 13)` to
`(6, 24, 20)` for stem thickness, extent, and leaf size; the decoration
repositioned cleanly. The natural-leaf prototype now defaults to `(3, 18, 16)`.

Five Foot windows were then decorated simultaneously. Every client reported
the Vines decoration, and closing four clients sequentially produced clean
relayouts. In true fullscreen the outward decoration is clipped beyond the
screen edge, so no vines remain visible and no corruption appears.

## Findings

- Adjacent decorated edges can become visually busy where tiled windows meet.
- The procedural count is capped per edge, but a production renderer should
  still reduce detail below a size threshold and offer an active-window-only
  mode.
- Each decorated window currently owns twelve 96×96 leaf textures. The cache is
  small in practice, but many-window GPU memory and transition churn still need
  formal profiling.
- Runtime controls must use Hyprland's Lua `hl.config(...)` API; the legacy
  `hyprctl keyword` path is unavailable with the Lua config provider.
- A full-border render call cannot be progressively clipped in this path;
  explicit perimeter segments provide a reliable prototype reveal.
- Hyprland's generic animation tick is conditional on native animated
  variables and cannot drive an independent plugin animation. OmaFrames now
  owns a timer that disarms as soon as each vine finishes growing.

## Still to test

- multiple monitors and mixed monitor scales (only one monitor is connected);
- a longer rapid move/resize and workspace-animation soak;
- GPU/damage profiling and long-running many-window performance;
- active-window-only behavior and integration with a host-wide reduced-motion
  preference (the pack-level animation toggle works);
- compositor upgrades and HyprPM compatibility pins.

The plugin was unloaded after testing, all disposable Foot windows were
removed, the theme was restored to the user's starting theme, and no persistent
Hyprland or Omarchy configuration file was edited manually.
