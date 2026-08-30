# Security

## Runtime boundary

OmaFrames includes native code loaded into the Hyprland compositor. A defect in
the plugin can crash the graphical session, and the plugin runs with the same
user permissions as Hyprland. The Omarchy bar widget is also unsandboxed and
invokes `hyprpm` and `hyprctl` with fixed argument arrays.

Review changes before updating. Save work before enabling an untested build and
keep a TTY available. The native host checks that its build headers match the
running Hyprland commit and refuses to initialize on a mismatch.

## Reporting a vulnerability

Please report security-sensitive issues privately to Joe Homs through GitHub's
private vulnerability reporting for this repository. Include the Omarchy and
Hyprland versions, the OmaFrames commit, reproduction steps, and relevant logs.
Do not include secrets or unrelated personal data.
