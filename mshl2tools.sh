rm -f mshl2tools_r34.7z
7z a -r0 -mx=9 -xr0!*/.svn/* -xr0!*/.svn mshl2tools_r34.7z *.sh arm7 bootlib libfat libelm libprism readme.txt license.txt \
dldi_m3r4 dldi_mati dldi_ez5v2 dldicaptor dlditest linktemplate \
mshl2wrap ndslink iniclear inilink ak2loader akaioms2 akaioysl akysload m3loader r4loader dscoverloader \
reset_mse reset_m3 reset_r4 resetproxy m3sakura_boot favlauncher favlauncher_bl favlauncher_ms \
ysall vhbootlib savbackup m3region xenofile nds_backup_tool_memory yswood woodload fwrecovery nesdswrap \
memdump stripmbr ttloader r4dump reset_mse_arco argvview argvloader reset_ievo wdayloader

#: << "#INTERNAL"
rm -f mshl2tools_DSEM.7z
mkdir DSEM
mkdir DSEM/extlink
mkdir DSEM/resource
mkdir DSEM/resetmse
mkdir DSEM/dldi
cp -f bootlib/bootloader/boot.bin DSEM/resource/
cp -f bootlib/bootstub/bootstub.bin DSEM/resource/
cp -f iniclear/iniclear.nds DSEM/resource/
cp -f reset_mse/moonshl2alt.nds DSEM/resource/
cp -f reset_mse/menu.xx DSEM/resource/
cp -f reset_mse/g003rest.dat DSEM/resource/
cp -f reset_mse/DSI2.nds DSEM/resetmse/
cp -f reset_mse/DSI2_ismart.nds DSEM/resetmse/
cp -f reset_mse/EZ5H.nds DSEM/resetmse/
cp -f reset_mse/EZ5i.nds DSEM/resetmse/
cp -f reset_mse/IMAT.nds DSEM/resetmse/
cp -f reset_mse/Mati.nds DSEM/resetmse/
cp -f reset_mse/R4DS.nds DSEM/resetmse/
cp -f reset_mse/RPGN.nds DSEM/resetmse/
cp -f reset_mse/XXXX.nds DSEM/resetmse/
cp -f reset_m3/M3DS.nds DSEM/resetmse/
cp -f reset_r4/R4TF.nds DSEM/resetmse/
cp -f reset_r4/R4TF.nds DSEM/resource/REBOOT.dat
cp -f ysall/ysall.nds DSEM/resource/
cp -f yswood/yswood.nds DSEM/resource/
cp -f woodload/woodload.nds DSEM/resource/
cp -f inilink/{nds.inilink.nds,inilink.ini} DSEM/extlink/
cp -f ak2loader/nds.ak2loader.nds DSEM/extlink/
cp -f akaioms2/nds.akaioms2.nds DSEM/extlink/
cp -f akaioysl/akloader.nds DSEM/resource/akloader_akaioysl.nds
cp -f akysload/akloader.nds DSEM/resource/akloader_akysload.nds
cp -f m3loader/{nds.m3loader.nds,m3loader.ini} DSEM/extlink/
cp -f mshl2wrap/{nds.mshl2wrap.nds,mshl2wrap.ini} DSEM/extlink/
cp -f vhbootlib/_vh.bootlib.nds DSEM/extlink/
cp -f r4loader/nds.r4loader.nds DSEM/extlink/
cp -f dldi_m3r4/{r4tf,m3r4,ttio}.dldi DSEM/dldi/
cp -f dldi_m3r4/scds.dldi DSEM/dldi/scdssdhc1.dldi
cd DSEM
7z a -r0 -mx=9 ../mshl2tools_DSEM.7z *
cd ..
rm -rf DSEM

