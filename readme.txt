MoonShell2 Tools
Here are tools for utilize MoonShell2.

mshl2wrap    (Link Launcher):
	Workaround for launching commercial roms on MoonShell (>= 2.07)
inilink      (inilinker xxxloader multibyte):
	YSMenu/AKAIO/AK2/M3TouchPod extlink which accepts multibyte characters
iniclear:
	clear ini then load the menu
dldicaptor   (DLDI Captor):
	makes dldi FILE from dldi ON MEMORY
reset_mse    (MoonShellExecute Soft Reset DLDI) (only in r31):
	Very common nds reset/loader.
favlauncher  (FavLauncher) (only in r31):
	Launch NDS along with the button held.
resetproxy (only in r31):
	Launch /moonshl2/resetmse/xxxx.nds. Useful in "multicarts on one TF".
m3sakura_boot (only in r31):
	Write /system/m3sakura/dldibody.bin then launch /m3sakura.dat. Useful in "multicarts on one TF".

linktemplate: .nds with no execution code; ONLY USE WITH nds.mshl2wrap.nds IN EXTLINK (of course with this method you cannot use other launchers such as M3Sakura DLDI). But only 6208 bytes.

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
Link Launcher will halt if loader contains /MOONSHL2/EXTLINK/NDS
* If yslaunch/ysboot is launched on R4DS, it will change DLDI ID to RPGS, which will be accepted by YSMenu.
* mshl2alt merged to reset_mse

0.10.100124beta1
Now compiled using devkitARM r27. Much faster boot (approx. 5x).
As libfat 1.0.6 supports Unicode natively, patching is very little. Possibly stabler.
Fixed: filename length could not exceed 256 characters.

0.11.100124
Now getsfnlfn() works again. From this version, _FAT_open_r / _FAT_close_r is directly called.
* yslaunch and ysboot doesn't require own rename function. Much much safer!!!
* new tool: common reset.mse (Very fast reset/loader)

0.12.100126
Fixed: Converting UTF16 to UTF8 failed (Fatal)
* minIni no longer use rename() to write back ini.
* Now reset.mse can be configured via mselink.

0.13.100201
Fixed: DLDI might not be loaded correctly from file if you use multiple DLDIs (Fatal)
If AKAIO DLDI is used  in yslaunch/ysboot, try to load /__ak2/ak2_sd.dldi.
Now use libnds API for FIFO.
* License fixed. Now all other than DLDITool/minIni are in public domain.
* Document fixed.
* New tool: Select Loader X (Still Beta)

0.20.100207
Fixed: DLDI Patching was incomplete from 0.05.
Now dldi.c is rewritten. You can use the whole source in proprietary software (Creative Commons CC0 other than minIni).
* Now yslaunch/ysboot can treat CRLF ini correctly (but minIni is no longer thread safe. OK in NDS).

0.21.100210
Fixed a nasty bug in dldipatch (dldi.c). Very sorry.

0.30.100309
My modified Chishm bootlib won't prevent loading ROMs any longer!

Recompiled with devkitARMr28.
Fixed two nasty bugs in dldipatch.
Link Launcher will halt if loader contains /MOONSHL2/EXTLINK/NDS
* If yslaunch/ysboot is launched on R4DS, it will change DLDI ID to RPGS, which will be accepted by YSMenu.
* mshl2alt merged to reset_mse
* Fixed compilation option
* Now yslaunch comes with two binaries (modified MoonShell Simply loader / Chishm bootlib).
* Many many bug fixes.

0.35.100315 QuasiFinal
mshl2tools legacy support dropped.
Now that extlink rearranging implemented, you can put any nds.*.nds in /moonshl2/extlink/ as long as nds.mshl2wrap.nds is put there.
mshl2wrap.ini configuration changed. Now you can set extlink for each DLDI ID. Ready to put DSTT/AK2i on one TF!
* DLDI Captor "white out for a while" fixed (actually in 0.30)
* SelectLoader X renamed to FavLauncher and it is now configurable.
* ysboot/yslaunch now uses normal rename() for writing ini.
* Added m3sakura_boot(launch /m3sakura.dat with updating /system/m3sakura/dldibody.bin)
* and reset.mse proxy (launch /moonshl2/resetmse/[DLDIID].nds, use with MoonShellSimply addon)
* these two are useful if you want to put DSTT/AK2i on one TF!

