; �ű��� Inno Setup �ű��� ���ɣ�
; �йش��� Inno Setup �ű��ļ�����ϸ��������İ����ĵ���

#define MyAppName "������Ƶ������"
#define MyAppName_EN  "Mago Video Player"
#define MyAppVersion "1.0.0.3"
#define MyAppPublisher_CN "�Ϻ��������缼�����޹�˾"
#define MyAppPublisher_EN "Shanghai Mago Network Technology Co.,Ltd."
#define MyAppURL "http://www.shmgwl.com/"
#define MyAppExeName "MagoPlayer.exe"

[Setup]
; ע: AppId��ֵΪ������ʶ��Ӧ�ó���
; ��ҪΪ������װ����ʹ����ͬ��AppIdֵ��
; (�����µ�GUID����� ����|��IDE������GUID��)
AppId={{D956FCD4-01F1-4E39-98A7-81423261B203}
AppName={cm:MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
;AppPublisher={#MyAppPublisher}
AppPublisher={cm:AppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\MagoPlayer
DisableProgramGroupPage=yes
OutputDir=.\
OutputBaseFilename=MagoPlayer_{#MyAppVersion}
SetupIconFile=.\MagoPlayer.ico
Compression=lzma
SolidCompression=yes
UsePreviousAppDir=No

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\english.isl"

[Messages]
english.BeveledLabel=englishlish
chinesesimp.BeveledLabel=Chineses

[CustomMessages]
chinesesimp.MyAppName={#MyAppName} 
chinesesimp.Type="zh_CN"
chinesesimp.Title="������Ƶ������" 
chinesesimp.AppPublisher={#MyAppPublisher_CN}
english.MyAppName={#MyAppName_EN}
english.Type="en_US"
english.Title="MagoPlayer"
english.AppPublisher={#MyAppPublisher_EN}


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone; OnlyBelowVersion: 0,8.1

[Files]
Source: ".\vc2008redist_x86.exe"; DestDir: "{app}"; Flags: ignoreversionSource: ".\vc2013redist_x86.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\Files\*"; DestDir: "{app}\"; Flags: ignoreversion recursesubdirs createallsubdirs

; ע��: ��Ҫ���κι���ϵͳ�ļ���ʹ�á�Flags: ignoreversion��


[Icons]
Name: "{commonprograms}\{cm:MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{cm:MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
;���ݲ�ͬ�����Ի�����ʾ��Ӧ�Ŀ�ݷ�ʽ
Filename: "{app}\vc2008redist_x86.exe";Parameters:"/q";Flags: postinstall waituntilterminated unchecked;
Filename: "{app}\vc2013redist_x86.exe";Parameters:"/q";Flags: postinstall waituntilterminated unchecked;

