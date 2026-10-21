# VLC Reimagined — Windows x64 Preview

An unofficial community build of VLC with Juxtopposed's redesigned interface,
integrated from David Sojevic's native implementation. Uses the existing VLC
media engine. Not affiliated with or endorsed by VideoLAN or Juxtopposed.

## Download and install

**Recommended: `vlc-reimagined-windows-x64-setup.exe`**

1. Download the setup EXE in **Assets** below.
2. Run it and follow the installation wizard. It installs for your Windows account
   in a separate **VLC Reimagined** folder; administrator access is not required.
3. Open **VLC Reimagined** from the Start menu. A desktop shortcut is optional.

**Portable alternative: `vlc-reimagined-windows-x64-portable.zip`**

Use **Extract All**, open **VLC Reimagined**, then open **vlc.exe**. Keep the whole
folder together. The automatically generated GitHub source ZIP is not the app.

Both options keep the preview's profile in its own `portable` folder. The setup
provides an uninstaller through Windows Installed apps. Uninstalling preserves
profile data; remove that folder manually only if you no longer need it.
The preview does not change default file associations or replace official VLC.

## Validation and limitations

Published only after native compilation, Qt tests, Windows startup/audio/video
checks, portable extraction, installation/repair and uninstall checks succeed.
`BUILD_INFO.json` and `VALIDATION.txt` identify the exact evidence. Matching
application source and release packaging scripts are in
`vlc-reimagined-source.tar.gz`; dependency recipes, patches, URLs and checksums
are under `contrib/`. `SHA256SUMS.txt` covers the attached files.

**Windows x64 only. Unsigned preview:** Windows may display a publisher warning.
Follow your machine's policy; no code-signing certificate is supplied. Automated
runtime and installer checks use Windows Server 2022 x64. Consumer Windows
version coverage, ARM64/32-bit, hardware audio output, all codecs/devices, full
Figma parity and accessibility have not been comprehensively certified.
Linux and macOS packages are not included.

Use this repository's Releases page for manual updates. The inherited in-app
updater still targets the earlier community fork. Source application version and
the preview's date-based release tag are different identifiers.

## Credits and license

- VLC engine/application: VideoLAN and VLC contributors, GPL-2.0-or-later;
  libVLC LGPL-2.1-or-later. Retained dependency notices apply.
- Design: [VLC Redesign by Juxtopposed](https://www.figma.com/community/file/1691465999134370980/vlc-redesign),
  recorded under CC BY 4.0; [inspiration video](https://www.youtube.com/watch?v=ERFeOgqk324).
- Native redesign: [David Sojevic and contributors](https://github.com/dsojevic/juxtopposed-vlc),
  with author history and notices retained.
- Integration, validation and Windows packaging: Uvais Bughari's community fork.
- Inter: Rasmus Andersson and contributors, SIL Open Font License 1.1.

Report reproducible problems at [GitHub Issues](https://github.com/Uvaisbugh/vlc/issues).
