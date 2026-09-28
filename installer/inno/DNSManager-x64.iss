; ---------------------------------------------------------------------------
; DNS Manager — Inno Setup script (64-bit build)
;
; Produces: DNSManager-<version>-windows-x64-setup.exe
; Built in CI (.github/workflows/release.yml) after electron-builder emits
; electron-app/release/win-unpacked/. Run locally with:
;
;   iscc installer/inno/DNSManager-x64.iss /DAppVersion=2.0.0
;
; Support the project (Ethereum / EVM, MetaMask):
;   0x3f9A75Bd8bc2B4A703Ce071275D7B0ec2bED12E5
; See also: installer/inno/SUPPORT.txt (installed next to the app) and README.
; ---------------------------------------------------------------------------

#define AppName "DNS Manager"
#ifndef AppVersion
  #define AppVersion "2.0.0"
#endif
#define AppPublisher "DnsManager Contributors"
#define AppURL "https://github.com/callmeD4N13L/DNS-Manager"
#define AppExeName "DNS Manager.exe"

[Setup]
AppId={{8E2B4F6A-4D7A-4C1E-9F2A-DNSMANAGER64}}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}/releases
DefaultDirName={autopf}\DNS Manager
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
SetupIconFile=..\..\resources\win\app.ico
UninstallDisplayIcon={app}\{#AppExeName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=..\output
OutputBaseFilename=DNSManager-{#AppVersion}-windows-x64-setup
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Setup (64-bit)
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; electron-builder NSIS/portable staging dir for x64 (win-unpacked).
Source: "..\..\electron-app\release\win-unpacked\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Support / donation card (contains the MetaMask wallet address).
Source: "SUPPORT.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
