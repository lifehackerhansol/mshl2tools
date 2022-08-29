@echo off
set NDS=nds
set LINK=mshl2wrap
set TEMPLATE=nds.mshl2wrap.nds

ndslink.exe "%TEMPLATE%" "/%NDS%/" "%LINK%/"
