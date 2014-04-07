#define Version "3.0.0b1"
#define Application "RealOpInsight Workstation" 
[Setup] 
AppName={#Application}
AppVersion={#Version}
DefaultDirName={pf}\RealOpInsight Workstation
DefaultGroupName=RealOpInsight Workstation
LicenseFile=COPYING                                  
OutputDir=output                  
OutputBaseFilename=realopinsight-workstation-{#Version}-wininst
WizardSmallImageFile=icon.bmp
WizardSmallImageBackColor=$ffba64
WizardImageFile=realopinsight.bmp          
WizardImageStretch = false       
WizardImageBackColor=$FFFFFF
                                                   
                                                                    
[Files]
Source: "C:\Dev\qt-build\release\ngrt4n-manager.exe" ; DestDir: "{app}"      
Source: "C:\Dev\qt-build\release\ngrt4n-oc.exe" ; DestDir: "{app}"  
Source: "C:\Dev\qt-build\release\ngrt4n-editor.exe" ; DestDir: "{app}"
Source: "deps\*.dll" ; DestDir: "{app}"          
Source: "C:\Program Files (x86)\Graphviz2.36\bin\*" ; DestDir: "{app}"    
                            
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Core.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Gui.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Network.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Script.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5WebKit.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5WebKitWidgets.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Widgets.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Xml.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\icuin51.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\icuuc51.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\icudt51.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Sql.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Qml.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Quick.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Positioning.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Multimedia.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Sensors.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5OpenGL.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5PrintSupport.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5MultimediaWidgets.dll" ; DestDir: "{app}"
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\Qt5Svg.dll" ; DestDir: "{app}"

Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\libstdc++-6.dll" ; DestDir: "{app}"    
Source: "C:\Qt-5.2\5.2.1\mingw48_32\bin\libwinpthread-1.dll" ; DestDir: "{app}"

Source: C:\Dev\qt-build\release\*  ; DestDir: "{app}" 
Source: "..\examples\small_hosting_platform.*.ngrt4n.xml" ; DestDir: "{app}\examples" 
Source: "icon.ico" ; DestDir: "{app}\icons"         
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
Source: "..\i18n\ngrt4n_*.qm" ; DestDir: "{app}\i18n" 
                                                                
[Icons]                    
Name: "{group}\{#Application} Configuration Manager"; Filename: "{app}\ngrt4n-manager.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\{#Application} Operations Console"; Filename: "{app}\ngrt4n-oc.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\{#Application} Editor"; Filename: "{app}\ngrt4n-editor.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\Uninstall RealOpInsight"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#Application} Operations Console"; Filename: "{app}\ngrt4n-oc.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{commondesktop}\{#Application} Editor"; Filename: "{app}\ngrt4n-editor.exe"; IconFilename: "{app}\icons\icon.ico";
        
[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
    
; NOTE: Most apps do not need registry entries to be pre-created. If you
; don't know what the registry is or if you need to use it, then chances are
; you don't need a [Registry] section.
                                              
[Registry]             
; Start "Software\My Company\NGRT4N" keys under HKEY_CURRENT_USER
; and HKEY_LOCAL_MACHINE. The flags tell it to always delete the
; "NGRT4N" keys upon uninstall, and delete the "My Company" keys
; if there is nothing left in them.
Root: HKCU; Subkey: "Software\RealOpInsight Labs"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\RealOpInsight Labs\RealOpInsight Workstation"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\RealOpInsight Labs"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\RealOpInsight Labs\RealOpInsight Workstation"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\RealOpInsight Labs\RealOpInsight Workstation\Settings"; ValueType: string; ValueName: "Path"; ValueData: "{app}"
