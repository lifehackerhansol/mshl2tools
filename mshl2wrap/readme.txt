MoonShell2 Extlink Wrapper

Enables to launch commercial roms in 2.07 or later using a neat link scheme.
Different from simple copy, the link file only consumes 128KB.
Also the link can be used even without moonshell's extlink(this itself creates moonshl2/extlink.dat).
#Tested with 2564 (LEC book keeping 3rd grade)

### This is a workaround, not solution. Be careful. ###

Wondering why the filename is nds.mshl2wrap.nds? Because this prototype was developed as a extlink.

Based on reset_mse_06b_for_ak2 By Moonlight, Rudolph, kzat3
DevKitARMr20 + libnds-20070127 + libfat-20070127(modified)

Additional libraries:
minIni http://code.google.com/p/minini/
dlditool http://svn.dslinux.in-berlin.de/viewvc/dslinux/tags/dlditool/

To install:
1. Put mshl2wrap.ini (and nds.mshl2wrap.nds on your opinion) in /moonshl2/extlink/
2. Put your roms in /nds (can be changed by modifying ndslink.bat.
3. Put nds.mshl2wrap.nds and ndslink.exe/bat in your MicroSD then execute ndslink.bat.

About mshl2wrap.ini:
"hbmode" is what loader should be used for launching real Homebrew.
0==selected loader(in "loader" content) 1==moonshell-hn(experimental) 2==this wrapper itself.
"loader" is loader for commercial roms.

The link file is created using http://www.bottledlight.com/ds/index.php/FileFormats/NDSFormat.

Limitation:
1. To run ROMs in Moonshell you need to run files in the /mshl2wrap folder, not in nds folder.
2. If you add ROMs you need to run ndslink each time.
