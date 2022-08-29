MoonShell2 Tools
Here are tools for utilize MoonShell2.

***
Currently devkitARM r23b/r27/r28/r30/r31/r32 are supported. r20-r23 requires special rename wrapper like
void rename(const char *old, const char *new){copy(old,new);unlink(old);}
***

dldicaptor     - Capture DLDI to file
mshl2wrap      - [extlink] Workaround to launch commercial roms on MoonShell >=2.07
iniclear       - Clear autoboot info
inilink        - [extlink] Set autoboot info for kernels
reset_m3       - Fast DSBooter loader
reset_mse      - Fast NDS loader
resetproxy     - reset.mse which launchs /moonshl2/resetmse/xxxx.nds
m3sakura_boot  - Write /system/m3sakura/dldibody.bin then boot m3sakura.dat
favlauncher    - A simple launcher. You can have 13 selection.
favlauncher_bl - favlauncher which uses bootlib
favlauncher_ms - favlauncher which uses moonshellreset
ak2loader      - [extlink] launch /akloader.nds (ak2loader.nds)
dscoverloader  - DSCovered general extlink wrapper
m3loader       - [extlink] launch /system/minigame.* or /_system_/_sys_data/r4_firends.ext
m3dscover      - modified m3loader for DSCovered
ndslink        - makes link file for mshl2wrap
r4loader       - [extlink] launch /__rpg/r4loader.nds
ysall          - YSMenu for all flashcart
vhbootlib      - Alternative VeryHugeNDSLoader
savbackup      - sav backupper
m3region       - show M3 cart region
nds_backup_tool_memory - You can dump DS card using NDS main memory, as another libcarddump frontend
xenofile       - simple and sophisticated filer

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

100405 Diff (0.37a)
* unneeded chishm.bin linked to inilink, removed
* Now favlauncher supports UseNone feature (if no buttons are pressed None= will be launched).
* But as I don't recommend it, you have to set UseNone=1 in favlauncher.ini.

100419 Diff (0.37b)
* inilink / iniclear now supports (my custom version of) WoodR4.

100422 Diff (0.37c)
* inilink / iniclear now supports WoodRPG with autorunWithLastRom. Ini isn't compatible with before so look inilink.ini carefully.

100515 Diff (0.37d)
* inilink / iniclear now supports R4iTT(beta).
* iniclear and reset_mse can be used on R4iSDHC(redbox).
* Added m3loader, which directly launch /system/minigame.* or /_system_/_sys_data/r4_firends.ext.

100519 Diff (0.37e)
* added NDSLink on DS.

0.38.100520.F2
Overall optimization using -O3.
Externalized bootlib(boot.bin) and arm7.bin. (Fixed license issue again)
#Please put chishmloader/boot.bin in / (root).
Since mshl2wrap couldn't load some loaders, now it uses moonshellreset+bootlib.
Now you can press A to shutdown when dldicaptor/ndslink is completed or rebooting is failed.
* favlauncher_bl is no longer "test".

100523 Diff (0.38a)
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

100626 Diff (0.41a)
* mshl2wrap.ini / favlauncher.ini.sample updated.
* inilink supports DEMO/R4DS/_R4i(clone) RPGN/XXXX(AKRPG/+) iTDS/R4_I(not recommended)
* dscoverloader/loader.nds now depends on /moonshl2/extlink/mshl2wrap.ini. loader.cfg isn't required any longer.

0.42.100720
* Fixed: DefaultDir in iniclear didn't work at all.
* Recompiled with devkitARMr31. libnds etc were also recompiled.

100725 Diff (0.42a)
* Added g6dsload.eng (only M3iZero) (directly jumps to /moonshl2/reload.dat)
* Recompiled using libnds 1.4.4

100726 Diff (0.42b)
* Added YSMenu for all flashcart / r4loader extlink(WoodR4 loader).
* nds.ak2loader.nds gets savename precisely; SFN can be different from nds name.

0.50.100801
Now mshl2tools can be compiled in both r31 and r23b (Of course, r23b comes without bootstub support).
Now informations are written both in Main and Sub screen.
Added M3DS reset.