100316 Diff
favlauncher/m3sakura_boot/reset_mse/ysboot are now can be used as akmenu4.nds. All credits go to wintermute.

0.36.100323
In M3DS, hbmode is forced to 1 if set to 0.
Now hbmode 2 is forced if (hbmode==1)&&(ARM9 address!=0x02000000). Avoids some issue in hn loader.
* yslaunch renamed to inilink. Now it can launch AKAIO/M3 TouchPod. The world's first extlink for M3DS.
* ysboot renamed to iniclear. Depends on /moonshl2/extlink/inilink.ini. ysboot.ini won't be used.
* If you are on M3DS, read inilink/readme.txt carefully or your save will be lost
* (especially, if sav>=1MB&&Kaura, lost permanently because Sakura 1.44 save restore only works for sav==512KB. Backup frequently!)

0.37.100329 Final
Rebuilt with devkitARMr30. Finalized.
* ak2loader/DScovered loader merged.

100405 Diff
* unneeded chishm.bin linked to inilink, removed
* Now favlauncher supports UseNone feature (if no buttons are pressed None= will be launched).
* But as I don't recommend it, you have to set UseNone=1 in favlauncher.ini.

100419 Diff
* inilink / iniclear now supports (my custom version of) WoodR4.

100422 Diff
* inilink / iniclear now supports WoodRPG with autorunWithLastRom. Ini isn't compatible with before so look inilink.ini carefully.

100515 Diff
* inilink / iniclear now supports R4iTT(beta).
* iniclear and reset_mse can be used on R4iSDHC(redbox).
* Added m3loader, which directly launch /system/minigame.* or /_system_/_sys_data/r4_firends.ext.

100519 Diff
* added NDSLink on DS.

0.38.100520.F2
Overall optimization using -O3.
Externalized bootlib(boot.bin) and arm7.bin. (Fixed license issue again)
#Please put chishmloader/boot.bin in / (root).
Since mshl2wrap couldn't load some loaders, now it uses moonshellreset+bootlib.
Now you can press A to shutdown when dldicaptor/ndslink is completed or rebooting is failed.
* favlauncher_bl is no longer "test".

100523 Diff
* m3loader now can load r4_homebrew.ext for a homebrew loading. ini isn't compatible with before.

0.39.100527
License is modified. Well, using Library or Program should be distinct even out of GNU talk.
#Well I don't like the ideology that "Library is included in Program", which means even Libraries should be under GPL :p
Removed two old documents.
Updated bootlib.
Installing bootstub is supported(but I don't know how people like to exit to "_BOOT_DS.NDS").
Now die() can exit to bootstub if one is installed when booting.

0.40.100528
Now Rudolph's loader is working fine again. Stripping option was wrong.

0.41.100609
Fixed another dldipatch bug(not fatal but there was a possibility to patch nds which isn't actually dldi-zed)
Optimization option modified again. Uses ARMv5TE instead of ARM9TDMI.
* Now in bootlib, Power LED blinks while loading NDS. This will lessen your irritation.
* Now dscoverloader use loader.cfg instead of loader.ext (let's copy _te to cfg.TextEdit.nds)

100626 Diff
* mshl2wrap.ini / favlauncher.ini.sample updated.
* inilink supports DEMO/R4DS/_R4i(clone) RPGN/XXXX(AKRPG/+) iTDS/R4_I(not recommended)
* dscoverloader/loader.nds now depends on /moonshl2/extlink/mshl2wrap.ini. loader.cfg isn't required any longer.

0.42.100720
* Fixed: DefaultDir in iniclear didn't work at all.
* Recompiled with devkitARMr31. libnds etc were also recompiled.

100725 Diff
* Added g6dsload.eng (only M3iZero) (directly jumps to /moonshl2/reload.dat)
* Recompiled using libnds 1.4.4

100726 Diff
* Added YSMenu for all flashcart / r4loader extlink(WoodR4 loader).
* nds.ak2loader.nds gets savename precisely; SFN can be different from nds name.
