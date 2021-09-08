#!/bin/sh
echo "\e[1mInstalling g++ and git...\e[0m"
apt update && apt install -y g++-10 git
echo "\e[1mDownloading...\e[0m"
git clone https://github.com/Slackadays/Hajime
echo "\e[1mCompiling...\e[0m"
g++ -Ofast -std=c++20 -o hajime Hajime/source/hajime.cpp
echo "\e[1mCleaning up...\e[0m"
rm -r Hajime
