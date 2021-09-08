#!/bin/sh
echo "\e[1mInstalling g++ and git...\e[0m"
apt update && apt install -y g++-10 git #g++-10 or the latest version of g++
echo "\e[1mDownloading...\e[0m"
git clone https://github.com/Slackadays/Hajime
echo "\e[1mCompiling...\e[0m"
cd Hajime/source
g++ -Ofast -std=c++20 -o hajime hajime.cpp
echo "\e[1mCleaning up...\e[0m"
mv hajime ../../ #move the binary to the original folder where the script was started
rm -r ../../Hajime #remove the cloned directory

