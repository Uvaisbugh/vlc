; SPDX-License-Identifier: GPL-2.0-or-later
; Compile with /DRuntimeDir=... /DOutputDir=... /DPreviewVersion=2026.10.21
#ifndef RuntimeDir
  #error RuntimeDir must identify the verified staged runtime
#endif
#ifndef OutputDir
  #error OutputDir is required
#endif
#ifndef PreviewVersion
  #error PreviewVersion is required
#endif

[Setup]
AppId={{65855656-0D2A-4A27-BDB2-43997E047B1B}
AppName=VLC Reimagined
AppVersion={#PreviewVersion}
AppVerName=VLC Reimagined {#PreviewVersion} Preview
AppPublisher=VLC Reimagined community
AppPublisherURL=https://github.com/Uvaisbugh/vlc
AppSupportURL=https://github.com/Uvaisbugh/vlc/issues
AppUpdatesURL=https://github.com/Uvaisbugh/vlc/releases
DefaultDirName={localappdata}\Programs\VLC Reimagined
DefaultGroupName=VLC Reimagined
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
MinVersion=10.0.17763
OutputDir={#OutputDir}
OutputBaseFilename=vlc-reimagined-windows-x64-setup
LicenseFile={#RuntimeDir}\COPYING
InfoBeforeFile={#RuntimeDir}\START_HERE.txt
UninstallDisplayIcon={app}\vlc.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupLogging=yes

[Tasks]
Name: desktopicon; Description: "Create a desktop shortcut"; Flags: unchecked

[Dirs]
Name: "{app}\portable"; Flags: uninsneveruninstall

[Files]
Source: "{#RuntimeDir}\*"; DestDir: "{app}"; Excludes: "portable\*"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{userprograms}\VLC Reimagined"; Filename: "{app}\vlc.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\VLC Reimagined"; Filename: "{app}\vlc.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\vlc.exe"; Description: "Open VLC Reimagined"; Flags: nowait postinstall skipifsilent
