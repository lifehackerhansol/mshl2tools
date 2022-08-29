MoonShell2 inilinker xxxloader multibyte

This is MoonShell2 extlink for YSMenu/AKAIO/AK2/M3TouchPod; write auto boot things to ini then boot the menu.
(The successor of yslaunch)
This treats Unicode LFN, not SFN. So ROMs with multibyte character filename can be launched.

Use with iniclear; otherwise when the menu is executed it will "auto boot".

This version can launch YSMenu/AKAIO/AK2/M3TouchPod.
M3TouchPod loading is very experimental.
1.Don't use for homebrew. It might crash in creating "sav". (mshl2wrap will force hbmode=1 if hbmode set to 0)
2.It is recommended to create save file in other carts.
3.Make sure you have launched the ROM outside inilinker before use.

3 is the most important. If not meet it will destroy your sav.
