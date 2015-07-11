#!/bin/bash
############################
# Installer Script for H-Client by unsigned char*
#
# + FEATURES:
# Create .desktop file link in ~/.local/share/applications
# Create symbolic link in ~/Desktop to .desktop file (optional)
# Create symbolic link in /usr/local/bin to teeworlds binary
# Copy data/teeworlds.png to /usr/share/pixmaps
##################################################################################

TWNAME="Teeworlds (H-Client Mod)"
TWNAMEBIN="teeworlds"
TWFILENAME="$PWD/$TWNAMEBIN"
DFFILENAME="$HOME/.local/share/applications/$TWNAMEBIN.desktop"
DESKTOPF=$(xdg-user-dir DESKTOP)"/$TWNAMEBIN.desktop"

if [ ! -f $TWFILENAME ]; then
    echo "Error: Teeworlds binary not found! please run this script from teeworlds directory"
    exit 1
fi


read -r -d '' DFDATA << EOM
[Desktop Entry]
Version=1.0
Encoding=UTF-8
Terminal=false
Type=Application
Categories=Game;ArcadeGame;
Comment=Teeworlds Mod
Icon=teeworlds_hclient
Path=${PWD}
Name=${TWNAME}
GenericName=${TWNAME}
Exec=${TWNAMEBIN}
EOM


echo "Installing H-Client..."
echo -e "Teeworlds Binary: $TWFILENAME\n"

# Put icon in share/pixmaps folder
if [ -f "$PWD/data/teeworlds.png" -a ! -f "/usr/share/pixmaps/teeworlds_hclient.png" ]; then
    sudo cp "$PWD/data/teeworlds.png" "/usr/share/pixmaps/teeworlds_hclient.png"
elif [ ! -f "/usr/share/pixmaps/teeworlds_hclient.png" ]; then
    echo "Warning: Teeworlds icon not found"
fi


# Create .desktop in applications folder
if [ ! -d "$HOME/.local/share/applications" ]; then
    mkdir -p "$HOME/.local/share/applications"
fi
if [ ! -f $DFFILENAME ]; then
    echo "$DFDATA" >$DFFILENAME
    chmod +x $DFFILENAME
fi

# Put in local/bin folder
if [ ! -d "/usr/local/bin" ]; then
    sudo mkdir -p "/usr/local/bin"
fi
if [ ! -f "/usr/local/bin/$TWNAMEBIN" ]; then
    sudo ln -s $TWFILENAME "/usr/local/bin/$TWNAMEBIN"
fi

# Put in desktop folder
read -r -p "Do you want to put a shortcut on the desktop? [Y/n]: " response
if [[ ! $response =~ [nN] ]]; then
    if [ ! -f $DESKTOPF ]; then
        ln -s $DFFILENAME $DESKTOPF
    fi
fi

echo "H-Client installed successfully :)"

