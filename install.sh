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
if [ "$(uname -s)" = "Darwin" ]
then
	echo "macOS detected"
	if [ "$(uname -m)" = "arm64" ]
	then
		echo "Downloading Hajime for macOS ARM directly"
		curl -o hajime-macos-arm64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-macos-arm64.zip
		unzip hajime-macos-arm64.zip
                if [ -e hajime ]
                then
                        chmod +x hajime
			rm hajime-macos-arm64.zip
                        ./hajime
                        exit 0
                fi
	fi
  if [ "$(uname -m)" = "x86_64" ]
	then
		echo "Downloading Hajime for macOS x86_64 directly"
		curl -o hajime-macos-amd64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-macos-amd64.zip
		unzip hajime-macos-amd64.zip
                if [ -e hajime ]
                then
                        chmod +x hajime
			rm hajime-macos-amd64.zip
                        ./hajime
                        exit 0
                fi
	fi
fi
if [ "$(uname -s)" = "Linux" ]
then
	echo "Linux detected"
	if [ "$(uname -m)" = "x86_64" ]
	then
		echo "Trying to download Hajime for Linux amd64 directly"
		curl -o hajime-linux-amd64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-linux-amd64.zip
		unzip hajime-linux-amd64.zip
		if [ -e hajime ]
		then
			chmod +x hajime
			rm hajime-linux-amd64.zip
			./hajime
			exit 0
		fi
	fi
	echo "Compiling Hajime now"
	sudo apt update && sudo apt -y install g++ git #g++-10 or the otherwise latest version of g++ that supports c++17
	sudo apt -y upgrade
	sudo yum install -y clang
	sudo yum install -y git
fi
if [ "$OSTYPE" = "OpenBSD" ]
then
	echo "OpenBSD detected"
	pkg_add git
fi
echo "Downloading..."
git clone https://github.com/Slackadays/Hajime
echo "Compiling..."
echo "This may take from a few seconds to a few minutes depending on your system speed."
cd Hajime/source
echo hajime.cpp getvarsfromfile.cpp server.cpp output.cpp languages.cpp installer.cpp wizard.cpp | xargs -n 1 -P 7 g++ -c -Ofast -std=c++17 #lstdc++fs enables filsystem library in older installations
g++ -o hajime hajime.o server.o getvarsfromfile.o installer.o output.o languages.o wizard.o -pthread -lstdc++fs
if [ "$OSTYPE" = "OpenBSD" ]
then
	echo hajime.cpp getvarsfromfile.cpp server.cpp output.cpp languages.cpp installer.cpp wizard.cpp | xargs -n 1 -P 6 eg++ -c -Ofast -std=c++17
	eg++ -o hajime hajime.o server.o getvarsfromfile.o installer.o output.o languages.o wizard.o -pthread -lstdc++fs
fi
echo "Cleaning up..."
chown -R $USER ../../Hajime/.git #change perms for certain misbehaving files that come with git
mv hajime ../../ #move the binary to the original folder where the script was started
rm -rf ../../Hajime #remove the cloned directory, ignore junk files from .git
cd ../../
./hajime
