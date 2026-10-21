# Try VLC Reimagined

[Back to the project](../../README.md) · [Status and evidence](STATUS.md)

## Choose a download

The verified download is the **Windows x64 portable preview**, release
[`redesign-preview-2026.10.20`](https://github.com/Uvaisbugh/vlc/releases/tag/redesign-preview-2026.10.20).
It is about 100 MB compressed and includes its runtime DLLs and plugins.
You need a writable extraction folder with space for the unpacked files, and a
local audio/video file to try. You do not need Git, Python, Qt, or a compiler.

Windows x64 describes the package architecture, not a promise about every Windows
version. Automated runtime checks used Windows Server 2022. Prior local Windows
checks are recorded without an exact OS version; Windows 10/11 version-by-version
compatibility remains to be documented. Windows ARM64 and 32-bit installations
are not validated. There are no verified Linux or macOS downloads from this fork.

## First launch

1. Download **[vlc-redesign-windows-x64-preview.zip](https://github.com/Uvaisbugh/vlc/releases/download/redesign-preview-2026.10.20/vlc-redesign-windows-x64-preview.zip)**.
   GitHub's automatically generated **Source code** archives contain source, not
   the ready-to-run Windows app.
2. In File Explorer, right-click the ZIP and choose **Extract All**. Pick a new
   writable folder, such as a folder under Downloads. Do not run the app inside
   the ZIP or copy it over your installed VLC.
3. Open the extracted **VLC Redesign** folder and double-click **vlc.exe**.
   Keep `portable`, `plugins`, DLLs, and the other bundled files beside it.
4. Follow any onboarding shown by the app. Choose a local file through the app
   menu's open-file action, or use Ctrl+O. Try play/pause and the seek bar.
   For a library screenshot, add only a folder of media you intend to use for testing.

The current app and folder may say **VLC Redesign**, **VLC media player**, or use
inherited community names. **VLC Reimagined** is the project's public-facing name;
the existing binary was not rebuilt or renamed for this documentation update.
No installer or administrator access is required. The `portable` folder keeps
the preview profile separate from your normal VLC settings.

The executable is unsigned, so Windows may display a publisher warning. Check
that the download came from the linked repository and that its checksum matches.
If your machine or organization blocks unsigned apps, use official VLC or build
from source under your own policy; do not disable security protections.

## Optional integrity check

Download [SHA256SUMS.txt](https://github.com/Uvaisbugh/vlc/releases/download/redesign-preview-2026.10.20/SHA256SUMS.txt)
from the same release. In PowerShell, in the folder containing the ZIP:

```powershell
Get-FileHash -LiteralPath '.\vlc-redesign-windows-x64-preview.zip' -Algorithm SHA256
```

Compare the resulting hash with the line for that ZIP in `SHA256SUMS.txt`.
For the inspected release, it is:

```text
468c968e12503bb6723ebae05f5ef44f7204d109d5bc9c8e0e0e25794d994ab1
```

A matching hash checks file integrity; it is not a publisher signature.
The release also provides `BUILD_INFO.json` and matching source
`vlc-redesign-source-0279317b.tar.gz`.

## Updates, rollback, and removal

Use [this repository's Releases page](https://github.com/Uvaisbugh/vlc/releases)
manually. The inherited updater still points to David Sojevic's repository and
expects different tag and package names; it is not a VLC Reimagined update path.

For a future preview, read its platform and validation notes first. Extract it
into a separate folder and keep the previous folder for rollback. Start with the
new package's own profile; profile migration between previews has not been
verified. If needed, close the new copy and open the older copy from its folder.

To remove a portable copy, close it and delete its extracted folder when you no
longer need its settings. Back up the `portable` folder first if you want to keep
that profile. This does not uninstall a separate installed VLC or delete media
stored elsewhere.

## Troubleshooting

| Symptom | What to try / report |
| --- | --- |
| There is no `vlc.exe` | Check that you downloaded the named Windows ZIP, not a source archive, and extracted it fully |
| A DLL/plugin is missing or the app will not open | Extract the whole archive into a new folder, retain the bundled files, and compare the checksum. Report the exact error; do not download individual DLLs from unrelated sites |
| Launch is blocked by a publisher/security warning | The preview is unsigned. Follow your machine's policy; signing is not provided by this release |
| A different VLC copy opens | Close other VLC windows and open `vlc.exe` from the preview folder. Include the full executable path in your private diagnosis and a sanitized path in a public report |
| The library is empty | Playing a file is not evidence that a folder has been indexed. Add a small test-media folder through the library UI and allow indexing; report persistent failure. Some source-build recipes explicitly disable the library |
| No picture, no sound, or a crash | Record OS version, CPU architecture, GPU/driver if relevant, media container/codecs and steps. Try a second local file. The smoke tests do not certify every GPU, codec or speaker output |
| Online subtitles do not work | The optional provider key is not guaranteed in this preview. See [Qt HACKING](../../modules/gui/qt/HACKING); local subtitle files can be used without a provider key |
| An update points to another project | Use this fork's release page manually; see the updater limitation above |

For a diagnostic log, close the preview, open PowerShell in the extracted
**VLC Redesign** folder, and run:

```powershell
& '.\vlc.exe' --no-one-instance --verbose=2 --file-logging --logfile='.\vlc-reimagined.log'
```

Reproduce the problem, then close the app before reading the log. These flags
are used by the repository's runtime checks; this is a diagnostic procedure,
not a claim that the current session reproduced your issue. Logs may contain
media paths and URLs. Remove private details before attaching relevant excerpts
to a [bug report](https://github.com/Uvaisbugh/vlc/issues/new/choose).

For the official stable application and its support channels, visit
[VideoLAN](https://www.videolan.org/vlc/).
