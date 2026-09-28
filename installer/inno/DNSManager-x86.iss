; ---------------------------------------------------------------------------
; DNS Manager — Inno Setup script (32-bit build)
;
; Produces: DNSManager-<version>-windows-x86-setup.exe
; Built in CI (.github/workflows/release.yml) after electron-builder emits
; electron-app/release/win-ia32-unpacked/. Run locally with:
;
;   iscc installer/inno/DNSManager-x86.iss /DAppVersion=2.0.0
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
AppId={{7A1C3E5B-2B6D-4A9C-8D1B-DNSMANAGER32}}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion} (32-bit)
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}/releases
DefaultDirName={autopf}\DNS Manager (32-bit)
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile=..\..\LICENSE
SetupIconFile=..\..\resources\win\app.ico
UninstallDisplayIcon={app}\{#AppExeName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=..\output
OutputBaseFilename=DNSManager-{#AppVersion}-windows-x86-setup
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Setup (32-bit)
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; electron-builder staging dir for the 32-bit (ia32) target.
Source: "..\..\electron-app\release\win-ia32-unpacked\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; Support / donation card (contains the MetaMask wallet address).
Source: "SUPPORT.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
