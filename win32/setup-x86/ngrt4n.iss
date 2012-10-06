#define AppVersion "2.1.0beta1"
[Setup]
AppName=RealOpInsight
AppVersion={#AppVersion}
DefaultDirName={pf}\NGRT4N
DefaultGroupName=NGRT4N Monitoring Suite (RealOpInsight)
LicenseFile=COPYING     
OutputDir=output                 
OutputBaseFilename=RealOpInsight{#AppVersion}-wininst
WizardSmallImageFile=icons\icon.bmp
WizardSmallImageBackColor=$ffba64
WizardImageFile=icons\logo-roi.bmp 
WizardImageStretch = false
WizardImageBackColor=$FFFFFF  
                                              
                                                            
[Files]
Source: "C:\Dev\qtbuild\release\ngrt4n-manager.exe" ; DestDir: "{app}"      
Source: "C:\Dev\qtbuild\release\ngrt4n-oc.exe" ; DestDir: "{app}"  
Source: "C:\Dev\qtbuild\release\ngrt4n-editor.exe" ; DestDir: "{app}"
Source: "qtdeps\*" ; DestDir: "{app}"   
Source: "zmqdeps\*" ; DestDir: "{app}"             
Source: "gvdeps\*" ; DestDir: "{app}"
Source: "examples\*" ; DestDir: "{app}\examples" 
Source: "icons\icon.ico" ; DestDir: "{app}\icons"         
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
                                                              
[Icons]                    
Name: "{group}\Configuration Manager"; Filename: "{app}\ngrt4n-manager.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\Operations Console"; Filename: "{app}\ngrt4n-oc.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\Editor"; Filename: "{app}\ngrt4n-editor.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{group}\Uninstall NGRT4N"; Filename: "{uninstallexe}"
Name: "{commondesktop}\NGRT4N Operations Console"; Filename: "{app}\ngrt4n-oc.exe"; IconFilename: "{app}\icons\icon.ico";
Name: "{commondesktop}\NGRT4N Editor"; Filename: "{app}\ngrt4n-editor.exe"; IconFilename: "{app}\icons\icon.ico";
        
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
Root: HKCU; Subkey: "Software\NGRT4N Project"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\NGRT4N Project\NGRT4N"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\NGRT4N Project"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\NGRT4N Project\NGRT4N"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\NGRT4N Project\NGRT4N\Settings"; ValueType: string; ValueName: "Path"; ValueData: "{app}"
