# VLC Reimagined community roadmap

Status reviewed on 2026-10-21. This is a list of genuine remaining work, not a
release schedule or a list of assigned issues. No completion dates are promised.
The [status record](doc/reimagined/STATUS.md) separates source implementation,
automated checks, and runtime evidence.

**Implemented** means code exists. **Verified** applies only to the stated checks.
**In progress** below describes the repository's partial audit, not an assertion
that a contributor is currently assigned. **Planned** work has no completion claim.

| Area | Implemented / verified baseline | Remaining work and completion evidence | Priority / state |
| --- | --- | --- | --- |
| Visual design and usability | Native player, library, queue, settings, theme and customization components; focused tests | Complete the Figma frame/state inventory; compare actual captures at matching size/theme/scale and record deviations. Test empty/error/hover/focus states | P1 / audit in progress |
| Playback and regression testing | Windows audio/video decode smoke checks; prior local play/pause, seek, subtitle, queue-display and fullscreen checks | Exercise queue advance, multiple tracks, rate, mute/volume, end-of-file, network failures, and representative media; report exact build and reproducible outcomes | P1 / broader coverage planned |
| Installation and packaging | Published unsigned Windows x64 portable ZIP, matching source and checksum | Establish a Windows consumer-OS compatibility matrix; test fresh extraction and profile isolation; document update/rollback. Validate other platforms before advertising downloads | P1 / planned |
| Accessibility and localization | Accessible properties and translatable strings exist in parts of the interface | Audit keyboard-only use, focus visibility, screen-reader announcements, reduced motion, text scaling and translated labels. Record each tested control/environment and failure | P1 / audit planned |
| Documentation and developer experience | Beginner and contributor guides, existing build scripts and CI | Verify the documented setup on a fresh development machine; capture real screenshots and add a short captioned demo; document platform-specific obstacles | P2 / planned |
| Bug fixes and platform compatibility | Integration fixes for text metrics, add-on metadata bindings and measured dialog opacity; native Qt suite passed on Linux | Reproduce new reports against the exact preview; verify mini-player/PIP transitions and library indexing/persistence; validate Linux/macOS runtime packages independently | P1 / verification planned |
| Update identity | Inherited GitHub release checker exists | Align endpoint, allowed release URLs, tag parsing, asset naming and tests with this fork. Current code points to `dsojevic/juxtopposed-vlc` and expects semantic tags/`vlc-juxtopposed-*` assets; changing only a URL is insufficient | P1 / fix planned |
| Contribution CI | Windows workflow supports manual runs and selected branch pushes; native Qt workflow is reusable | Design a bounded, read-only PR validation path with appropriate build cost and no release secrets; verify fork PR behavior before treating it as required CI | P2 / planned |

## Small first tasks

These are suggestions to turn into issues, not fabricated issue numbers or labels.

1. **Check the installation guide on one machine.** Record Windows version,
   architecture, archive hash, and whether first launch/open-file succeeds.
   Done when a reproducible report supports one row of the compatibility matrix.
2. **Audit keyboard focus in one panel.** Start with the queue or settings panel.
   Record Tab/Shift+Tab order, focus visibility, Escape behavior and inaccessible
   controls. Done when the result includes build, scale and exact reproduction steps.
3. **Capture one current preview screenshot.** Use shareable media and readable
   window dimensions. Done when a PNG, alt text, release identity and capture
   conditions distinguish it from inherited reference screenshots.
4. **Review one translated screen.** Note locale, clipped labels, and untranslated
   strings. Done when each observation names the control and includes evidence;
   suggest wording without claiming a complete localization audit.
5. **Improve one development instruction.** Reproduce a command in the documented
   environment and clarify a missing prerequisite. Done when the guide explains
   both the successful path and any environment-specific limitation.

See [CONTRIBUTING.md](CONTRIBUTING.md) before starting. Complex updater, playback,
or packaging changes should be discussed before implementation.

## Evidence required before expanding support

For each candidate package: record the exact source/dependency identity; compile;
run appropriate automated tests; extract and launch on the target OS; check real
audio/video decoding and output; exercise the changed UI paths; retain licenses,
matching source, logs and checksums. Visual parity and accessibility remain
separate audits. Use [build/release validation](doc/redesign/BUILD_AND_RELEASE.md)
and the [release tooling reference](doc/RELEASING.md).

Signing/notarization, installers, additional architectures, and automatic
installation are future decisions. Existing packaging scripts are not evidence
that those products have shipped or been validated for VLC Reimagined.