0.50a.100820
Updated libnds to 1.4.5.
Added savbackup. (libprism also updated)

0.51.100823
Fixed a bug that lower screen didn't work.
savbackup.ini uses scandir and bakdir. Not compatible with before.
savbackup writes copy progress to lower screen.
Now favlauncher.ini/ndslink.ini/savbackup.ini haven't to be in root if launcher supports ARGV.
ARGV receiving/sending support added.

0.51a.100824
dldicaptor gets friendlyName more precisely.
ndslink(both PC and nds app) improved for moonshell2 icon.
UseAK2_iniclear/UseRPG_iniclear/UseWoodR4_iniclear were added to inilink.ini (only read from iniclear).
This might be useful for those who want AKAIO extlink but keep YSMenu as primary kernel.
Well, you might think you can just put YSMenu.nds as kernel, but ini is editable via MoonShell2... For debug :p
Due to some demands, (though SnemulDS plugin reads only /snemul.cfg) I put a special treatment to savbackup.nds.
Now it tries /savbackup.ini, /_dstwoplug/savbackup.ini, then the same dir as ARGV[0].
I'm so annoyed that DSTWO lacks ARGV support. I wish I could remove this treatment.
Why ndslink doesn't require this treatment? Because unfortunately DSTWO doesn't have any extlinks.

0.51b.100825
Fixed: I should have added initClockIRQ()...
Tweaked ARM7 ResetRudolph.
Clarified noreturn in die().
Now savbackup compares timestamp in backing up.
Also you can backup as .sav if scandir and savdir are different.

0.51c.100829
Fixed: reset_m3 wasn't archived...
Added reset_r4.
Updated dldipatch.
fatx.c now uses open/__get_handle combo rather than direct _FAT_open_r. Much safer.

0.51c1.100829
Fixed two segmentation faults enbugged in 0.51c. Very sorry.

0.51d.100831
m3loader supports G003_minigame.* as type=2.
Searching ini is safer.
Added linkpath.ini feature to reset_*; If a reset_mse is linked to "ZZZ", it reads {/,/_dstwoplug/}linkpath.ini.
---
[linkpath]
ZZZ=/tool/dldicaptor.nds ;This nds will be launched.
---

0.51d1.100901
Fixed favlauncher unstability. There were too many local(stack) strings.
Fixed a fatal bug in boot.bin enbugged in 0.51.

0.51e.100902
Now you can specify NDS name for Use*_iniclear.
Added alternative M3DS dldi(the first dldi whose source is available). Base information is credited to toro.
inilink / iniclear now support YSMenu on M3 (which uses modified r4patch.dat).
For detailed information, please see DS Env Maker document.

(not public until 0.60)
0.51f 100903 alpha
inilink should use YSM3 config as normal YSMenu cannot be used on M3...
Merged m3region.
iniclear/favlauncher: added r4/m3 bootstrap as well as ak2/dsone/dstt/demo.
Rewritten die() and returning from loader SDK for XenoFile.

0.51g 100903 alpha
XenoFile: file listing / nds loading / launching texteditor / launching extlink.

0.51h 100904 alpha
(internal) key getting method changed.
XenoFile: sav convertion. iterating extlink.

0.51i 100906 alpha
XenoFile: DLDI switching.

0.51j 100907 alpha
Put wait to favlauncher.
Added favlauncher_ms.
XenoFile: gds convertion.

0.51k 100907 alpha
Added getFragments().
XenoFile: context menu.

0.51l 100907 beta
Xenofile goes to beta phase.

0.51m 100908 beta
Added showing image support. From now on VRAM_C is used for sub screen.
Stopped setting VRAM_C in MoonShell Simply Loader.

0.51n 100908 beta
Added backspace support to console.
XenoFile: added b15 showing basic support.

0.51o 100909 beta
keyboard image compression.

0.51p 100909 beta
XenoFile: added rename, hoorey! No longer need to use unstable DSOrganize!
Supported lid (now backlight off when lid is closed)

0.51q 100910 beta
XenoFile: added MD5 calculation.

0.60.100911
Added XenoFile.
Added nds_backup_tool_memory.

