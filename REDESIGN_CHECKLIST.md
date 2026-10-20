# VLC redesign checklist

Updated: 2026-10-20. Checked items require evidence; unchecked items are pending.
See `REDESIGN_PLAN.md` for sequencing and the source map.
See `doc/redesign/FRAME_COVERAGE.md` for the observed screen-family/source map.

## Repository and preparation

- [x] Create a public fork of `videolan/vlc`: https://github.com/Uvaisbugh/vlc
- [x] Clone upstream and connect `origin` to the fork and `upstream` to VideoLAN.
- [x] Record baseline revision: `429df92c508f98038ac992772fea1fec1322c9f9`.
- [x] Inspect native Qt/QML source, styles, tests, and Windows build entrypoint.
- [x] Save an implementation plan, source map, and release checklist.
- [x] Open the supplied Figma file in the browser and identify the main player frame.
- [x] Inspect the signed-in player/control/play layers and export the original play SVG.
- [ ] Make Figma design-reading tools available in the implementation chat.
- [ ] Retrieve root design context and initial screenshot.
- [ ] Enumerate all pages, frames, variants, overlays, and interactions.
- [ ] Retrieve detailed context for every frame; resolve sparse responses.
- [ ] Record design tokens, exact assets, attribution, and dynamic content rules.

## Build baseline

- [x] Establish the upstream-compatible compiler/Qt/dependency environment in GitHub CI.
- [x] Fetch enough Git history for dependency hashes and upstream comparisons in CI.
- [x] Build unchanged VLC with the Qt interface enabled; run `39383363475`, build step passed.
- [ ] Launch and smoke-test the unchanged application.
- [x] Record build commands, environment, logs, and artifact revision in CI artifacts.
- [ ] Add and validate fork CI for compilation and relevant tests.

## Native implementation

The native implementation from `dsojevic/juxtopposed-vlc` at
`b876f75f43130249162796fd3057e1327e2b1d2b` is integrated and pushed with its history
and attribution. The areas below have source implementations available, but
remain unchecked until audited against Figma and validated in this fork.

The following areas are a provisional work breakdown. Final frame coverage
must come from the actual Figma inventory, not these category names.

- [ ] Shared style tokens, typography, themes, and exact static assets.
- [ ] Main window, title/menu bars, and navigation.
- [ ] Empty/open-media states, file opening, drag/drop, and network opening.
- [ ] Player header, metadata, artwork, and video layout.
- [ ] Play/pause, previous/next, stop, shuffle, repeat, and seek controls.
- [ ] Volume/mute, playback speed, fullscreen, and auto-hide behavior.
- [ ] Audio/subtitle selection, external subtitles, and synchronization controls.
- [ ] Queue display, selection, reorder, remove, and persistence as specified.
- [ ] Mini player and picture-in-picture states.
- [ ] Home, video library, music library, and playlists.
- [ ] Search, sorting, filtering, media details, and context menus.
- [ ] Preferences, effects/equalizer, and additional inventoried dialogs.
- [ ] Any newly required backend models or features found in the design.
- [ ] Loading, empty, error, hover, focus, selected, and disabled states.
- [ ] Responsive behavior, high DPI, keyboard navigation, and accessibility.
- [ ] Translation-ready strings and settings persistence.
- [ ] Register new QML, assets, and C++ files in both supported build systems.

## Frame inventory

No complete layer inventory has been retrieved yet. Add one row per actual
frame/state, including variants. Do not mark categories above done until all
their rows have passed.

| Figma node ID / frame | Theme / size / state | Native files | Assets | Functional result | Visual evidence | Status |
| --- | --- | --- | --- | --- | --- | --- |
| `247:445` / Player | 1920 × 1080, dark, video playback | `player/qml/redesign/RedesignPlayer.qml`, `player/qml/redesign/RedesignPlayerView.qml`, `RedesignPlayerChrome.qml` (under `modules/gui/qt/`) | Play/pause SVGs exported; integrated assets require audit | Native runtime pending | Browser canvas inspected on 2026-10-20; detailed context still pending | Identified |
| `970:3091` / Miniplayer | Component; variants not yet inspected | `modules/gui/qt/player/qml/MiniPlayer.qml` | Not exported | Not run | Browser search selection on 2026-10-20 | Identified |
| `6259:14954` / new playlist | Dark; 713 × 589; radius 14; background #191919 at 95%; blur 50; shadow y 8 / blur 15 / black 46% | `player/qml/redesign/RedesignPlaylistDialog.qml`, `RedesignDialogSurface.qml` | Integrated | Geometry and interaction tests pass with Inter; native runtime pending | Signed-in Figma Properties and canvas inspected 2026-10-20 | Reference measured; rendered parity pending |
| `2425:13624` / Player - Light | 1920 × 1080; vertical layout; V opens subtitle overlay | `player/qml/redesign/RedesignPlayerView.qml`, `RedesignTheme.qml` | Integrated; audit pending | Native runtime pending | Signed-in canvas and Properties inspected 2026-10-20 | Identified |
| `3046:9282`, `999:5047` / Settings - global | 1048 × 865; radius 14; #0e0e0e at 95%; blur 50; shadow y 8 / blur 15 / black 46% | `player/qml/redesign/RedesignSettingsDialog.qml`, `RedesignSettings.qml` | Integrated | 5 surface/modal/resize tests pass | Signed-in Properties inspected 2026-10-20 | Reference measured; native content/frame audit pending |

