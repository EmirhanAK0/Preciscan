; Preciscan Inno Setup Script

#define MyAppName "Preciscan"
#define MyAppVersion "1.1"
#define MyAppPublisher "M2Lab"
#define MyAppExeName "preciscan_ui.exe"

[Setup]
; Temel ayarlar
AppId={{8A4A9B32-9F0A-4E14-A2B8-C5B67A8C9D0E}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; Çikti konumu
OutputDir=.\Output
OutputBaseFilename=Preciscan_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; İsteğe bağlı: Setup simgesi
SetupIconFile=..\pc\app.ico

[Languages]
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; dist klasöründeki her şeyi yükleme klasörüne kopyala
Source: "dist\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "dist\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOT: "dist" klasöründe uygulamanın çalışması için gereken LLT.dll ve Qt dll'leri olmalıdır.

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Kurulum sirasinda Arduino surucusunu sessizce kur
Filename: "{app}\CH341SER.EXE"; Parameters: "/s"; StatusMsg: "Arduino suruculeri (CH340) yukleniyor lutfen bekleyin..."; Flags: waituntilterminated skipifdoesntexist runascurrentuser

; Kurulum bittikten sonra hemen başlatma seçeneği
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
