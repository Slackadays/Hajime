#!/bin/sh
echo "\e[1mInstalling g++ and git...\e[0m"
sudo apt update && sudo apt install -y g++ git #g++-10 or the otherwise latest version of g++ that supports c++17
echo "\e[1mDownloading...\e[0m"
git clone https://github.com/Slackadays/Hajime
echo "\e[1mCompiling...\e[0m"
echo "\e[1mThis may take from a few seconds to a few minutes depending on your system speed.\e[0m"
cd Hajime/source
g++ -Ofast -std=c++17 -o hajime hajime.cpp -lstdc++fs #lstdc++fs enables filsystem library in older installations
echo "\e[1mCleaning up...\e[0m"
chown -R $USER ../../Hajime/.git #change perms for certain misbehaving files that come with git
mv hajime ../../ #move the binary to the original folder where the script was started
rm -rf ../../Hajime #remove the cloned directory, ignore junk files from .git
