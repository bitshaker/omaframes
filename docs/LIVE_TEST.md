# Native live-test report

Date: 2026-08-29

The first native compositor test used the locally installed Hyprland 0.56.2.
The running compositor and development headers both identified the exact commit
`efb50993780079460b0cbed1363e2166a2de1d9f` before the plugin was loaded.

![OmaVines rendered around a controlled Foot window](native-foot-test.png)

## Passed

| Check | Result |
|---|---|
| Qt 6 lint and deterministic QML render | Pass |
| Clean C++23 native build | Pass |
| Required Hyprland plugin exports | Pass |
| Runtime ABI/hash guard | Pass |
| Plugin load and registration | Pass |
| Attachment to a window that existed before load | Pass |
| Attachment to a Foot window opened after load | Pass |
| Runtime option registration | Pass |
| Stem, leaves, and buds visible in the compositor | Pass |
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
window's decoration list reported `OmaVines prototype` at priority 9980 while
loaded and no longer reported it after unload.

The expanded state test moved a floating Foot window, resized it to 640×480,
toggled maximized and true fullscreen states, moved it from workspace 1 to 3
and back, then disabled and re-enabled rendering through `hl.config`. Runtime
geometry was increased from the defaults `(3, 16, 13)` to `(6, 24, 20)` for
stem thickness, extent, and leaf size; the decoration repositioned cleanly.

Five Foot windows were then decorated simultaneously. Every client reported
`OmaVines prototype`, and closing four clients sequentially produced clean
relayouts. In true fullscreen the outward decoration is clipped beyond the
screen edge, so no vines remain visible and no corruption appears.

## Findings

- Fixed leaf counts look crowded on very small tiled windows.
- Adjacent decorated edges can become visually busy where tiled windows meet.
- A production renderer should reduce detail below a size threshold and offer
  an active-window-only mode.
- Runtime controls must use Hyprland's Lua `hl.config(...)` API; the legacy
  `hyprctl keyword` path is unavailable with the Lua config provider.

## Still to test

- multiple monitors and mixed monitor scales (only one monitor is connected);
- a longer rapid move/resize and workspace-animation soak;
- GPU/damage profiling and long-running many-window performance;
- active-window-only and reduced-motion behavior, which are not implemented;
- compositor upgrades and HyprPM compatibility pins.

The plugin was unloaded after testing, the disposable Foot window was closed,
and no Hyprland or Omarchy configuration was changed.