0.60a.100911
XenoFile default filter set to all for NDS whose L/R are broken.
XenoFile/ExtlinkWrapper/m3loader now recognize GBALdr and PPSEDS as homebrew correctly.

0.60b.100911
Fixed: iniclear for YSMenu didn't work...

0.61.100913 (not public)
XenoFile now supports swaping microSD. I have got AK2i working on a SD without akmenu4.nds...
Well, this feature can be used to modify/recover kernel environment without PC.
Now you can see system and firmware infomation. Y button is assigned to system menu.
Fixed bootlib.
Slightly modified moonshellreset.c / ret_menu9_Gens.s to get it work on EZ (and clones such as iSmartDS).
# MoonShell 2.00beta5 doesn't do IRQ_HANDLER ARM9 things...
# Still no compatibility with DSLinux...

0.61a.100914 alpha / 0.61b.100915 beta (not public)
Removed DSONE EOS support because even enterLastDirWhenBoot isn't working (it was undocumented though)
Very buggy support for DSTWO/iSakuReal added to inilink.
Really experimental, so avoid if you are n00b.
This support will be in alpha phase forever, so please be careful to use.
In short, this feature is only self-satisfaction to buy DSTWO.

The following is the text I copied to warning screen.
---
DSTWO support will be in alpha phase forever.
inilink moves nds to /extlink_eos/ then iniclear moves it back to original folder.
Of course you have to press nds again in DSTWO.
Multibyte filename isn't supported.
Make sure iniclear is set as /_dstwo/dsgame.nds.

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
---
iSakuReal support will be in alpha phase forever.
inilink moves nds to /defaulty.nds then iniclear moves it back to original filename.
Of course you have to hold Y button.
Multibyte filename isn't supported.
Make sure iniclear or iSakuReal/M3Sakura/
MoonShell2 is set as primary kernel.
If latter, when reseted iniclear copied as /defaultn.nds does things (and erase myself) then poweroff.
Press A to accept (from next time this notice will not be shown). Press B to shutdown.
---

0.62.100917 gamma
Version bump :p

0.63.100918
Added libprism_[f]utime() to libprism. The first NDS library to change timestamp!
Added BidirectionalCopy feature to savbackup. You can backup to /save to use in EZ, possibly.
Added create new file/make directory/change timestamp to XenoFile.
In XenoFile, pasting(copy) destination will have the same timestamp as source.
XenoFile now has faster MoonShell2 splash decoder.

0.63a.100919
Added wait to XenoFile keyrepeat and repeating speed is faster.
Now lower screen uses the same color as upper. If you want contrast like before, search fe634d6f646501 in hex then modify "01" to "02".
Now XenoFile stat shows actual file attribute.
You can toggle showing hidden/system files using X key.

0.63b.100921
Fixed r4loader for WoodR4 1.13.

0.63b1.100923
Fixed inilink for WoodR4 1.13(forgot in 0.63b...)
Added ex4tf.dldi (alternative DLDI which works on R4iLS/EX4DS)

0.63c.100927 (not public)
Now BootDSBooter() emits correct header CRC16.
Now XenoFile can enable/disable YSMenu softreset.
Now XenoFile can show battery state in system info.
For a reason, m3sakura_boot will modify /system/misakura/dldibody.bin then launch /misakura.nds from now on.

0.64.100928
Now MoonShell Simply loader can load homebrews whose first 4 bytes aren't \x2E\0\0\xEA such as PPSEDS correctly.
Added Slot2 NDS / Slot2 GBA feature to XenoFile. /xenogba.b15 is used for gba border. You can generate b15 using png2b15 program.

