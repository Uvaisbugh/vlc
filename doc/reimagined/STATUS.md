# VLC Reimagined: status and evidence

Reviewed 2026-10-21 against source `7f39528839a1473cf8919989f0603743101a207b`
on `codex/vlc-redesign`. This page distinguishes prior validation from the
documentation audit. [Download instructions](GETTING_STARTED.md) · [Roadmap](../../ROADMAP.md).

## Provenance

| Contribution | Evidence |
| --- | --- |
| VLC engine and application foundation | VideoLAN baseline `429df92c508f98038ac992772fea1fec1322c9f9`; existing `src/`, `lib/`, `modules/`, AUTHORS and license notices |
| Original visual concept | Juxtopposed's [video](https://www.youtube.com/watch?v=ERFeOgqk324) and [Figma Community file](https://www.figma.com/community/file/1691465999134370980/vlc-redesign); [recorded asset credits](../../modules/gui/qt/pixmaps/redesign/CREDITS.md) |
| Full native community implementation | David Sojevic's `b876f75f43130249162796fd3057e1327e2b1d2b`, integrated by merge `e2fcdd03`; original history preserved |
| This fork's work | Git diff from `b876f75f` to audited HEAD: CI/runtime validation, asset exports and attribution, targeted text/metadata/dialog fixes, packaging portability fix, and preview documentation |

The local checkout is shallow, but the relevant integration and subsequent
commits are present. This is not an exhaustive audit of VLC's full history.
Compare with `git diff b876f75f HEAD` rather than attributing the entire redesign
to this fork. [Integration details](../redesign/INTEGRATION.md).

## Published preview and validation

