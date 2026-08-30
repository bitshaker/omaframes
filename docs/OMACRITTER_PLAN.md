# OmaCritter implementation plan

OmaCritter is the second built-in OmaFrames pack: one small, theme-aware
creature that walks around window edges and occasionally jumps to another
window on the same visible workspace and monitor.

This plan deliberately keeps live rendering inside the Hyprland plugin. QML is
used for visual study and future settings, not as a transparent desktop-sized
overlay.

## Prior art

The design borrows structural ideas, not code or art, from two desktop-pet
projects:

- [PixelPaws](https://github.com/jordansbc/pixelpaws) separates its behavior
  state machine, per-frame simulation, window-surface discovery, and sprite
  animation. Its surface provider filters unusable windows and its engine
  recovers when the current platform disappears. OmaCritter uses the same
  separation, but Hyprland supplies exact window geometry instead of polling
  Win32 rectangles.
- [desktop-pet](https://github.com/7ang0n1n3/desktop-pet) demonstrates a
  Wayland/Hyprland pet with one timer, randomized walk intervals, Cairo-drawn
  creatures, and monitor-aware motion. Its layer-shell surface is appropriate
  for a standalone application; OmaCritter instead renders in the compositor
  so it can remain attached to animated window geometry without an overlay.

Classic Oneko and XPenguins establish the visual language of short idle
behaviors, simple frame animation, and creatures treating desktop boundaries
as physical surfaces. Their X11 root-window assumptions are not used.

## Product boundary

The first version provides:

- one critter globally;
- a calm idle/walk/crouch/jump/land loop;
- traversal around all four edges of tiled, floating, and maximized windows;
- jumps only between eligible windows on one visible workspace and monitor;
- a theme-aware procedural gecko rendered from cached Cairo textures;
- no input handling or click interception;
- a reduced-motion mode that parks the critter on the active window;
- hiding during true fullscreen.

Cross-monitor jumps, multiple critters, pointer interaction, feeding, speech,
and other desktop-pet features are explicitly deferred.

## Native architecture

### Built-in pack registry

The host will replace its direct Vines calls with a small compile-time registry.
Each built-in pack supplies:

```cpp
struct SPack {
    std::string_view id;
    void (*registerConfig)();
    void (*start)();
    void (*attach)(PHLWINDOW);
    void (*stop)();
};
```

This is not a dynamic third-party ABI. Both packs remain compiled into the one
Hyprland plugin and therefore share its exact-version guard.

### Critter pack

- `CritterPack` owns configuration and delegates lifecycle to the director.
- `CritterDirector` owns the single actor, monotonic clock, target selection,
  signal listeners, timer, shared texture cache, and recovery rules.
- `CritterDecoration` is attached to every mapped window. It gives Hyprland
  correct decoration extents, receives geometry updates, and queues the
  perched critter in the normal window decoration pass.
- `CritterPassElement` draws one cached texture. During a jump it is queued at
  Hyprland's post-window render stage, above application windows but below
  top/overlay layer-shell surfaces and the lock screen.

The actor state is:

```text
hidden -> idle -> walking -> crouching -> airborne -> landing -> idle
```

All window references held across frames are weak. If the host or target closes,
moves to another workspace, becomes hidden, or enters fullscreen, the director
selects the active eligible window, falls back to another visible candidate, or
hides cleanly.

## Geometry and motion

Window rails use Hyprland's current animated position and size plus floating and
workspace offsets. A normalized clockwise perimeter coordinate is converted to
one of four edge positions and orientations. Landing candidates are clamped
away from corners for stable touchdown; walking crosses corners by rotating
into the next edge.

Flights use a smooth horizontal interpolation plus a sine arc:

```text
position(t) = lerp(source, target, smoothstep(t))
arc(t)      = -height * sin(pi * t)
```

The target rail point is refreshed while airborne so normal tiling and workspace
animations do not leave the critter landing in empty space. Large topology
changes abort and rehome rather than snapping across the monitor.

One Hyprland event-loop timer runs at approximately the active monitor refresh
rate while motion is visible. It damages only the union of the actor's previous
and current boxes, arms a longer deadline during idle pauses, and disarms when
disabled or hidden.

## Rendering

The initial gecko is asset-free and drawn as a small, readable silhouette in
Cairo. The texture cache contains idle, two walk frames, crouch, flight, and
landing poses for four window-edge orientations and both travel directions.
Textures are shared across windows and rebuilt only when the quantized theme
palette changes. Hyprland scales the final render box for the active monitor.

The perched pass uses the host window's decoration ordering alongside Vines.
The airborne pass is compositor-global; therefore a jumping
critter can briefly pass above application popups. It remains below Omarchy's
top and overlay layers.

## Configuration

```text
plugin:omaframes:critter:enabled
plugin:omaframes:critter:motion_enabled
plugin:omaframes:critter:theme_aware
plugin:omaframes:critter:size
plugin:omaframes:critter:walk_speed
plugin:omaframes:critter:jump_interval_ms
plugin:omaframes:critter:hide_on_fullscreen
plugin:omaframes:critter:col.body
plugin:omaframes:critter:col.accent
```

## Delivery stages

1. Record this plan and add a deterministic QML behavior study.
2. Introduce the built-in pack registry without changing Vines behavior.
3. Add a static native critter and prove attachment, scaling, theming, and
   cleanup.
4. Add perimeter walking, bounded damage, pauses, and reduced motion.
5. Add inter-window target selection and the post-window flight pass.
6. Exercise the full live-test matrix and document remaining limitations.

Stages 1–5 are implemented. Stage 6 is incomplete: registration, existing/new
window attachment, runtime settings, reduced-motion toggling, and one perched
2× render passed. The extended test was interrupted by the GPU recorder
incident documented in [INCIDENT_2026-08-30.md](INCIDENT_2026-08-30.md).

## Acceptance criteria

- Existing and newly opened windows receive one Critter decoration.
- Exactly one critter is visible across all windows.
- Walking follows live tiled/floating/maximized geometry without lag or jitter.
- Corners rotate the pose without teleporting.
- A jump transfers ownership with no duplicate or missing settled frame.
- Closing either window during a jump cannot leave stale drawing or crash.
- One-window and zero-window workspaces are stable.
- True fullscreen hides the actor and restoration returns it safely.
- Runtime enable, reduced motion, size, speed, jump interval, and theme controls
  apply without a plugin reload.
- The critter is click-through and does not reserve layout space.
- Animation damages bounded actor regions rather than the full monitor.
- Vines rendering and configuration remain unchanged.
- Repeated load/unload cycles remove timers, listeners, pass elements, and
  decorations cleanly.
- QML lint, native compilation, symbol checks, HiDPI testing, and a long-running
  multi-window soak pass.

The development machine currently has one 2x, 60 Hz monitor. Mixed-scale and
cross-monitor behavior must therefore be verified in a nested Hyprland session
or on additional hardware before release.
