# Contributing to VLC Reimagined

Thanks for helping make this unofficial VLC fork easier to use and understand.
Documentation, testing, accessibility observations, translations, and design
feedback are useful contributions alongside code. Small, focused changes are welcome.

## Start without a build

Try the [Windows preview](doc/reimagined/GETTING_STARTED.md), read the
[known limitations](doc/reimagined/STATUS.md), or pick a task from the
[roadmap](ROADMAP.md). Suggested first contributions include:

- Walk through the setup or ZIP instructions on one Windows x64 machine and record the
  exact OS version, extraction path, and result.
- Check keyboard navigation through one player panel; document focus order,
  visible focus, and any unreachable control.
- Capture one screenshot from the preview using your own or redistributable
  media; include the release, viewport, scale, and theme.
- Improve one unclear instruction or report one clipped translated label.

These are proposed tasks, not already-created or assigned GitHub issues.
Search existing issues first; open an issue before starting a large or overlapping change.

## Report a bug

Use the [bug report template](https://github.com/Uvaisbugh/vlc/issues/new/choose).
Until these templates reach the default branch, use a
[plain issue](https://github.com/Uvaisbugh/vlc/issues/new) with the same details.
Include what you expected, what happened, and the shortest steps that reproduce it.
Please include the release tag or commit, OS version and CPU architecture. For UI
bugs, add the theme, display scale, window size, and a screenshot if useful.
For playback issues, describe the container/codecs and whether the media is local
or streamed. A small sample you can legally share is helpful, but not required.

Remove private filenames, usernames, stream credentials, tokens, and unrelated
content from logs and recordings. Never upload private media just to satisfy a
template. If practical, say whether the same problem happens in official VLC;
this comparison is helpful, not a prerequisite for reporting a fork bug.

Keep fork-specific reports here. For an independently reproduced upstream bug,
use [VideoLAN's tracker](https://code.videolan.org/videolan/vlc/-/issues) and
cross-link the relevant report rather than filing duplicates.

## Make a change

The redesign is integrated into **`master`**. Fork the repository on GitHub, then
replace `YOUR_USERNAME` in these commands with your account:

```sh
git clone --branch master https://github.com/YOUR_USERNAME/vlc.git vlc-reimagined
cd vlc-reimagined
git remote add project https://github.com/Uvaisbugh/vlc.git
git switch -c codex/my-change
```

If your fork is behind, add the `project` remote above, then run
`git fetch project master` and create your contribution branch from
`project/master`. Keep complete history for dependency
selection; if you used a shallow clone, run `git fetch --unshallow` first.

See [DEVELOPMENT.md](doc/reimagined/DEVELOPMENT.md) for build, test, and debugging
instructions. Documentation-only work does not require a native compiler.

## Coding and design conventions

- Follow nearby code and [Qt HACKING](modules/gui/qt/HACKING). Qt C++ uses four
  spaces, no tabs or trailing spaces, and `override` for overrides. QML files
  include compact existing styles; avoid unrelated formatting changes.
- Reuse the existing player/library controllers and theme components. Keep
  user-facing QML text translatable with `qsTr()` as nearby code does.
- Register new sources and resources in the applicable Meson and Automake lists
  and `assets.qrc`; do not leave one supported build path out.
- Include keyboard access, focus visibility, accessible names, and translated
  text sizing in UI review. A screenshot alone does not establish accessibility.
- Preserve author/SPDX headers, license files, design credits, and font notices.
  Identify the source and license of any new asset and distinguish a concept
  image from an application screenshot.

## Testing expectations

| Change | Useful validation |
| --- | --- |
| Documentation/templates | Check paths, Markdown links, examples, and `git diff --check`; a full VLC build is unnecessary |
| Qt/QML behavior | Run the affected native Qt test(s), and the full Qt suite when feasible; record the environment and any tests not run |
| Visible interface | Exercise the actual app; include before/after captures at the same scale and theme for visual changes |
| Playback/backend | Add or run a meaningful regression check and test the affected media path; compilation alone is insufficient |
| Packaging | Run release-tool tests and validate the extracted package on its target OS, including a path with spaces |

Do not claim a check passed unless it ran successfully. If a full build is beyond
your machine, say so in the PR and provide the checks you could perform.
Maintainers can arrange the relevant CI checks; newcomers need no release keys.

## Open a pull request

Push your contribution branch to your own fork when ready, then open a PR with
**base `Uvaisbugh/vlc:master`**. Explain the user problem, the resulting
behavior, and your validation. Link an issue if there is one; a small documentation
fix does not need an issue first. Use the PR template and keep changes focused.

The current workflows do not automatically validate every pull request. A PR
with no CI result must not be treated as tested. See the development guide for
the existing workflow triggers and maintainer validation paths.

Discuss changes respectfully and report reproducible observations. Do not publish
releases or move tags as part of a normal contribution. Changes intended for
official VLC follow [VideoLAN's merge-request process](https://code.videolan.org/videolan/vlc/)
instead of this fork's PR process.