The GitHub API confirmed the public, non-draft prerelease
[`redesign-preview-2026.10.20`](https://github.com/Uvaisbugh/vlc/releases/tag/redesign-preview-2026.10.20).
It contains the Windows ZIP (99,779,905 bytes), matching source tarball,
`BUILD_INFO.json`, and `SHA256SUMS.txt`. Binary source:
`0279317bbd0e26bfaec7e6cc3b854d2b92801de8`.

| Check | Evidence and boundary |
| --- | --- |
| Windows build and startup/audio/video smoke | [Run 39404250425](https://github.com/Uvaisbugh/vlc/actions/runs/39404250425): `packaging-tests`, `windows-x64`, and `windows-smoke` jobs succeeded. Its separate `native-qt-tests` job failed; do not describe the entire run as green |
| Native Qt suite | [Run 39418826419](https://github.com/Uvaisbugh/vlc/actions/runs/39418826419) succeeded at `9c9516d321b63b29b8a9c3753b8ebc6e18f37dd8`; saved JSON log contains 82 results, all `OK`. The diff from binary source changes only `.github/workflows/native-qt-tests.yml` |
| Portable runtime | Existing local `build-baseline/portable-smoke/result.txt` records startup, Qt loading, PCM playback and video success; this ignored local evidence is not a public artifact link |
| Interactive behavior | Prior integration/release notes record subtitle rendering, play/pause, pointer seeking, seek preview, queue display, settings open/close and fullscreen/restore. These were not repeated during this documentation change |
| Package integrity | Local release ZIP SHA-256 rechecked against the public asset digest: `468c968e12503bb6723ebae05f5ef44f7204d109d5bc9c8e0e0e25794d994ab1` |

The native test environment was Linux with compiled VLC modules, bundled Inter,
Qt 6.10.2 and its matching SVG plugin, offscreen/software rendering and ordinary
user permissions. It verifies tests, not a Linux desktop distribution. Runtime
smoke checks use generated media and dummy audio output; they do not certify
all codecs, physical audio devices, GPUs, HDR, streams or every interaction.

## Source-backed feature status

Paths below are under `modules/gui/qt/`.

| Area | Source evidence | Status / remaining evidence |
| --- | --- | --- |
| Player and seek preview | `player/qml/redesign/RedesignPlayer.qml` binds playback/seek/fullscreen to Player/MainCtx; `player/player_controller.hpp` exposes seek-preview methods | Implemented; limited prior runtime verification |
| Queue | `RedesignQueue.qml`, `RedesignQueuePanel.qml`, native `tst_RedesignQueue` suite | Implemented and component-tested; display checked locally, full reorder/persistence/advance audit incomplete |
| Library | `RedesignLibrary.qml` imports `VLC.MediaLibrary`; `medialibrary/`; native `tst_RedesignLibrary` suite | Implemented and component-tested; indexing, search, artwork, sorting and persistence need systematic packaged-runtime checks |
| Settings/dialogs | `RedesignSettings.qml`, `RedesignSettingsDialog.qml`, `RedesignDialogSurface.qml`; native dialog/settings suites | Implemented; open/close and some geometry tested, all effects/persistence not verified |
| Themes/customization/waveform/subtitle timeline | Adjacent redesign components, `audio_waveform_scanner.cpp`, `subtitle_timeline_scanner.cpp`, corresponding native suites | Source and tests present; not a claim of complete design or runtime coverage |
| Mini-player/PIP, accessibility, localization | Existing source paths and translatable/accessibility properties | Full behavior, assistive-technology and locale validation outstanding |

The [frame coverage map](../redesign/FRAME_COVERAGE.md) is partial. Unchecked
items in [REDESIGN_CHECKLIST.md](../../REDESIGN_CHECKLIST.md) often mean “not
fully audited,” not “no source implementation.” Earlier historical sections may
say a later-verified step is pending; use dated results and exact job/source IDs.

## Known limitations and documentation corrections

- Only Windows x64 is distributed and validated here. Exact consumer Windows
  version coverage is missing; Linux/macOS package validation remains open.
- No publisher signature, notarized Mac release, or verified installer is offered.
- The updater defaults to `dsojevic/juxtopposed-vlc` in
  [github_release_service.hpp](../../modules/gui/qt/util/github_release_service.hpp).
  [release_info.cpp](../../modules/gui/qt/util/release_info.cpp) also validates
  that repository's URLs and `vlc-juxtopposed-*` assets. The dated preview does
  not match its semantic tag scheme. Use manual downloads from this fork.
- The main repository defaults to `master`; redesign development is on
  `codex/vlc-redesign`. These documentation changes must be integrated into the
  public landing branch before that page or its default issue chooser improves.
- Existing workflows lack general PR triggers. No checks on a PR means untested,
  not passed. New templates become available through GitHub after reaching the
  default branch; plain issues and the Markdown instructions remain alternatives.
- The Linux/macOS and multi-platform release guides came from the integrated
  project. Their old platform results, filenames and release plans are retained
  as reference, not presented as this fork's completed work.

## Screenshots and attribution review

The three images in [doc/images/redesign](../images/redesign) were inspected:
`player.png`, `sidebar.png`, and `video-settings.png`, all 1920 × 1080.
Git history attributes their updates to David Sojevic, including `b3e588ef`.
They show an actual native app, but they are inherited reference captures.
The sidebar image shows an empty library; it is not evidence of successful indexing.
No new screenshot or recording is claimed by this documentation update.

Retained credits name Juxtopposed's design under CC BY 4.0 and Inter under OFL 1.1.
`COPYING`, `COPYING.LIB`, file headers, AUTHORS, THANKS, and bundled notices were
reviewed and remain unchanged. The software licenses do not establish permission
to imply VideoLAN endorsement. The public name is documentation-only here;
executables, icons, package IDs and existing downloads were not changed.

The video page/oEmbed and Figma page could not be retrieved by the research tool;
no video transcript, detailed video review, or new design-license verification is
claimed. Attribution follows the repository's existing source-linked record.
Juxtopposed's site and the CC BY 4.0 deed were reachable. VideoLAN's legal page
returned HTTP 418; no new trademark authorization is inferred.

## Checks for this documentation change

- The source-version validator reports `4.0.0-beta.4`.
- All 16 release-tool unit tests passed on Python 3.14 on Windows after directing
  `TEMP`/`TMP` to an ignored workspace directory. The first attempt failed on
  sandbox temporary-directory permissions, not a packaging assertion.
- Markdown local targets/anchors and balanced code fences were checked across
  all changed/new Markdown files; the Git diff was checked for whitespace errors.
- Existing workflows and license files were left unchanged. No new native
  compilation, full Qt suite run, platform certification, or release was performed.