## Verification

- [ ] Actual VLC build succeeds with no new UI loading/runtime errors.
- [ ] Relevant existing Qt/QML tests pass; changed behavior has regression coverage.
- [ ] Every frame matches its reference at the recorded viewport and scale.
- [ ] Every static asset has the correct local file, layer slot, callsite, and geometry.
- [ ] Local audio/video playback, seeking, pausing, end-of-file, and queue advance work.
- [ ] Subtitle/audio tracks, rate, volume, mute, and supported sync controls work.
- [ ] Fullscreen, minimize/restore, resize, mini player, and PIP transitions work.
- [ ] File/URL open, drag/drop, unavailable media, and network failures are handled.
- [ ] Library indexing, search, filtering, sorting, and artwork update correctly.
- [ ] Preferences persist across restart and do not break upstream configuration.
- [ ] Keyboard-only navigation, focus visibility, and accessible names are verified.
- [ ] Windows package is tested on a clean machine/environment.
- [ ] Linux build and smoke tests pass.
- [ ] macOS implementation and testing are separately completed or explicitly deferred.

## Public release

- [ ] Confirm design attribution and retain upstream notices/licenses.
- [ ] Mark the product and repository description as an unofficial community fork.
- [ ] Package the executable with required runtime components and plugins.
- [ ] Make matching source and applicable dependency source/license information available.
- [ ] Generate release checksums and record the source commit and build environment.
- [ ] Document installation, supported platforms, signing status, and known limitations.
- [ ] Publish tested binaries and matching source under a public release tag.
- [ ] Verify public download links and installation from the published artifact.

## Current blockers and evidence

| Blocker | Evidence | Resolution |
| --- | --- | --- |
| Figma tools unavailable to this chat | Callable tool inventory contains no Figma tools; signed-in browser inspection now exposes Properties and SVG exports | Continue inspecting layers through the browser; full inventory remains pending |
| Local native compiler environment unavailable | Qt/MinGW/Docker not found in inspected locations/PATH; no WSL distributions listed | GitHub cross-compilation now works; use the produced runtime for Windows smoke testing |

## Build history

- 2026-10-20: Added `.github/workflows/windows-build.yml` and started the first
  Windows x64 baseline build using the versioned VideoLAN cross-compilation
  container. [Run 39382291096](https://github.com/Uvaisbugh/vlc/actions/runs/39382291096).
  The initial run pulled the image successfully but checkout failed because
  the container UID could not write GitHub runner state files. The workflow
  now overrides the container user to fix that mismatch. Compilation, packaging,
  and runtime verification are not yet marked complete.
- Restarted run after the checkout fix:
  [39382578278](https://github.com/Uvaisbugh/vlc/actions/runs/39382578278).
  It failed the Git ownership check, subsequently fixed using the scoped
  workspace safe-directory setting. The next baseline and first UI patch built
  successfully (runs 39383363475 and 39386507328).

- Full native integration validation first started in run 39397790039 after
  cancelling obsolete partial-component run 39396050384. Subsequent validation
  fixes require a new run; consult the branch's latest Actions run for status.

## First UI patch

Native play-button geometry, the exported play SVG, default accent, and dark
Window background are implemented. Syntax/resource checks pass; native build,
interaction checks, and visual comparison for this patch remain pending.
See `doc/redesign/PLAYER_SPEC.md` for exact inspected values and current gaps.

Baseline build [39383363475](https://github.com/Uvaisbugh/vlc/actions/runs/39383363475)
successfully compiled and uploaded the Windows runtime for commit
`84d6aa25f6fb5970c7bc8c2ad9380f02fdaba2e1`. Runtime smoke testing is pending.

No redesigned screen or public release is currently claimed complete.

## Integration validation

- [x] Confirm the community source descends from our official VLC baseline.
- [x] Preserve community authorship/history, design credit, and font license.
- [x] Resolve the resource merge by retaining both the SVGs and Inter fonts.
- [x] Run release-tool unit tests: 16 passed after two Windows portability fixes.
- [x] Run Qt Designer text-extraction tests: 2 passed, 1 platform skip.
- [ ] Run the QML suite with the actual compiled VLC.Util module and target Qt.
- [ ] Complete a fresh native build with the integration's patched dependencies.
- [ ] Run packaged Windows startup/playback checks on the integrated build.
- [ ] Complete screen-by-screen visual and interaction comparison with Figma.

The initial standalone Windows Qt 6.12 run recorded 645 passes and 23 failures.
After loading the same bundled Inter font as the app, fixing live add-on URL
refresh, measuring breadcrumbs with matching text-rendering metrics, and waiting
for the breadcrumb button to render before clicking, the result is **659 passed,
9 failed**. All nine failures are import/compile failures from the missing native
VLC.Util module. The isolated library suite passes all 85 checks. These results
do not replace the native suite or establish full visual parity.

Windows smoke validation now includes generated FFV1 video and PCM audio,
requires actual decoders and a Qt video window/display, rejects dummy video
output and graphics initialization failures, and retains runtime logs. This
test is defined but must pass on the freshly built integrated package.

Native Linux compilation passed at `0d38edc1` in run 39402484983. Its Qt suite
result was 72 passing suites / 10 failing suites: nine SVG decoding failures
from the missing runtime image plugin, and one read-only storage test invalidated
by root permissions. The workflow now provisions the matching SVG plugin and
runs tests without root privileges. A passing rerun remains required.
