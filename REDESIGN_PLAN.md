# VLC redesign implementation plan

Status: native redesign integrated locally; full native validation in progress; no release delivered.
Updated: 2026-10-20.

## Outcome

Implement Juxtopposed's VLC redesign in a public fork of VideoLAN VLC, using
VLC's existing Qt/QML interface and playback engine. Deliver a usable Windows
x64 desktop build first, with public source and a path to Linux and macOS.
The requested scope is every screen and state in the supplied Figma file;
the exact inventory must be captured before claiming scope coverage.

- Fork: https://github.com/Uvaisbugh/vlc
- Upstream: https://github.com/videolan/vlc
- Baseline commit: `429df92c508f98038ac992772fea1fec1322c9f9`
- Design: https://www.figma.com/design/jrTJ63LEcUmnj4hFo2C6KX/VLC-Redesign--Community-?node-id=0-1
- Community source: https://www.figma.com/community/file/1691465999134370980/vlc-redesign
- Video: https://youtu.be/ERFeOgqk324

## Current constraints

1. This chat does not expose Figma's design-reading tools. Signed-in browser
   inspection now exposes Properties and SVG exports, and the first player
   measurements and asset have been captured. The complete frame/state inventory
   remains pending. Do not invent node IDs or claim unverified visual fidelity.
2. This checkout is on Windows. No usable native Qt/MinGW build toolchain or
   Docker command was found on PATH, and the WSL distribution listing returned
   no distributions. GitHub cross-compilation now produces a Windows runtime;
   local runtime testing and the redesigned build remain pending.
3. VLC's macOS package uses a separate native interface. Qt/QML work does not
   automatically implement the macOS design. Scope that port explicitly after
   the Figma inventory and Windows baseline are verified.
4. The local checkout is shallow. Fetch sufficient history before workflows
   that calculate dependency hashes or compare historical changes.

The supplied Figma file can now be inspected in the signed-in browser. The main
player (`247:445`, 1920 × 1080), Controls (`491:1928`), and play button have been
inspected; see `doc/redesign/PLAYER_SPEC.md`. The miniplayer component
(`970:3091`) has been identified but not fully inspected. Baseline run
`39383363475` compiled and uploaded a Windows runtime through
`.github/workflows/windows-build.yml`; its checkout requests complete history.

## Implementation sequence

### Community implementation integration

The source now incorporates the native implementation from
https://github.com/dsojevic/juxtopposed-vlc at
`b876f75f43130249162796fd3057e1327e2b1d2b`, a direct descendant of our VideoLAN
baseline. Its Git history, software notices, Juxtopposed design attribution,
and Inter font license are retained. README identifies this integration clearly.

This supplies 85 redesign QML components, native player/library/settings
controllers, and 46 focused QML test files. Integration is not evidence that
every supplied Figma frame matches or that every feature works in our build.
The sequence below now describes auditing and completing this implementation,
rather than recreating its existing components.

The integrated backend requires patched media-library sources and network-enabled
Qt. The next Windows build must build matching dependencies; the original
baseline's upstream dependency archive is insufficient for these features.
Native Linux Qt tests and Windows packaged-runtime smoke tests have been added
to the fork's workflow.

### 1. Capture the complete design specification

Read the Figma root, enumerate its pages and frames, and request detailed
design context plus screenshots for each implementation-sized frame. Expand
sparse responses into their visible child nodes. Record each frame in the
inventory in `REDESIGN_CHECKLIST.md` with its node ID, dimensions, theme,
interaction states, source paths, and validation evidence.

Extract shared typography, spacing, corner radii, colors, shadows, icons,
motion, focus treatment, and responsive rules. Download static assets using
the Figma tool's export instructions, retain attribution, and map every asset
to its intended layer and local callsite. Media artwork remains dynamic.

### 2. Establish an unchanged VLC build

Use the pinned build environment and commands in upstream
`extras/ci/gitlab-ci.yml`; use `extras/package/win32/build.sh` for Windows
packaging. Build the baseline commit before changing UI code. Record compiler,
Qt version, dependencies, exact commands, and artifacts. Launch the executable
and verify a local video and audio file, seeking, fullscreen, and shutdown.

Keep source and binary identifiers together so later failures can be compared
against the unchanged baseline. Build instructions and release gates are in
`doc/redesign/BUILD_AND_RELEASE.md`.

### 3. Implement foundations and application shell

Extend the existing palette and style components to match the verified design.
Reuse native widgets, focus navigation, translations, and accessible labels.
Implement the main window, navigation, title/menu bars, empty state, and opening
media. Keep upstream functionality reachable throughout the redesign.

### 4. Implement playback and queue

