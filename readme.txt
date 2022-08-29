MoonShell2 Tools
Here are tools for utilize MoonShell2.

# These tools uses modified libfat for treating Unicode. Be careful.

mshl2wrap: Workaround for launching commercial roms MoonShell (>= 2.07)
yslaunch: ysloader extlink which accepts multibyte characters
ysboot: AUTO_BOOT in YSMenu.ini clearer
dldicaptor: makes dldi FILE from dldi ON MEMORY

linktemplate: .nds with no execution code; ONLY USE WITH nds.mshl2wrap.nds IN EXTLINK. But only 6208 bytes

Changelog:
0.00.091208alpha2
Using Chishm's NDS_loader.
Partial extlink support.
Can get Unicode filename using modified libfat.

0.00.091209beta1
Using reset_mse_06b_for_ak2 i.e. akmenu4.nds launcher.
Complete extlink / LFN support.
Succeeded to launch commercial rom on moonshell 2.08 via external link.

0.00.091210beta2
INI configuration / dldi patching support.
Can choose homebrew loader from selected loader, moonshell(_hn.HugeNDSLoader.nds) or this wrapper itself.

0.00.091211beta3
Now can be used both as Extlink Wrapper and as Link Launcher.

0.01.091212
Created linkage tool.

0.01a.091231
Fixed ndslink.bat. Now space can be inserted in filenames.

0.01b.100102
Fixed ndslink.
Now recursive nds search is implemented in ndslink.
Link name is written in UTF8. 2byte issue solved (not tested though).

0.02.100104
Fixed license issue.
Fixed dldi issue (ysloader didn't work, very sorry).
Due to my bad programming skill, dldi patching is done to disk, not to memory.
i.e. dldi patching makes write access to your microSD. Be careful and very sorry.
#As for nds.akaio.nds, it doesn't have DLDI section, so no write access.

0.03.100109
Fixed dldi issue again.
The dldi patching now only reads from / writes to memory. Much safer for your microSD.
A little speedup.
* new tool: ysboot

0.04.100110
Fixed multibyte issue again.
For fix, I use original UTF8 <=> UTF16 conversion routine (wcstombs and so on are broken). Be careful.
* new tool: yslaunch

0.05.100116
The method to get dldi size has changed (Based on DLDITool 0.32.4).
linkmaker added to mshl2wrap makefile. Now modifying NDS header to configure linkage is done automatically (for devs).
Fixed: multibyte filenames cannot be used with "nds.mshl2wrap.nds as extlink".
Fixed: "loader" cannot contain multibyte characters.
If this is launched for commercial roms, it launches "loader". This will be useful as loader selecter(you only have to modify mshl2wrap.ini), even after Normmatt's MoonShell patch released.
* ysboot is now configurable via ysboot.ini.
* new tool: DLDI Captor
* new linktemplate
* Read warning_about_mshl2wrap_configuration.txt twice if you use ndslink.

0.06.100123
Shows DLDI Name before initializing libfat to confirm DLDI is correctly patched.
Restores DLDI Magic string before patching.
Checking DLDI size fixed: relied on DO_driverSize.
Stripped more source code from reset_mse_06b for ak2.
Checks content of DO_startup and if nullified try to use moonshl2/dldibody.bin.
On some adapters: now can correctly be launched from softwares using Chishm's NDS_loader library such as DSision (but freezes on launching commercial roms. Certainly NDS_loader breaks something. Actually not usable).
* This version is confirmed to work on M3Sakura DLDI (if you want don't put "nds.mshl2wrap.nds as extlink" in /moonshl2/extlink).
* Now ttmenu.dat is patched using boyakkey's, captured by DLDI Captor.

0.07.100126 (Fatal Fix release)
Fixed: Converting UTF16 to UTF8 failed (Fatal)
* minIni no longer use rename() to write back ini.

0.08.100201 (Fatal Fix)
Fixed: DLDI might not be loaded correctly from file if you use multiple DLDIs (Fatal)
If AKAIO DLDI is used  in yslaunch/ysboot, try to load /__ak2/ak2_sd.dldi.
* License fixed. Now all other than DLDITool/minIni are in public domain.
* Document fixed.

0.09.100207 (Fatal Fix)
Fixed: DLDI Patching was incomplete from 0.05.
Now dldi.c is rewritten. You can use the whole source in proprietary software (Creative Commons CC0 other than minIni).
* Now yslaunch/ysboot can treat CRLF ini correctly (but minIni is no longer thread safe. OK in NDS).

0.09a.100210 (Fatal)
Fixed a nasty bug in dldipatch (dldi.c). Very sorry.

0.09b.100228
dldipatch bug fixed.
Link Launcher will halt if loader contains /MOONSHL2/EXTLINK/NDS
* If yslaunch/ysboot is launched on R4DS, it will change DLDI ID to RPGS, which will be accepted by YSMenu.
* mshl2alt merged to reset_mse

100609 Diff
dldipatch another bug fixed.

P.01.100703
Modified libfat to drop flashcart support other than DLDI. libfat initialization became faster.