0.65.100930
XenoFile/mshl2wrap/dscoverloader/m3loader: Now homebrews are detected more precisely (checks ARM9 offset and ARM7 exec address).
Now XenoFile can show the NDS type(phat/lite/i).
XenoFile can show GIF images using /moonshl/plugin/gif.msp
(very sorry but do not try to open jpg/png, the same issue as MoonShell Simply).
BMP/ICO are opened using internal libnsbmp decoder.
*** Showing image is limited to the starting 256x192. Fixme... ***
XenoFile slot2 GBA now supports /gbaframe.bmp as border (if /xenogba.b15 exists, it is used as before)
Well this is also a sample implementation for Searinox...
Added XenoFile G003 bootstrap (g003rest.dat).
As XenoFile is now too complicated, reverted Makefile to that of mshl2tools 0.4x...
Added alternative scds.dldi / ttio.dldi.
This DLDI can change function as to DLDI ID (SCDS or not).
This ttio.dldi works on SD.
Also scds.dldi has a feature to bypass the need of ttreset to use YSMenu on DSONEi.
Due to these useful hack, I have decided to change bootstrap's DLDI to these.
Now inilink / iniclear can load YSMenu on DSONEi directly using SCDS_SetSDHCModeForDSTT() magic. SCYSMenu setting is removed.

0.65a.101001
Fixed several bugs which occurred in refactoring for 0.65.
ndslink improved for Pokemon B/W.

0.65b.101004
Now XenoFile can fix NDS header and encrypt/decrypt secure area.

0.65b1.101005
Fixed a fatal bug that secure area encryption key grabbing was wrong.

0.65c.101008
Making argv fixed.
Now XenoFile can use /__rpg/rpglink.nds as loader plugin (workaround for dslinux)
XenoFile splash viewing handler now supports iSakuReal and M3Sakura.

0.66.101010 #See the number sequence!
inilink/iniclear now support iSmartDS FishShell2. Of course you have to set ismat.dat to iniclear.
---
iSmartDS support will be in alpha phase forever.
inilink moves nds to /defaultn.nds then iniclear moves it back to original filename.
Multibyte filename isn't supported.
Make sure iniclear is set as /system/ismat.dat.

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
---

0.66a.101013
Fixed last cluster bug again. Users should update immediately.
XenoFile shows time in system info.

0.66b.101018
inilink.ini sample fixed for woodr4sdhc.
inilink works for AKAIO 1.8.1.
iniclear is now in debug mode. I need more tests to get DSTWO support correctly?

0.67.101021
XenoFile now supports .u8m
XenoFile now supports playing music files using msp.
But m4a/ogg have glitches lol

0.68.101021
Updated to devkitARM r32 / libnds 1.4.8 / libfat r4416.
Now I think -O2 is enough other than DLDI... XenoFile -20KB.
Now that inilink has too many warning screens, it uses lzma to decode them.

0.68a.101022 (not public)
savbackup now searches /ismartplug/ for savbackup.ini. Why don't they add ARGV support?
Added iply.dldi.
Highly optimized DLDIs.

0.68b.101025
Fixed a fatal bug in DLDI enbugged in 0.68a lol

0.70.101029
nds.r4loader.nds can handle /__rpg/cheats/usrcheat.dat (Killed compatibility with under 1.15)
nds.r4loader.nds is configurable via /__rpg/woodload.ini.
nds.r4loader.nds is no longer beta...
I hope nds.ak2loader.nds can handle softreset.
nds.ak2loader.nds can decrypt 1.6RC2 internally again.
Now MoonShell Simply loader can handle NDS whose code size is <=2MB. Safe for NitroFS homebrews.
Added YSMenu with WoodEngine. Put woodload as /YSMenu/woodload.nds then boot yswood.nds.
# The difference between ysall+r4loader combo and yswood is that you can configure cheat inside YSMenu!
YSMenu for all flashcart and YSMenu with WoodEngine read /moonshl2/extlink/inilink.ini to determine YSMenu.nds path,
if it isn't /YSMenu/YSmenu.nds.

