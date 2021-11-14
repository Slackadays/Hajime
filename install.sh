#!/bin/sh
echo "Installing necessary packages..."
if [ "$USER" != "root" ]
then
        echo "You may need to be the root user to install g++ and git."
fi
if [ "$OSTYPE" = "FreeBSD" ]
then
        echo "FreeBSD detected"
        pkg update
        pkg install gcc10 git
        pkg upgrade
fi
echo "Linux detected"
sudo apt update && sudo apt -y install g++ git #g++-10 or the otherwise latest version of g++ that supports c++17
sudo apt -y upgrade
sudo yum install -y clang
sudo yum install -y git
pkg_add git
echo "Downloading..."
git clone https://github.com/Slackadays/Hajime
echo "Compiling..."
echo "This may take from a few seconds to a few minutes depending on your system speed."
cd Hajime/source
echo hajime.cpp getvarsfromfile.cpp server.cpp output.cpp languages.cpp installer.cpp | xargs -n 1 -P 6 g++ -c -Ofast -std=c++17 #lstdc++fs enables filsystem library in older installations
g++ -o hajime hajime.o server.o getvarsfromfile.o installer.o output.o languages.o -pthread -lstdc++fs
echo hajime.cpp getvarsfromfile.cpp server.cpp output.cpp languages.cpp installer.cpp | xargs -n 1 -P 6 eg++ -c -Ofast -std=c++17
eg++ -o hajime hajime.o server.o getvarsfromfile.o installer.o output.o languages.o -pthread -lstdc++fs
echo "Cleaning up..."
chown -R $USER ../../Hajime/.git #change perms for certain misbehaving files that come with git
mv hajime ../../ #move the binary to the original folder where the script was started
rm -rf ../../Hajime #remove the cloned directory, ignore junk files from .git