Translate the verified player layouts into native QML. Wire play/pause, stop,
previous/next, seek, elapsed/remaining time, volume, mute, playback speed,
fullscreen, mini player, tracks, subtitles, and queue controls to the existing
controllers. Preserve control-bar customization and keyboard navigation where
they intersect the new layouts. Verify auto-hide and popup focus behavior.

Treat features needing new data models (for example any waveform, thumbnail
preview, or subtitle synchronization visualization found in Figma) as separate
backend tasks. Do not substitute decorative graphics for working features.

### 5. Implement library, browsing, dialogs, and preferences

Apply each verified frame to library and network views, search, sort/filter,
media details, context menus, playlists, and settings. Use existing media
library models and settings persistence. Audit loading, empty, error, selected,
hover, focus, and disabled states. Add missing backend support where the
design's behavior cannot be expressed by current controllers.

### 6. Verify every frame and behavior

Build the actual VLC application. Compare screenshots at matching viewport,
theme, and scale against the Figma targets. Record per-frame results; fix
in-scope mismatches. Check every static asset's file, slot, callsite, and rendered
geometry. Exercise native playback and settings, not just a standalone mockup.

Run relevant upstream Qt tests and add focused regression tests for changed
behavior. Check keyboard-only use, screen reader labels, high DPI, resizing,
fullscreen transitions, and performance. Verify on Windows and Linux; keep
macOS status distinct until its interface is implemented and tested.

### 7. Publish a public downloadable release

Keep upstream license notices and credit the design author. Identify this as
an unofficial fork. Publish the matching source revision, dependency/license
information, checksums, installation steps, and known limitations with each
binary. Smoke-test the packaged app on a clean Windows environment before
calling it ready. Signing and platform coverage must be stated accurately.

## Initial source map

These are verified upstream paths, not a completed Figma frame inventory.
Paths are relative to `modules/gui/qt/` unless otherwise specified.

| Area | Existing source | Integration concern |
| --- | --- | --- |
| Shared style | `style/VLCStyle.qml`, `style/systempalette.cpp`, `style/defaultthemeproviders.hpp` | Scale-aware tokens, light/dark/system palettes |
| Shell | `maininterface/qml/MainInterface.qml`, `maininterface/qml/MainDisplay.qml` | Window lifecycle, navigation, embedded video |
| Navigation | `maininterface/qml/SideNavigationPane.qml`, `maininterface/qml/LocalTopbar.qml`, `maininterface/qml/MenuTopbar.qml` | Focus order and native window controls |
| Empty/open state | `maininterface/qml/NoMedialibHome.qml`, `maininterface/qml/OpenFileSplitButton.qml` | Open dialogs and media-library availability |
| Video player | `player/qml/Player.qml`, `player/qml/TopBar.qml` | Video surface, fullscreen, auto-hide |
| Transport and seek | `player/qml/ControlBar.qml`, `player/qml/PlayerControlLayout.qml`, `player/qml/SliderBar.qml` | Player state, seekability, time labels |
| Volume | `player/qml/controlbarcontrols/VolumeWidget.qml` | Volume/mute bindings and shortcuts |
| Mini/PIP player | `player/qml/MiniPlayer.qml`, `player/qml/PIPPlayer.qml`, `player/qml/MinimalView.qml` | Window sizing and video ownership |
| Tracks and speed | `player/qml/TracksMenu.qml`, `player/qml/TracksPageAudio.qml`, `player/qml/TracksPageSubtitle.qml`, `player/qml/PlaybackSpeed.qml` | Real tracks, subtitles, rate |
| Queue | `playlist/qml/PlaylistPane.qml`, `playlist/qml/PlaylistDelegate.qml` | Reordering, selection, current item |
| Library | `medialibrary/qml/HomeDisplay.qml`, `medialibrary/qml/VideoDisplay.qml`, `medialibrary/qml/MusicDisplay.qml` | Existing library models and artwork |
| Network | `network/qml/BrowseHomeDisplay.qml`, `network/qml/ServicesHomeDisplay.qml` | Discovery, URLs, failures |
| Preferences/effects | `dialogs/preferences/`, `dialogs/extended/` | Existing QWidget dialogs and persistent configuration |
| Build integration | `Makefile.am`, `meson.build` | Register all new QML, assets, and C++ sources |
| Tests | `tests/`, `qtest.pro`, `quicktest.pro` | Existing Qt and QML testing infrastructure |
| macOS interface | `modules/gui/macosx/` (repository root) | Separate platform implementation |

## Completion rule

The task is complete only when every inventoried Figma screen/state is accounted
for, implemented where applicable, functionally verified, visually compared,
and included in a tested public desktop release. A plan, theme-only patch,
successful compilation, or a screenshot alone does not satisfy that rule.
