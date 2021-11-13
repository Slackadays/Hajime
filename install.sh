#!/bin/sh
echo "\033[1mInstalling g++ and git...\033[0m"
if [ "$OSTYPE" = "FreeBSD" ]
then
        echo "FreeBSD detected"
        if [ "$USER" != "root" ]
        then
                echo "You may need to be the root user to install Hajime"
        fi
        pkg update
        pkg install gcc10 git
        pkg upgrade
else
        echo "Linux detected"
        sudo apt update && sudo apt -y install -y g++ git #g++-10 or the otherwise latest version of g++ that supports c++17
        sudo apt -y upgrade
fi
echo "\033[1mDownloading...\033[0m"
git clone https://github.com/Slackadays/Hajime
echo "\033[1mCompiling...\033[0m"
echo "\033[1mThis may take from a few seconds to a few minutes depending on your system speed.\033[0m"
cd Hajime/source
echo hajime.cpp getvarsfromfile.cpp server.cpp output.cpp languages.cpp installer.cpp | xargs -n 1 -P 6 g++ -c -Ofast -std=c++17 #lstdc++fs enables filsystem library in older installations
g++ -static -o hajime hajime.o server.o getvarsfromfile.o installer.o output.o languages.o -pthread -lstdc++fs
echo "\033[1mCleaning up...\033[0m"
chown -R $USER ../../Hajime/.git ../../Hajime/source/hajime #change perms for certain misbehaving files that come with git
mv hajime ../../ #move the binary to the original folder where the script was started
rm -rf ../../Hajime #remove the cloned directory, ignore junk files from .git
