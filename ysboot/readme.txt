YSMenu Boot

This clears AUTO_BOOT in ysmenu.ini then boot YSMenu.nds.
# in Acekard, please call from akmenu. DLDI in AKAIO doesn't work.

ttmenu.dat: ttio dldi-patched. Use it as DSTT kernel. Now I can safely use ysloader...

Caution: when clearing AUTO_BOOT, this directly calls _FAT_rename_r() because original rename() is broken. Be careful.

ysboot.ini: YSBoot configuration.
Put it in one of following: /YSMENU/ /_SYSTEM_/ /TTMENU/ /__AK2/ /__RPG/ /
