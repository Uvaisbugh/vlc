# Build and release validation

Updated 2026-10-20. The baseline and first player component compiled on Windows;
the full native redesign still needs passing native tests and packaged runtime QA.
See REDESIGN_CHECKLIST.md for release gates and doc/redesign/INTEGRATION.md for
source provenance and current test results.

## Reproducible validation

The branch workflow `.github/workflows/windows-build.yml` runs four jobs:

1. `packaging-tests`: release-tool unit tests and locally generated FFV1/PCM
   media for runtime testing. FFmpeg is installed explicitly on the hosted runner.
2. `windows-x64`: official VideoLAN cross-compilation container, matching contrib
   dependencies, native VLC compilation, stripped runtime archive, SHA-256 checksum,
   licenses, and build evidence.
3. `windows-smoke`: a fresh Windows 2022 runner verifies/extracts the matching
   archive, launches the packaged dummy and Qt interfaces, plays generated audio
   and video, and rejects missing decoders, dummy video output, QML/resource errors,
   and graphics initialization failures. Logs are retained as artifacts.
4. `native-qt-tests`: Linux builds the patched media library and native VLC with
   Qt enabled, then builds/runs the actual C++ and QML `qt` Meson suite with the
   compiled VLC.Util module. The test harness uses the app's bundled Inter font.

The containers are versioned from upstream `extras/ci/gitlab-ci.yml`:

```text
registry.videolan.org/vlc-debian-win64-posix:20260611225331
registry.videolan.org/vlc-debian-unstable:20260611225331
```

The Windows container's observed digest is:

```text
sha256:8e83e800036f8f4a6da1031a6b80c2b5bad8337259c072ffa22398baaeaad880
```

Both jobs request complete source history and configure the workspace as a Git
safe directory inside their root-owned container. The local Windows checkout
has no configured native MinGW/Qt toolchain, Docker, or WSL distribution.

## Source and dependency identity

- VideoLAN baseline: `429df92c508f98038ac992772fea1fec1322c9f9`.
- Community native implementation: `b876f75f43130249162796fd3057e1327e2b1d2b`.
- Windows entrypoint: `extras/package/win32/build.sh`.
- Cross-build arguments: `-a x86_64 -x -o <runtime-directory>`.
- Native test setup: `meson setup build-redesign -Dqt=enabled
  -Dmedialibrary=enabled -Dtests=enabled -Dextra_checks=true`.
- Native test command: `meson test -C build-redesign --suite qt
  --print-errorlogs --timeout-multiplier 2`.

The redesign requires its patched media-library and network-enabled Qt sources.
The workflow calculates the contrib identity and uses a prebuilt archive only
when that exact archive exists. Do not substitute the baseline's upstream
archive for the integrated dependencies. Native Linux validation builds the
patched media library with `extras/package/release/build-medialibrary.sh`.

## Evidence and results

- Baseline build passed: https://github.com/Uvaisbugh/vlc/actions/runs/39383363475
- First player component build passed: https://github.com/Uvaisbugh/vlc/actions/runs/39386507328
- Current runs: https://github.com/Uvaisbugh/vlc/actions/workflows/windows-build.yml
- Public implementation PR: https://github.com/Uvaisbugh/vlc/pull/1

Each runtime artifact contains the matching source SHA in its name. Keep the
archive, SHA256SUMS, environment/contrib/build logs, native test diagnostics, and
Windows smoke logs together when assessing a candidate. A cancelled run provides
partial diagnostic evidence only. A successful compile alone is not a tested
Windows application or a completed design audit.

## Public release gates

1. Record all Figma frames/states and pass their visual/behavior comparisons.
2. Pass compilation, relevant C++/QML tests, and packaged runtime audio/video tests.
3. Verify seeking, pause/resume, queue advance, subtitles/audio tracks, fullscreen,
   resize/high DPI, opening files/URLs, and settings persistence on Windows.
4. Verify dependencies and installation from the candidate on a clean environment.
5. Include upstream notices, design attribution, matching source and dependency
   information, checksums, supported platforms, limitations, and signing status.
6. Publish source and tested binaries together and verify the public downloads.

`beta-release.yml` also supplies the community implementation's multi-platform
packaging pipeline. It is separate from this branch's validation and must not be
presented as completed Linux/macOS testing in this fork.

Juxtopposed's community design is credited under CC BY 4.0. Preserve its source
link and VLC's existing licenses. This is an unofficial community fork.
