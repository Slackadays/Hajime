# Hajime
A fully-featured Minecraft server startup script suite that offers a friendly user interface, blazing fast speeds, and wide compatibility.

# Discord
For discussion, rapid support, development updates, and announcements about Hajime, join our Discord at https://discord.gg/J6asnc3pEG.

# Quick Installation
Use this command to install Hajime in one step. It will download and automagically compile the latest version available on GitHub.
```
curl https://raw.githubusercontent.com/Slackadays/Hajime/master/install.sh | sh
```
Like with all shell scripts, check the install.sh file first for any potentially malicious commands.

# Why?

## The Big Problem
Most other startup scripts are either bare-bones shell scripts or systemd services. Some utilities let you create a custom shell script for your server, but then you have to go a website and enter your server's information, copy that script, download it to the server, and more. Yuck! This "script" is a little different because it provides way more features and is always blazing fast thanks to being compiled C++.

## Features
- 100% FLOSS with the LGPL license!
- Easy installation (full installation wizard coming soon).
- Multiple server support through multithreading.
- Compiles and works on most Linux, Windows, FreeBSD, and OpenBSD (and soon Mac) systems. Hajime currently does not compile on NetBSD, but binaries may run.
- Configure every setting exactly how you want it.
- Many customizable log messages to see what exactly happened.
- Help and support available in our Discord server.
- Can be compiled for every installation to suit every server's capabilities.
- Uses memory-safe modern C++ features and true cross-platform compatibility.

## Requirements
There are currently a couple prerequisites. You'll need a POSIX-compliant system, so this means Linux or FreeBSD or MacOS. Also, to use the one-step installation option and its compilation path, your platform will need to support the git and g++ packages.

## Why C++?
I decided to make this script in C++ to help me learn it as well as get acceptable performance no matter how many features I add. 
There's also a lot of standard libraries available for C++ and that might be an advantage over something like shell script. Plus, "Modern C++" features are great for memory safety and beautifying code.

## What's up With That Name?
"Hajime" is simply "begin" in Japanese. I know this because I hear it every time I train with my judo instructor.

# Instructions (WIP)

## Pre-compiled binaries (from the [Releases](https://github.com/Slackadays/Hajime/releases) page)
If you are using a precompiled binary, stay in this section. If you are compiling, skip this section. Download the latest version for your platform. Next, place it in a simple, memorable location. In my own server, I use `./media`. 
Now, run `sudo ./hajime -I` to install the initial configuration file. 

### Using `systemd` to make Hajime a service 

If you would like to make a systemd service, now run `sudo ./hajime -S`
to make a systemd service file. By default, the file created is called **hajime.service**. If you would like a different name (for example, if you are running multiple servers) change the setting in the file **hajime.conf**. Now enable it using `sudo systemctl enable hajime`to run Hajime on startup. Before rebooting, you must change the settings in the **server.conf** and **hajime.conf** files. Make server.conf by running `sudo ./hajime -I` again. `server.conf` is the settings file for an individual server object. This is done for future-proofing for future versions that may implement multithreading. `hajime.conf` is the settings file for the main program.

## Compiling Your Own
It's easy to compile Hajime. First, download the files in the [**source**]./source/) section. Then, run this command:
```
g++ -std=c++20 -Ofast -o hajime hajime.cpp
```
Hajime requires at least **C++20** to work.
   
# Using Hajime

## Enable the systemd Service 
This script is a cinch to actually use. To check its status, run `sudo systemctl status hajime` (or substitute "hajime" if you have renamed the systemd service file). This will display the messages sent by Hajime. For debugging, you may also run it manually.

## Configure the Server
Once you run Hajime for the first time, create a "servers file" AND a "server file." Then, open server0.conf in a text editor and apply the needed settings.

## Important Note
You must use the command "screen" in the server execution command for Hajime to work. This limitation will be removed in a future update.

# Troubleshooting
If Hajime seems to not work after it's been running for a while, just restart it. This solves 99% of problems!
You may also open an issue in the repository if you find functionality you don't believe is intentional
