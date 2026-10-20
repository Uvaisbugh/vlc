# Community redesign integration

The full native Qt/QML implementation is integrated from
[dsojevic/juxtopposed-vlc](https://github.com/dsojevic/juxtopposed-vlc), commit
`b876f75f43130249162796fd3057e1327e2b1d2b`. It descends directly from this fork's
initial VideoLAN baseline `429df92c508f98038ac992772fea1fec1322c9f9`.
Its author history and existing licenses/attribution are preserved.

This integration provides the application shell, player/chrome customization,
queue, library, onboarding, settings panels, tracks, subtitle timeline, waveform,
equalizer, and supporting backend changes. These are source capabilities pending
validation in our builds, not a claim that all Figma variants are complete.

## Local validation on 2026-10-20

- The earlier baseline and first component build succeeded on GitHub.
- Both original play/pause SVGs were exported from the supplied Figma file and
  retain their original 24 × 24 root dimensions.
- The single merge conflict was in `modules/gui/qt/assets.qrc`; the resolution
  includes both our original SVG resource entries and the integrated Inter fonts.
- Release tooling: 16 tests pass. The integration exposed two Windows
  portability issues: SOURCE_COMMIT metadata used CRLF, and a version-probe test
  omitted the required Windows PWD environment field. The metadata now writes
  LF explicitly and the test checks the correct platform-specific environment.
- Qt Designer text extraction: 2 tests pass, 1 platform skip.
- Initial standalone QML tests using Windows Qt 6.12: 645 pass, 23 fail.
  Correcting the test font to match the app's bundled Inter exposed two fixes:
  add-on website bindings must observe backend metadata revisions, and
  breadcrumb widths must use the same Qt text-rendering metrics as their labels.
  The breadcrumb test also waits for rendering before sending its pointer event.
  The latest result is **659 pass, 9 fail**; all remaining failures report the
  missing compiled VLC.Util module. The isolated library suite passes all 85
  checks. This standalone result is not a passing native suite.
- The native QML harness now loads bundled Inter and fails clearly if it is
  missing. Meson is allowed to build test targets before running them, including
  targets deliberately excluded from the default application build.

## Required fresh native validation

The integrated media-library patch set and network-enabled Qt change the
dependency requirements. Do not reuse the baseline's dependency archive as if
it were equivalent. The fork workflow builds matching sources when no matching
prebuilt archive exists. Its native Linux test job builds the patched media
library and actual VLC Qt modules before running the `qt` Meson test suite.

Windows smoke testing verifies archive checksums, launches native dummy and Qt
interfaces with ignored user configuration, plays generated PCM and FFV1 video
fixtures, requires real audio/video decoders and Qt video output, and checks
startup/resource/graphics errors. Pointer/keyboard interactions, rendered output,
visual parity, clean-machine installation, Linux/macOS distribution, and a
public release remain separate checklist gates.