rm -f mshl2tools_internal.7z
mkdir internal
mkdir internal/_dstwo
mkdir internal/_ismart
mkdir internal/fishell2
mkdir internal/fishell2/extlink
mkdir internal/moonshl2
mkdir internal/moonshl2/extlink
mkdir internal/moonshl2/resetmse
mkdir internal/moonshl
mkdir internal/moonshl/plugin
mkdir internal/d_system
mkdir internal/d_system/loaders
mkdir internal/tool
mkdir internal/system
mkdir internal/YSMenu
mkdir internal/app
cp -f ak2loader/nds.ak2loader.nds internal/moonshl2/extlink/
cp -f akaioms2/nds.akaioms2.nds internal/moonshl2/extlink/
cp -f akaioysl/akloader.nds internal/YSMenu/
#cp -f akysload/akloader.nds internal/YSMenu/
cp -f argvloader/arg.argvloader.nds internal/moonshl2/extlink/
cp -f argvloader/argv.argvloader.nds internal/moonshl2/extlink/
cp -f argvview/argvview.nds internal/tool/
cp -f bootlib/bootloader/boot.bin internal/
cp -f bootlib/bootstub/bootstub.bin internal/
cp -f dldicaptor/dldicaptor.nds internal/tool/
cp -f dlditest/dlditest.nds internal/tool/
cp -f dscoverloader/loader_extlink.nds internal/d_system/loaders/
cp -f favlauncher/favlauncher.nds internal/
cp -f favlauncher_bl/favlauncher_bl.nds internal/
cp -f favlauncher_ms/favlauncher_ms.nds internal/
cp -f fwrecovery/fwrecovery.nds internal/tool/
cp -f inilink/nds.inilink.nds internal/moonshl2/extlink/
cp -f iniclear/iniclear.nds internal/
cp -f iniclear/akmenu4.nds internal/
#cp -f memdump/memdump.nds internal/akmenu4.nds
#dldipatch xenofile/source/dldi/rpgs.dldi internal/akmenu4.nds
cp -f iniclear/ttmenu.dat internal/
cp -f iniclear/ismat.dat internal/system/
cp -f iniclear/dsgame.nds internal/_dstwo/
cp -f iniclear/dsgame.nds internal/_ismart/
cp -f iniclear/akmenu4.nds internal/moonshl2/resetmse/RPGS.nds
cp -f iniclear/ttmenu.dat internal/moonshl2/resetmse/TTIO.nds
cp -f iniclear/ismat.dat internal/moonshl2/resetmse/IMAT.nds
cp -f iniclear/ismat.dat internal/moonshl2/resetmse/Mati.nds
cp -f iniclear/dsgame.nds internal/moonshl2/resetmse/DSI2.nds
#cp -f m3dscover/loader_m3.nds internal/d_system/loaders/
cp -f m3loader/nds.m3loader.nds internal/d_system/loaders/loader_m3.nds
cp -f m3loader/nds.m3loader.nds internal/moonshl2/extlink/
cp -f m3sakura_boot/m3sakura.nds internal/
cp -f m3region/m3region.nds internal/tool/
cp -f memdump/memdump.nds internal/tool/
cp -f mshl2wrap/nds.mshl2wrap.nds internal/moonshl2/extlink/
cp -f mshl2wrap/nds.mshl2wrap.nds internal/fishell2/extlink/
binreplace internal/fishell2/extlink/nds.mshl2wrap.nds //moonshl2//extlink.dat/0in/0 //fishell2//extlink.dat/0in/0
cp -f ndslink/ndslink.nds internal/tool/
cp -f nesdswrap/nes.nesdswrap.nds internal/moonshl2/extlink/
cp -f r4dump/r4dump.nds internal/tool/
cp -f r4loader/nds.r4loader.nds internal/moonshl2/extlink/
cp -f reset_mse/moonshl2alt.nds internal/
cp -f reset_mse/moonshl2alt.nds internal/moonalt.nds
#cp -f reset_mse/R4DS.nds internal/moonshl2/resetmse/
#cp -f reset_mse/RPGS.nds internal/moonshl2/resetmse/
#cp -f reset_mse/IMAT.nds internal/moonshl2/resetmse/
#cp -f reset_mse/Mati.nds internal/moonshl2/resetmse/
cp -f reset_mse/menu.xx internal/system/
cp -f reset_m3/M3DS.nds internal/moonshl2/resetmse/
cp -f reset_r4/R4TF.nds internal/moonshl2/resetmse/
cp -f reset_ievo/CEVO.nds internal/
cp -f resetproxy/reset.mse internal/moonshl/plugin/
cp -f savbackup/savbackup.nds internal/tool/
cp -f vhbootlib/_vh.bootlib.nds internal/moonshl2/extlink/
cp -f wdayloader/wdayloader.nds internal/app/
cp -f woodload/woodload.nds internal/YSMenu/
cp -f xenofile/xenofile_02000000.nds internal/
cp -f xenofile/xenofile_02000000.nds internal/boot.nds
cp -f ysall/ysall.nds internal/
cp -f yswood/yswood.nds internal/
cd internal
7z a -r0 -mx=9 ../mshl2tools_internal.7z *
cd ..
rm -rf internal
#INTERNAL

