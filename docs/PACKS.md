# Effect-pack contract

This is the prototype contract for built-in OmaFrames effects. It is kept small
on purpose and can change while the renderer is experimental.

## Native pack

Each pack lives under `native/src/packs/<id>/` and owns:

- its stable id and user-facing display name;
- configuration values under `plugin:omaframes:<id>:...`;
- its `IHyprWindowDecoration` and render-pass elements;
- duplicate-attachment protection and render-pass cleanup.

The host iterates a compile-time registry and calls four pack operations:

```cpp
void registerConfig();
void start();
void attach(PHLWINDOW window);
void stop();
```

The pack is compiled into `omaframes-native.so`. This avoids introducing a
second unstable binary interface on top of Hyprland's own unstable plugin ABI.
`registerConfig` runs before Hyprland reloads its configuration. Existing and
new windows then receive `attach`, shared directors begin in `start`, and packs
release timers, listeners, textures, and pass elements in reverse order through
`stop`.

## QML pack

Preview components live under `qml/packs/<id>/`. The standalone study imports a
pack by directory and supplies the simulated window, size, progress, and motion
state. A future Quickshell companion will discover pack metadata and instantiate
the same preview component in its settings surface.

QML pack code should not assume that it owns a real overlay window or receives
Hyprland input. Final window-attached rendering remains native.

## Current pack: Vines

Vines registers color, geometry, theme, and motion controls; attaches one
decoration per mapped window; and removes its custom render-pass elements
during unload. Each decoration owns its initial growth state and inherits its
window's resolved Hyprland border gradient by default. It also owns a stable
procedural seed for variable foliage placement and reuses a small cache of
theme-colored leaf textures at several rotations. Its QML pack contains the
richer visual reference used to guide the native renderer.

## Current pack: Chameleon

Chameleon uses a decoration on each window as a geometry and render-pass hook,
but keeps one actor in a pack-wide director. The director owns eligibility,
host and target selection, behavior state, bounded damage, and the shared
procedural texture cache. Perched drawing follows the host window; airborne
drawing switches to one compositor-global pass until landing transfers host
ownership. The matching QML component is a vector behavior study, not the live
renderer.
