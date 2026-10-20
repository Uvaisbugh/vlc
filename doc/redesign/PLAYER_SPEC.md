# Player design inspection

Captured from the signed-in Figma Properties panel on 2026-10-20. This records
the inspected slice; it is not a complete inventory or a claim of visual parity.

File: https://www.figma.com/design/jrTJ63LEcUmnj4hFo2C6KX/VLC-Redesign--Community-

| Node / layer | Observed properties |
| --- | --- |
| `247:445` / Player | 1920 × 1080; vertical auto layout; space-between; background `#1B1B1B` |
| `491:1928` / Controls | Horizontal; fill width 1920; fixed height 54; top border 1; padding top/bottom 11, left/right 20; gap 20; background blur 50 |
| Controls / Frame 25 | Horizontal; hug 130 × 28; gap 8 |
| Controls / Frame 24 | Horizontal; fill width 1406; hug height 28; gap 8 |
| Controls / Frame 25 / play | Fixed width 54; hug height 28; radius 6; padding top/bottom 2, left 17, right 15; fill `Dark/Accent/Main`, `#D37918` |
| Controls / Frame 25 / play / Icon | 24 × 24; component name `play`; direction/state/value/EQ default; two-tone no |

Observed interactions on Player:

- Prototype key V opens the Add Subtitles to Playlist/Folder overlay instantly.
- Prototype key F navigates to Player - hide UI using a 300 ms ease-in-out smart
  animation. Application hotkeys need an explicit audit against VLC's existing
  bindings before changing behavior to match this prototype.

## First implementation slice

- Native `PlayButton.qml`: measured rectangular geometry, flat accent fill,
  original Figma SVG in the play state, and a rectangular pointer target.
- Existing native play/pause actions, accessibility actions, and hold-to-stop
  code paths remain wired. Interaction behavior still requires runtime testing.
- Shared default accent: `#D37918`; explicit custom/system accent configuration
  still overrides it through the existing palette code.
- Dark Window palette background: `#1B1B1B`.

The pause asset, complete inline seek/control arrangement, all other control
assets, light-theme comparison, interaction tests, and rendered comparison are
pending. The existing pause glyph is retained until its design asset is captured.

## Export evidence

`modules/gui/qt/pixmaps/redesign/play.svg` was exported directly through Figma's
SVG export control, copied without modification, and registered in `assets.qrc`.
Its root dimensions are 24 × 24 with viewBox `0 0 24 24`.

SHA-256:
`1c9eaea50ee36b647a9c30b8a9bb6606fa92c09a2793144f51ddfc8f1d4242c7`.

Qt 6.12.0's `qmlformat` parser accepted both changed QML files, and `rcc --list`
resolved the new SVG resource. This verifies syntax/resource registration only;
it does not establish a successful VLC build or visual/functional parity.
