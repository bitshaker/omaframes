# Architecture notes

OmaVines is exploring a hybrid design because the two halves of the effect have
very different jobs.

## QML companion

QML is a good fit for the visual language and eventual settings surface:

- animate a path from zero to full length;
- compose vector stems, leaves, buds, color palettes, and subtle motion;
- build controls and previews quickly;
- run as an Omarchy/Quickshell companion without taking window input.

The current QML prototype is a standalone visual study around a simulated
window. It deliberately does not claim to track a real Hyprland window yet.
`ShapePath.trim` is used for the growth reveal, which currently requires Qt
6.10 or newer.

## Native Hyprland decoration

A native decoration is a good fit for the compositor-facing mechanics:

- attach to every mapped window and future windows;
- receive exact position, size, rounding, workspace animation, and monitor scale;
- draw in the compositor's decoration pass;
- damage the correct area as a window moves or changes size;
- avoid a separate overlay surface per client.

The current native prototype renders a rounded stem border, simple leaf pills,
and buds. The shapes are intentionally basic. Its purpose is to prove the
attachment and rendering path before reproducing the richer QML design.

Hyprland's plugin ABI is unstable. The native `.so` must be rebuilt for the
exact installed Hyprland version, and a distributable repository will need
HyprPM commit pins for supported releases.

## Intended bridge

The likely production split is:

1. The native plugin owns per-window geometry, visibility, animation state, and
   final compositor rendering.
2. A Quickshell/QML companion owns palette editing, animation controls, preview,
   and accessibility options such as reduced motion.
3. The companion sends a small settings/state model to the native plugin. The
   transport is still open; a Unix socket or a tiny command dispatcher are the
   leading options.
4. Omarchy packaging installs and starts both pieces as one user-facing plugin.

Keeping the final vines native avoids the synchronization errors of a
screen-sized QML overlay, while keeping the companion in QML preserves fast
iteration on the look and controls.

## Prototype roadmap

- **P0 — complete here:** vector QML visual study and compile-checked native
  decoration skeleton.
- **P1:** live-test load/unload, confirm tiling/floating/fullscreen behavior,
  active-window rules, multi-monitor scaling, and compositor damage.
- **P2:** replace the native pills with an organic perimeter curve and staged
  growth animation; profile GPU and damage cost.
- **P3:** add the Quickshell settings/preview companion and an IPC spike.
- **P4:** add Omarchy/HyprPM packaging, release pins, recovery instructions, and
  compatibility CI.
