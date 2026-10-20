# VLC redesign checklist

Updated: 2026-10-20. Checked items require evidence; unchecked items are pending.
See `REDESIGN_PLAN.md` for sequencing and the source map.

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
| Pending Figma tool access | Unknown | Unassigned | Unexported | Not run | None | Blocked |
| `247:445` / Player | 1920 × 1080, dark, video playback | `player/qml/Player.qml`, `ControlBar.qml`, `TopBar.qml` (under `modules/gui/qt/`) | Not exported | Not run | Browser canvas inspected on 2026-10-20; detailed context still pending | Identified |
| `970:3091` / Miniplayer | Component; variants not yet inspected | `modules/gui/qt/player/qml/MiniPlayer.qml` | Not exported | Not run | Browser search selection on 2026-10-20 | Identified |

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

## Build attempts

- 2026-10-20: Added `.github/workflows/windows-build.yml` and started the first
  Windows x64 baseline build using the versioned VideoLAN cross-compilation
  container. [Run 39382291096](https://github.com/Uvaisbugh/vlc/actions/runs/39382291096).
  The initial run pulled the image successfully but checkout failed because
  the container UID could not write GitHub runner state files. The workflow
  now overrides the container user to fix that mismatch. Compilation, packaging,
  and runtime verification are not yet marked complete.
- Restarted run after the checkout fix:
  [39382578278](https://github.com/Uvaisbugh/vlc/actions/runs/39382578278).
  Observed in progress on 2026-10-20.

## First UI patch

Native play-button geometry, the exported play SVG, default accent, and dark
Window background are implemented. Syntax/resource checks pass; native build,
interaction checks, and visual comparison for this patch remain pending.
See `doc/redesign/PLAYER_SPEC.md` for exact inspected values and current gaps.

Baseline build [39383363475](https://github.com/Uvaisbugh/vlc/actions/runs/39383363475)
successfully compiled and uploaded the Windows runtime for commit
`84d6aa25f6fb5970c7bc8c2ad9380f02fdaba2e1`. Runtime smoke testing is pending.

No redesigned screen or public release is currently claimed complete.