0.70a.101031
Added g003.dldi (will corrupt your TF though lol)
Added demo.dldi (the same as ttio.dldi)
Optimized DLDIs again (put static inline to all functions other than exports)
Now inilink supports DSOneEOS and iSmartMultiMedia in a very dirty way. More buggy than DSTWO support!
Refrain from it (though I have tested on my iSmartMultiMedia)...
# Current DSTWO users have to write "DSTwoDir=/_dstwo/" to inilink.ini. Sorry.
Now XenoFile can touch file and change file attribute (only ReadOnly/Hidden/System).
Made sample mshl2wrap.ini better.
---
DSONE EOS support will be in alpha phase forever.
inilink moves nds to /!!!extlink/ then iniclear moves it back to original folder.
Of course you have to select nds again (only a few key input!) in EOS.
Multibyte filename isn't supported.
Make sure iniclear is set as /scfw.sc.

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
---
DSTWO/iSMM EOS support will be in alpha phase forever.
inilink moves nds to /!!!extlink/ then iniclear moves it back to original folder.
Of course you have to select nds again (only a few key input!) in DSTWO/iSMM.
Multibyte filename isn't supported.
Make sure iniclear is set as /_dstwo/dsgame.nds or /_ismart/dsgame.nds.

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
---

0.70b.101101
Fixed a severe bug that files created newly get readonly/hidden/system (enbugged in updating libfat).
Now XenoFile can dump /biosnds7.rom, /biosnds9.rom and /FWxxxxxxxxxxxx.bin.
Now XenoFile shows MAC address in system info.
Now XenoFile can go to NDS(L) firmware like YSMenu (if FW is 256KB...).
#This implemetation is based on desmume. So if you want to get GPL free version, recompile without -DGPL.

0.70c.101104
Fixed a fatal bug that time isn't updated after being launched. (won't affect other than XenoFile because they are just batch thing)
Now XenoFile can show M3Region...
Added several bootstraps, especially g003 and ezvi...
---
EZVi support will be in alpha phase forever.
inilink moves nds to /!!!extlink/ then iniclear moves it back to original folder.
Of course you have to select nds again (only a few key input!) in EZVi.
Multibyte filename isn't supported.
Make sure iniclear is set as /ez5sys.bin or /ez5isys.bin.

Press A to accept (from next time this notice will not be shown).
Press B to shutdown.
---

0.70d.101105
Now XenoFile can show Firmware Version and Temperature.
Return to NDS(L) firmware supports 512KB (I hope).
Fixed several warnings.

0.70d1.101105
Now XenoFile can return to DSi menu when you select "Return to NDS firmware".

0.70e.101107
Recompiled as stable...

