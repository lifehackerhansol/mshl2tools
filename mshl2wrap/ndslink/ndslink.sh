#!/bin/bash
#Please confirm your unix system uses UTF-8 for file names, if you want to use 2-byte characters.

NDS=nds
LINK=mshl2wrap
TEMPLATE=nds.mshl2wrap.nds

ndslink "$TEMPLATE" "/$NDS/" "$LINK/"
