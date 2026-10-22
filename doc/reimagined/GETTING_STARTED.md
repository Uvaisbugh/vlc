# Try VLC Reimagined

[Back to the project](../../README.md) · [Status and evidence](STATUS.md)

## Choose a download

The current Windows x64 preview provides two options:

- **[Windows Setup EXE — recommended](https://github.com/Uvaisbugh/vlc/releases/download/reimagined-preview-2026.10.22/vlc-reimagined-windows-x64-setup.exe):**
  an installation wizard, Start menu shortcut, optional desktop shortcut, and uninstaller.
- **[Portable ZIP](https://github.com/Uvaisbugh/vlc/releases/download/reimagined-preview-2026.10.22/vlc-reimagined-windows-x64-portable.zip):**
  extract the whole folder and run the app without installation.

[Release notes, matching source, and checksums](https://github.com/Uvaisbugh/vlc/releases/tag/reimagined-preview-2026.10.22).
Both include runtime DLLs and plugins. You need free space for the program and a
local audio/video file to try. You do not need Git, Python, Qt, or a compiler.

Windows x64 describes the package architecture, not a promise about every Windows
version. Automated runtime checks used Windows Server 2022. Prior local Windows
checks are recorded without an exact OS version; Windows 10/11 version-by-version
compatibility remains to be documented. Windows ARM64 and 32-bit installations
are not validated. There are no verified Linux or macOS downloads from this fork.

## Install with Setup

1. Download **vlc-reimagined-windows-x64-setup.exe** from the link above.
2. Run it, read the preview information, and follow the installation wizard.
   The default location is `%LOCALAPPDATA%\Programs\VLC Reimagined` for your account.
3. Keep the default separate folder. A desktop shortcut is optional; a Start menu
   shortcut is included. The installer requests no administrator privileges.
4. Open **VLC Reimagined** from the Start menu. Follow any onboarding shown,
   choose a local file from the app menu or Ctrl+O, then try play/pause and seeking.

Official VLC and default file associations are left alone. The program and its
installer are called **VLC Reimagined**; some inherited interface text still says
**VLC media player**. The separate `portable` profile inside this installation
keeps its settings apart from your normal VLC installation.

## Use the portable ZIP

1. Download **vlc-reimagined-windows-x64-portable.zip**. GitHub's automatically
   generated **Source code** archives are for developers, not the runnable app.
2. Right-click the ZIP in File Explorer and choose **Extract All** into a new
   writable folder. Do not run directly inside the ZIP or overwrite installed VLC.
3. Open **VLC Reimagined**, then double-click **vlc.exe**. Keep `portable`, DLLs,
   plugins, and the other bundled files together. No installer is required.

The executable is unsigned, so Windows may display a publisher warning. Check
that the download came from the linked repository and that its checksum matches.
If your machine or organization blocks unsigned apps, use official VLC or build
from source under your own policy; do not disable security protections.

## Optional integrity check

Download [SHA256SUMS.txt](https://github.com/Uvaisbugh/vlc/releases/download/reimagined-preview-2026.10.22/SHA256SUMS.txt)
from the same release. In PowerShell, in the folder containing your download:

```powershell
Get-FileHash -LiteralPath '.\vlc-reimagined-windows-x64-setup.exe' -Algorithm SHA256
```

Compare the hash with the line for that filename in `SHA256SUMS.txt`. For the ZIP,
replace the EXE filename in the command with the portable ZIP filename.

A matching hash checks file integrity; it is not a publisher signature.
The release also provides `BUILD_INFO.json`, `VALIDATION.txt` and matching source
`vlc-reimagined-source.tar.gz`, including packaging scripts and dependency recipes.

## Updates, rollback, and removal

Use [this repository's Releases page](https://github.com/Uvaisbugh/vlc/releases)
manually. The inherited updater still points to David Sojevic's repository and
expects different tag and package names; it is not a VLC Reimagined update path.

Close the app before running a future installer. Reinstalling this preview
preserves its profile; migration across future versions is not yet certified.
Back up the installation's `portable` folder if you want to retain a copy of its
settings. For portable updates, extract each preview into a separate folder and
keep the older folder for rollback. Start with the new package's own profile.

For a setup installation, use **Windows Settings → Apps → Installed apps →
VLC Reimagined → Uninstall** (the Apps page name varies by Windows version).
The uninstaller removes program files and shortcuts while preserving the profile.
Delete the remaining installation folder only if you no longer need those settings.
To remove a portable copy, close it and delete its extracted folder. Media stored
elsewhere and a separate official VLC installation are unaffected.

The [older portable preview](https://github.com/Uvaisbugh/vlc/releases/tag/redesign-preview-2026.10.20)
remains available for rollback; its folder is named **VLC Redesign**.

## Troubleshooting

| Symptom | What to try / report |
| --- | --- |
| There is no `vlc.exe` | Use the setup EXE to install, or fully extract the named portable ZIP. A source archive is not a runnable application |
| A DLL/plugin is missing or the app will not open | Run setup again to repair its program files, or extract the whole ZIP into a new folder. Compare the download checksum and report the exact error; do not download individual DLLs from unrelated sites |
| Launch is blocked by a publisher/security warning | The preview is unsigned. Follow your machine's policy; signing is not provided by this release |
| A different VLC copy opens | Close other VLC windows and open `vlc.exe` from the preview folder. Include the full executable path in your private diagnosis and a sanitized path in a public report |
| The library is empty | Playing a file is not evidence that a folder has been indexed. Add a small test-media folder through the library UI and allow indexing; report persistent failure. Some source-build recipes explicitly disable the library |
| No picture, no sound, or a crash | Record OS version, CPU architecture, GPU/driver if relevant, media container/codecs and steps. Try a second local file. The smoke tests do not certify every GPU, codec or speaker output |
| Online subtitles do not work | The optional provider key is not guaranteed in this preview. See [Qt HACKING](../../modules/gui/qt/HACKING); local subtitle files can be used without a provider key |
| An update points to another project | Use this fork's release page manually; see the updater limitation above |

For a diagnostic log, close the preview, open PowerShell in the installed or
extracted **VLC Reimagined** folder, and run:

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