0.70f.101118 (private)
XenoFile can "Run as homebrew after swapping flashcart" (half joking... tests didn't work).
Better dumping firmware (using malloc).
Can detect Korean Firmware.
Shows firmware size in system info.

0.70g.101216 (private)
Now XenoFile can test microSD speed. (half joking)
Updated libfat and libnds (only in r32 version)

0.71.110106 (private)
Fixed/Optimized around _consolePrintf. Overall faster.

0.72.110110 Beta (private)
XenoFile accepts .cc as text. CheatConfig or C++? I don't care.
You can use WoodM3 from inilink.
Modified r4loader/ak2loader to use ysmenu config.
ak2loader is no longer technology preview. Goodbye UiPack.
Added akysload / akaioysl / akaioms2.
From this version, AKAIO has to be 1.8.5 or later.
Homebrew loader of old versions freezes when AKAIO is loaded by mshl2tools.

0.72.110110 (private)
akaioysl / akaioms2 uses .bin rather than .cc. AKAIO 1.8.5 or later is required.
And you have to press select in cheat window again to set cheat config back to /__aio/cheat/usrcheat.dat's.

0.72a.110112 (private)
fixed some configuration bugs.

0.72b.110117 (private)
Added sav <-> Code Freak Save convertion (You need to rename to .cfs).

0.72c.110122 (private)
Fixed a serious issue on akaioms2.
Fixed a bug that r4loader couldn't use cheating.
Added a small workaround for menudo.

0.72d.110125
XenoFile:
* Now SD Test isn't a joke feature.
* Fixed fatal bug that I forgot xor with 0xffffffff when displaying CRC32.
* Added Trim NDS.

0.72e.110128
* Now inilink supports CycloDS (i)Evolution. Great! Sorry for (i)EDGE users as it doesn't have autoboot feature.
* Removed AKAIO DLDI workaround as AKAIO 1.8.5 DLDI works on YSMenu.
* XenoFile opens .xml as text.

0.74.110129 alpha1
Guarded Getting ARM7 bios and Firmware feature in DSi mode, but possibly DSi mode detecton isn't working?

0.74.110130 beta1
Now checks Running Mode also in legacy version. It seems that legacy nds is working?
* For DSi mode, use legacy edition. It doesn't use FIFO at all.

0.74.110130 beta2
Recovered ARGV support in DSi mode.

0.74.110130 beta3
Improved stability.

0.74.110202 beta5
Improved stability.
Now inilink Cyclo requires holding buttons specified in config.xml.
XenoFile: added bootlib+bootstub loader. You must update bootstub.bin too; otherwise target's exit will restart target ^^
XenoFile: added Return to bootstub.

0.75.110203 Interrude
Version bumped to stable. Prelude for mshl2tools 1.x.

0.75a.110203 Interrude
Fixed a bug that Slot2 wasn't bootable in 0.75.
Updated to libnds 1.4.10/libfat 1.0.9(modified).
Added "fix FSInfo Sector" to re-calculate the number of free clusters again.
Tuned MoonShell Simply loader up for 1.x.
You can now patch with NULL DLDI to force DSi SD.
DSi Volume Buttons no longer interrupt execution.

0.75b.110204 Interrude
Changed killing interrupts timing for 1.x.
Killed sd:/, as it seems to stuck if not booted from DSiWare.

0.75c.110205 Interrude
Slight optimization for 1.x.
Now libfat grubs LFN automatically for 8.3 files and it can handle 0xe5 filename. Yay!
Updated bootlib loader.
Now sd:/ is again available. Instead searching drivers will stop if fat:/ is OK.
Press L+R+START+SELECT to exit the program.

0.75d.110206 Interrude
Now bootlib uses thumb instead of arm.

0.76.110209 with elm
Added libelm+ywg's interface.
Extended fatx.c to support both libfat and libelm.

0.76a.110215 with elm
Updated libnds to 1.5.0.

0.77.110221 with elm
ARM7 is now written in thumb.
Fixed Rudolph/Moonlight hybrid loader.
Added MoonShell2 Loader (thx to MoonCalc src).

0.77a.110222 with elm
Added fwrecovery: if you lose firmware settings, you can recover from your fw.bin using this software.

0.77b.110226 with elm
Capsulized Frontend API.
Removed m3dscover; nds.m3loader.nds can now be used as loader_m3.nds.

0.77c.110227 with elm
Some fixes.
Now ARGV[1] IO is supported.

0.77d.110304 with elm
SCDS_SetSDHCForDSTT() is called before Main().

0.78.110313 with elm
Fixed inilink stack overflow.
Fixed some filename handling in inilink.
inilink now supports DSTWO fully.
XenoFile can now convert NO$GBA saves.
Added nesdswrap.

0.78a.110315 with elm
Fixed XenoFile msp handling.

0.78b.110316 with elm
Now nesdswrap is compressed with LZMA.

0.78c.110318 with elm
L+R+START+SELECT softreset is available also on r23.
L+R+START+SELECT shutdown/return to DSi menu on nesdswrap.

0.78d.110319 with elm
Added nesDS 0.44/nesterDS moonlight to nesdswrap.
inilink supports DSTWO and iSMM.

0.78e.110323 with elm
Fixed inilink CRLF regression.
Now XenoFile can access .bin and .ini in loading msp.

0.79.110327
returnDSMenu() is implemented to libvalkyria rather than XenoFile, which enabled die() extention.
Now this can be used as RAM unlocker.
Fixed disc_unmount() timing in some loaders (fixes m3sakura_boot regression).
Sorry but mshl2tools requires menu.xx trick for M3iZero autoboot from this version, due to large libprism.

0.79a.110403
11MB of DSi RAM is allocated for extram, rather than 8MB.
nesdswrap resources are compressed better.
Added nesDS latest (0.48a) to nesdswrap.
* From this version, lzma_alone is required for build, rather than LZMA Utils.

0.79b.110424
Added /_plugin_/ and /_iMenu/_ini/ to search path.
FavLauncher / ndslink ini can be searched.
FavLauncher doesn't show nds list if favlauncher.ini isn't found.
Fixed a fatal bug around die().

0.79c.110425
XenoFile can do openpatch according to /GameList.txt
Please note GameList has to be with Game ID (you can generate using GameListV2Builder.zip in XenoBox).
This is experimental; GameList.txt is in XenoBox package.

0.79d.110508
Updated nesdswrap.
Fixed XenoFile openpatch_single.

0.79e.110516
Updated nesdswrap.
Now commercial bridges ({ak2,m3,r4}loader) touch sav.

0.79e1.110605
Updated nesdswrap.
Fixed some warnings.

0.79f.110606
Now XenoFile can treat extmem for msp.

0.79g.110615 alpha
DMA is cleared on start (test)

0.80.110627
Rebuilded using devkitARM r33.

0.80a.110628
Updated nesdswrap.

0.80b.110703
Updated nesdswrap.
Updated libnds to 1.5.1.
Fixed mshl2wrap.
memdump now dumps register too.
Chessmaster issue was fixed for AK2i (not for DSONEi).
I don't know about side-effects though.

0.81.110706
Updated bootlib.
Updated nesdswrap.
Rebuilded using devkitARM r34.

0.81a.110707
Refixed bootlib.
Now it is much faster thanks to multisector read.

0.81b.110711
Updated nesdswrap.
Updated libnds to 1.5.2.

0.81c.110713
XenoFile can decrypt any R4 dats (on R4).
XenoFile can show DS card info (aww, only on SCDS/TTDS).

0.81d.110817
Updated libnds to 1.5.3.

0.81e.110823
Updated libnds to 1.5.4.
Updated nesdswrap.

0.81f.110824
Fixed no sound issue in 0.81e.

0.85.110827
Fixed fatal bug: r23 edition couldn't receive ARGV properly.
Removed unneeded 0x027fffe70 argv reader from r23 edition. It isn't required on modified ds_crt0_arm9.s.
Now libprism receives ARGV in proper way.
Added .argv handler to XenoFile.
Added argvview / argvloader.
mshl2tools treats all paths using driver letters.
(Perhaps) now mshl2tools support sudokuhax.

0.86.110830
Now moonshell simply loader patches NDS header to use 0x02ff... for DSi mode.

0.86a.110912 Schlaf
Updated nesdswrap.

0.87.110917 Schlaf
Fixed nesdswrap.
Added reset_ievo (compatible with DSi mode).
Fixed a fatal bug in _FAT_directory_lfnLength() (thanks to desmume's lovely bug).

0.87a.111005 Schlaf
Updated nesdswrap.
Fixed 95% warnings.

0.88.111116 Schlaf
Updated nesdswrap.
Fixed "Change DLDI" feature (forgot to clear cache :p).

0.88a.111121 Schlaf
Added wdayloader.

0.88b.111124 Schlaf
extlink.dat path is now modifiable.
internal archive has fishell2/extlink/nds.mshl2wrap.nds.
ARM7 bios isn't dumped if not required.
Outputs nocashMessage() (non-legacy only).

0.88c.111125 Phase:Rebirth
Cleaned up ram.c.

0.88d.111211 Phase:Rebirth
Fixed XenoFile's CRC16 calculation. (Now it shows the same value as LHA)
Fixed XenoFile's very rare case bug: if selected music file can't be opened, xenofile freezes.

0.88e.111216 (Internal)
Added gbadump.

0.89.120113 Phase:Rebirth
Hopefully XenoFile's warning are resolved using typeof casting (lol).
Rebuilt with DevKitARMr37 + libnds r4875.
Fixed mondayloader.
Updated nesdswrap.

0.89a.120122 Phase:Rebirth
savbackup now uses r+b if target is already present.

0.89b.120209 Phase:Rebirth
Added Google TwoFactor Authenticator on NDS.

0.89c.120228 Phase:Rebirth
Fixed libelm's bootlib loader.
Fixed libelm's obtaining SFN.

0.90.120308 Phase:Rebirth
nesdswrap hotfix.
detects extmem size more precisely.
XenoFile can now call /__rpg/associations.ini (libfat only).
