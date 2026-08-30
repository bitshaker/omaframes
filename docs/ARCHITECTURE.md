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

The Vines renderer currently draws a rounded stem border, simple leaf pills,
and buds. These shapes prove the lifecycle and rendering path before richer
geometry and animation are ported from the QML study.

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
- **P2:** replace the native pills with organic perimeter curves and staged
  growth; profile GPU and damage cost.
- **P3:** add a second pack to validate the interface, then build the
  Quickshell settings/preview companion and an IPC spike.
- **P4:** add Omarchy/HyprPM packaging, release pins, recovery instructions,
  and compatibility CI.
