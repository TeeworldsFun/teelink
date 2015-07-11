#!/bin/bash
############################
# Uninstaller Script for H-Client by unsigned char*
#
# + FEATURES:
# Remove .desktop file in ~/.local/share/applications
# Remove symbolic link in ~/Desktop to .desktop file
# Remove symbolic link in /usr/local/bin to teeworlds binary
##################################################################################
TWNAMEBIN="teeworlds"
DFFILENAME="$HOME/.local/share/applications/$TWNAMEBIN.desktop"
DESKTOPF=$(xdg-user-dir DESKTOP)"/$TWNAMEBIN.desktop"


echo "Uninstalling H-Client..."

# Remove from desktop folder
if [ -f $DESKTOPF ]; then
    unlink $DESKTOPF
fi

# Remove from local/bin folder
if [ -f "/usr/local/bin/$TWNAMEBIN" ]; then
    sudo unlink "/usr/local/bin/$TWNAMEBIN"
fi

# Remove from applications folder
if [ -f $DFFILENAME ]; then
    rm $DFFILENAME
fi

# Remove icon from share/pixmaps folder
if [ -f "/usr/share/pixmaps/teeworlds_hclient.png" ]; then
    sudo rm "/usr/share/pixmaps/teeworlds_hclient.png"
fi

echo "H-Client uninstalled successfully :)"

