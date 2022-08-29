XenoFile - simple and sophisticated filer

[content]
xenofile.nds          - normal program
xenofile_02000000.nds - a workaround for that EZVi/iSmartDS cannot execute homebrews whose arm9!=0x02000000
_ds_menu.dat          - bootstrap for R4
akmenu4.nds           - bootstrap for AK2
g6dsload.eng          - bootstrap for M3(eng) (M3iZero only, to use in M3Real install as menu.eng)
msforsc.nds           - bootstrap for SCDSONEi(eng)
r4.dat                - bootstrap for R4iSDHC(red)
ttmenu.dat            - bootstrap for DSTT

[source]
You need these to compile:
* XenoFile source folder
* mshl2tools arm7/libprism
* modified libfat
* utility: png2b15/zlibrawstdio (in breaksplash package)

[control]
 Cross : Cursor (Left/Right: move 10)
     A : Chdir/Execute/Open by textedit/Select DLDI/Launch via extlink/Convert save
     B : Chdir to parent
     Y : System Menu
   L/R : Switch file filter (all/nds/supported)
 Start : Context Menu
Select : Preferences

[supported files]
.nds: Execute (homebrew/commercial) #extlink wrapper(mshl2wrap) embedded
.c/.cpp/.h/.pl/.py/.rb/.php/.txt/.ini/.log/.cfg/.conf/.htm/.html/.lst: Open by textedit
.sav/.bak/.duc/.dsv/.gds: duc(ARDS MAX)/duc(ARDS ME)/dsv(desmume)/gds(gameshark) convertion
.dldi: Select DLDI to use for executing homebrew
.b15/.ani: Show image(useful for debugging MoonShell skin)

And any files supported by MoonShell extlinks!

[context menu]
Copy
Cut
Paste
Delete
Rename #You can use keyboard to set destination!
Stat
Calc hash (CRC16/CRC32/Adler32/MD5)
Run as homebrew
Run as DSBooter
Run as R4 kernel (only on R4)
Manual DLDI patch

[preferences]
Use Rudolph Loader to boot
Use MoonShell Simply Loader to boot
Use bootlib loader to boot
Use my DLDI to boot
Disable DLDI patching

[system menu]
Show partition info
Show system info
Open in text editor
Return to firmware(moonshl2)
Shutdown
