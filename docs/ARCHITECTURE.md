# Architecture notes

OmaFrames is a hybrid effect engine. The compositor-facing host, visual effect
packs, and eventual QML companion have deliberately different jobs.

## OmaFrames host

The native host is intentionally small. It checks Hyprland ABI compatibility,
listens for new windows, walks existing windows during startup, and delegates
registration, attachment, and cleanup to each built-in pack.

This keeps Hyprland lifecycle code shared while letting effects evolve
independently. The current host compiles packs into one plugin; dynamic external
pack loading can wait until the pack API and compatibility story are mature.

## Effect packs

An effect pack owns its renderer, settings, display name, and render-pass
cleanup. `vines` is the first implementation. Both halves use a matching path:

```text
qml/packs/vines/
native/src/packs/vines/
```

The native Vines entry point exposes a minimal lifecycle—register configuration,
attach to a window, and unload. The QML directory exposes visual components for
the preview study. See [PACKS.md](PACKS.md) for the current contract.

## QML companion

QML is a good fit for visual exploration and the eventual settings surface:

- animate paths from zero to full length;
- compose vector shapes, palettes, and subtle motion;
- build pack previews and controls quickly;
- provide accessibility options such as reduced motion.

The current QML app is a standalone Vines study around a simulated window. It
does not track a real Hyprland window. `ShapePath.trim` provides the growth
reveal and currently requires Qt 6.10 or newer.

## Native Hyprland plugin

Native decorations handle the compositor mechanics:

- attach to existing and future mapped windows;
- receive exact size, rounding, workspace animation, and monitor scale;
- draw in Hyprland's decoration pass;
- damage the correct area when geometry changes;
- avoid a separate overlay surface for every client.

The Vines renderer draws a segmented perimeter stem, heart-shaped leaves, and
buds. Leaves are authored as Cairo vector curves, rasterized into small
in-memory textures, and cached in twelve edge/tilt combinations. Each
decoration owns a stable procedural layout seed, so leaf counts, spacing,
rotation, and short sprout delays vary between windows without changing every
frame.

Each decoration also owns a monotonic growth clock and a short-lived Hyprland
event-loop timer. The timer damages the window's current full bounds every 8 ms
while growth is active, including one final settled frame, then disarms itself.
It cannot depend on Hyprland's generic animation tick because that tick goes
idle once the window-opening animation ends. Using Hyprland's live bounds keeps
invalidation aligned while a newly mapped window and its tiled neighbors are
still moving or resizing. The stem advances clockwise around four perimeter
segments and each leaf or bud sprouts after the stem reaches its position.
Hyprland's standard border is never replaced.

Theme-aware mode reads the window's already-resolved `m_realBorderColor`
gradient during rendering. That means active/inactive transitions, window-rule
colors, and Omarchy theme reloads reach the pack through Hyprland itself. Pack
colors remain available as explicit overrides when theme-aware mode is off.
Texture cache keys quantize those resolved colors to avoid rebuilding all tilt
variants for imperceptible steps in a live border-color transition.

Hyprland's plugin ABI is unstable. The `.so` must be rebuilt for the exact
installed Hyprland version. Distribution will need HyprPM commit pins for each
supported release.

## Intended bridge

The likely production split is:

1. The native host owns Hyprland compatibility, window lifecycle, geometry,
   visibility, and final compositor rendering.
2. Packs own their visual code, settings schema, animation state, and preview.
3. A Quickshell/QML companion provides pack selection, palette editing,
   animation controls, previews, and accessibility options.
4. The companion sends a small settings model to the native host. A Unix socket
   or tiny command dispatcher are the leading transport options.
5. Omarchy packaging installs and starts the host and companion as one plugin.

## Prototype roadmap

- **P0 — complete:** QML Vines study plus a native decoration that builds,
  loads, attaches to existing and new windows, renders on HiDPI, and unloads.
- **P1 — underway:** core geometry/state cases and five simultaneous windows
  pass. Remaining work includes active-window rules, size-aware detail,
  mixed-monitor scaling, and a longer damage/performance soak.
- **P2 — underway:** staged native growth, procedural heart-shaped foliage,
  per-window layout variation, motion disablement, and live theme inheritance
  pass. Organic stems, restrained idle motion, and GPU/damage profiling remain.
- **P3:** add a second pack to validate the interface, then build the
  Quickshell settings/preview companion and an IPC spike.
- **P4:** add Omarchy/HyprPM packaging, release pins, recovery instructions,
  and compatibility CI.
