#!/bin/sh

run_and_add_perms() {
  TARGET_FILE=$1
  unzip $1
  if [ -e hajime ]
  then
    echo "Assigning the CAP_PERFMON capability to Hajime..."
    sudo setcap 'cap_perfmon+pe' hajime
    chmod +x hajime
    rm $1
    ./hajime
    exit 0
  fi
}

update_and_install() {
  echo "Installing necessary packages..."
  if [ "$USER" != "root" ]
  then
          echo "You may need to be the root user to install g++, git, and cmake."
  fi
  if [ "$OSTYPE" = "FreeBSD" ]
  then
          echo "FreeBSD detected"
          pkg update
          pkg install gcc10 git cmake
          pkg upgrade
  fi
  if [ "$OSTYPE" = "OpenBSD" ]
  then
  	echo "OpenBSD detected"
  	pkg_add git cmake
  fi
  if [ "$(uname -s)" = "Linux" ]
  then
    sudo apt update
    sudo apt install -y g++10 cmake git
    sudo yum install -y clang && sudo yum install -y git
    sudo yum install -y cmake
  fi
}

if [ "$(uname -s)" = "Darwin" ]
then
	echo "macOS detected"
	if [ "$(uname -m)" = "arm64" ]
	then
		echo "Downloading Hajime for macOS ARM directly"
		curl -s -o hajime-macos-arm64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-macos-arm64.zip
		run_and_add_perms hajime-macos-arm64.zip
	fi
  if [ "$(uname -m)" = "x86_64" ]
	then
		echo "Downloading Hajime for macOS x86_64 directly"
		curl -s -o hajime-macos-amd64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-macos-amd64.zip
		run_and_add_perms hajime-macos-amd64.zip
	fi
  echo "Can't find a supported platform; compiling Hajime now"
  update_and_install
fi
if [ "$(uname -s)" = "Linu" ]
then
	echo "Linux detected"
  echo "Updating and installing the unzip package..."
  sudo apt update
  sudo apt install -y unzip
  sudo yum install -y unzip
	if [ "$(uname -m)" = "x86_64" ]
	then
		echo "Trying to download Hajime for Linux amd64 directly"
		curl -s -o hajime-linux-amd64.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-linux-amd64.zip
		run_and_add_perms hajime-linux-amd64.zip
	fi
	if [ -n "$(uname -m | grep armv7l)" ] || [ -n "$(uname -m | grep aarch64)" ]
	then
		echo "Trying to download Hajime for Linux arm32hf directly"
		curl -s -o hajime-linux-arm32hf.zip -L https://github.com/Slackadays/Hajime/releases/latest/download/hajime-linux-arm32hf.zip
		run_and_add_perms hajime-linux-arm32hf.zip
	fi
	echo "Can't find a supported platform; compiling Hajime now"
  update_and_install
fi
echo "Cloning repo..."
git clone https://github.com/slackadays/Hajime
echo "Compiling..."
echo "This may take from a few seconds to a few minutes depending on your system speed."
cd Hajime
cmake source
cmake --build . -j 9
echo "Cleaning up..."
chown -R $USER "../Hajime/.git" #change perms for certain misbehaving files that come with git
mv hajime ../ #move the binary to the original folder where the script was started
rm -rf ../Hajime #remove the cloned directory, ignore junk files from .git
cd ../
echo "Assigning the CAP_PERFMON capability to Hajime..."
sudo setcap 'cap_perfmon+pe' hajime
./hajime
exit 0
