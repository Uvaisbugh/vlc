# Build and release preparation

This document records a proposed route, not a successful build. Use the
checked-out upstream scripts as the authoritative source when commands change.

As of 2026-10-20, `.github/workflows/windows-build.yml` implements a first
Windows baseline build on GitHub-hosted Linux using the upstream container.
It compiles with extra checks and stages the native Windows runtime using
`-a x86_64 -x -o <artifact-directory>`. It preserves build logs, source/toolchain
identifiers, and SHA-256 checksums. A successful cross-compilation will still
require a separate Windows launch/playback test. The first run is
https://github.com/Uvaisbugh/vlc/actions/runs/39382291096.

## Baseline

- Upstream revision: `429df92c508f98038ac992772fea1fec1322c9f9`.
- Windows packaging entrypoint: `extras/package/win32/build.sh`.
- Upstream CI configuration: `extras/ci/gitlab-ci.yml`.
- Qt tests: `modules/gui/qt/tests/`, `qtest.pro`, and `quicktest.pro`.
- macOS packaging entrypoint: `extras/package/macosx/build.sh`.

## Windows x64

Upstream's `win64` job uses this versioned Linux cross-compilation image:

```text
registry.videolan.org/vlc-debian-win64-posix:20260611225331
```

The image was identified from source; registry access and execution are not
yet verified. Use a compatible Linux runner or Linux containers. The installed
Windows VLC runtime alone cannot compile this source tree.

Inside the build environment, start with upstream's compile/check invocation:

```sh
extras/package/win32/build.sh -a x86_64 -x
```

For an unsigned release archive, the script exposes:

```sh
extras/package/win32/build.sh -a x86_64 -r -i u
```

Confirm packaging behavior against the script and actual output before
publishing. Do not label these commands as verified until they succeed.
`-p` is an optional prebuilt-contrib path; upstream CI first computes the
contrib hash and checks availability. Preserve that check rather than assuming
that prebuilt dependencies exist for every revision.

The local checkout is shallow. Obtain needed history before running
`extras/ci/get-contrib-sha.sh`, which participates in upstream dependency
selection. Record the exact dependency source and image digest in build evidence.

## Linux

Upstream's Debian jobs use:

```text
registry.videolan.org/vlc-debian-unstable:20260611225331
```

Follow `extras/ci/gitlab-ci.yml` for build tools, configure flags, Meson/Ninja,
and tests. Keep Qt enabled: a successful engine-only build cannot validate the
redesigned application. Do not claim host-independent UI verification from a
cross-compile alone.

## Evidence required for each build

Record source commit, host/target architecture, image digest, tool versions,
dependency revisions, configure/build commands, logs, executable/package paths,
and SHA-256 checksums. Record tests as pass/fail/not-run with concrete details.
Keep baseline and redesigned build evidence distinguishable.

## Release gates

1. Complete frame inventory and behavior checklist.
2. Pass compilation and relevant tests.
3. Launch the packaged application and play real local video and audio.
4. Validate seeking, queue transitions, tracks, subtitles, fullscreen, scaling,
   opening media, and settings persistence.
5. Compare rendered UI against the reference frames; save evidence.
6. Verify runtime/plugin dependencies on a clean Windows environment.
7. Include upstream notices, design credit, matching source and dependency
   information, checksums, platform limitations, and signing status.
8. Publish source and binaries together; verify public download links.

The Figma community page identifies Juxtopposed as the author and lists
CC BY 4.0. Preserve the source link and record adaptations when implementing
the design. Retain VLC's existing licenses and notices. Do not imply that the
fork is an official VideoLAN release or endorsed by the design author.
