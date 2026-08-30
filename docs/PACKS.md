# Effect-pack contract

This is the prototype contract for built-in OmaFrames effects. It is kept small
on purpose and can change while the renderer is experimental.

## Native pack

Each pack lives under `native/src/packs/<id>/` and owns:

- its stable id and user-facing display name;
- configuration values under `plugin:omaframes:<id>:...`;
- its `IHyprWindowDecoration` and render-pass elements;
- duplicate-attachment protection and render-pass cleanup.

The host currently calls three pack operations:

```cpp
void registerConfig();
void attach(PHLWINDOW window);
void unload();
```

The pack is compiled into `omaframes-native.so`. This avoids introducing a
second unstable binary interface on top of Hyprland's own unstable plugin ABI.
Once two substantially different effects exercise this contract, a common
interface or registry can replace the direct calls.

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
window's resolved Hyprland border gradient by default. Its QML pack contains
the richer visual reference used to guide the native renderer.
