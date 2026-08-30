# OmaVines

Experimental living window decorations for Omarchy and Hyprland.

![QML vine growth visual study](docs/qml-preview.png)

This repository currently contains two deliberately narrow prototypes:

- a standalone Qt 6/QML visual study that grows vector vines around a simulated
  window;
- a native Hyprland window-decoration plugin that proves exact per-window
  attachment and compositor rendering with a simple stem, leaves, and buds.

The intended product is a hybrid: native rendering for precise window geometry,
plus a Quickshell/QML companion for settings, previews, palettes, and reduced
motion controls. See [the architecture notes](docs/ARCHITECTURE.md).

## Status

Prototype only. The QML study has been linted and rendered. The native plugin
has been compiled and live-tested against Hyprland 0.56.2. Attachment to both
existing and newly opened windows, HiDPI rendering, repeated load/unload, and
clean decoration removal have passed. Floating move/resize, maximized and true
fullscreen transitions, workspace movement, live settings, and five concurrent
Foot windows have also passed. Mixed-monitor and long-running performance tests
remain.

## QML visual study

Requirements:

- Qt 6.10 or newer (`ShapePath.trim` is used for the growth reveal);
- `qmlscene6` or `qml6`;
- Qt Quick Controls and Qt Quick Shapes.

Run the interactive study:

```bash
./scripts/run-qml-prototype
```

Regenerate the deterministic preview image with an offscreen software renderer:

```bash
./scripts/capture-qml-prototype
```

The study is asset-free: its stems, leaves, veins, and buds are all QML vector
paths and primitives.

## Native decoration study

Requirements:

- Hyprland development headers matching the running compositor exactly;
- a C++23 compiler;
- the development packages exposed by Hyprland's `pkg-config` metadata.

Build it:

```bash
make -C native
```

The output is `native/omavines-native.so`. The plugin checks the running
Hyprland hash during initialization and refuses to load if it was built against
different headers.

![Native OmaVines decoration around the Foot test window](docs/native-foot-test.png)

The first controlled compositor test is documented in
[the live-test report](docs/LIVE_TEST.md).

### Optional live test

A native Hyprland plugin can crash the compositor while it is experimental.
Save work first and have a TTY available before opting into this step:

```bash
hyprctl plugin load "$PWD/native/omavines-native.so"
```

Unload it with:

```bash
hyprctl plugin unload "$PWD/native/omavines-native.so"
```

The prototype exposes these runtime values:

```text
plugin:omavines:enabled
plugin:omavines:stem_thickness
plugin:omavines:extent
plugin:omavines:leaf_size
plugin:omavines:col.stem
plugin:omavines:col.leaf
plugin:omavines:col.bud
```

Hyprland 0.56 uses the Lua config API for transient changes. For example:

```bash
hyprctl eval 'hl.config({ plugin = { omavines = { enabled = false } } })'
hyprctl eval 'hl.config({ plugin = { omavines = { enabled = true } } })'
```

## Verification

Run the QML linter and rebuild the native plugin in one command:

```bash
./scripts/check
```

## License

[MIT](LICENSE)
