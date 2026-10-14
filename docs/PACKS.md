# Effect-pack contract

This is the prototype contract for built-in OmaFrames effects. It is kept small
on purpose and can change while the renderer is experimental.

## Native pack

Packs normally live under `native/src/packs/<id>/`; closely related packs may
share a renderer directory. Each owns:

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

## Fireflies, Frost and Railway

These three registry entries share the small `accents/` native renderer and QML
preview component, selected by a fixed enum. Each has its own config namespace,
decoration name, duplicate-attachment check and independent enabled/motion state.
All start disabled. Registry callbacks bind each enum value; shared render-pass
cleanup is idempotent. Config key strings have static pack-owned storage because
Hyprland retains their pointers.

Fireflies draws layered translucent lights with stable per-window phases. Frost
caches four small Cairo corner textures and scales them during a finite growth
animation. Railway draws rails, sleepers and three independently positioned cars
around the perimeter. Continuous motion requests the next damaged frame through
a one-shot timer just after visible rendering, leaving refresh pacing to the
compositor; timer damage is restricted to perimeter strips. Static
mode and completed frost growth do not rearm timers. Unload removes each timer
and pending accent render passes.

The standalone `AccentStudy.qml` gallery exercises all three looks and includes
a motion toggle. `AccentCapture.qml` fixes time for repeatable offscreen capture.
This preview is a visual study, not evidence of native compositor validation.
