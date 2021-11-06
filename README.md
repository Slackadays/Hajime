# Hajime 
<img src="HJ.png" alt="Hajime logo" width="100"/>
A fully-featured Minecraft server startup script suite that offers a friendly user interface, blazing fast speeds, and wide compatibility.

# Discord
For discussion and development updates about Hajime, join our Discord at https://discord.gg/J6asnc3pEG!

# Quick Installation
## Linux/BSD
Use this command to install Hajime in one step. It will download and automagically compile the latest version available on GitHub.
```
curl -L https://gethaji.me | sh
```
Like with all shell scripts, check the install.sh file first for any potentially malicious commands.

## Windows
Download the latest version available in the Releases page or from Discord. If you'd like to compile Hajime yourself, the code will work with the Visual C++ compiler.

# Why?

## The Problem
Many other startup scripts are bare-bones shell scripts or systemd services. Some utilities let you create a shell script, but then you have to go a website and enter your server's information, copy that script, download it to the server, and more. Yucky! This "script" is different because it provides many more features and is always blazing fast thanks to being compiled C++.

## Features
- Supports Aikar's Flags out of the box.
- Easy installation (full installation wizard coming soon).
- Multiple server support through multithreading.
- Compiles and works on most Linux, Windows, FreeBSD, and OpenBSD (and soon Mac) systems. Hajime currently does not compile on NetBSD, but binaries may run.
- Hardware agnostic, so Hajime will compile and run on just about any processor you throw at it.
- Configure every setting exactly how you want it.
- Many customizable log and debug messages to see what exactly happened.
- Help and support available in our Discord server.
- Can be compiled for every single installation to suit every server's instruction capabilities.
- Uses memory-safe modern C++ features.
- 100% FLOSS!

## Why C++?
I decided to make this script in C++ to help me learn it as well as get acceptable performance no matter how many features I add. 
There's also a lot of standard libraries available for C++ and that might be an advantage over something like shell script. Plus, "Modern C++" features are great for memory safety and beautifying code.

## What's up With That Name?
"Hajime" is simply "begin" in Japanese. I know this because I hear it every time I train with my judo instructor.

# Instructions (WIP)

## Pre-compiled binaries (from the [Releases](https://github.com/Slackadays/Hajime/releases) page)
We offer binaries for Windows systems that are as easy as a quick download.

## Setup
0. Install Hajime using either the provided binaries or the install script, and put the file in any suitable location.
1. Once you install Hajime using either a downloaded binary or the install script, use `-i` s a flag to set up the initial Hajime confguration file.
2. Do `-ss` next to set up what we call a "servers" file.
3. Next, do `-s` to set up the first server file.
4. Edit the created server file (`server0.conf`) with the desired settings.

### Using `systemd` to make Hajime a service 

If you would like to make a systemd service, now run `sudo ./hajime -S`
to make a systemd service file. By default, the file created is called **hajime.service**. If you would like a different name (for example, if you are running multiple servers) change the setting in the file **hajime.conf**. Now enable it using `sudo systemctl enable hajime`to run Hajime on startup. Before rebooting, you must change the settings in the **server.conf** and **hajime.conf** files. Make server.conf by running `sudo ./hajime -I` again. `server.conf` is the settings file for an individual server object. This is done for future-proofing for future versions that may implement multithreading. `hajime.conf` is the settings file for the main program.

## Compiling Your Own
It's easy to compile Hajime. First, download the files in the [**source**]./source/) section. Then, run this command:
```
g++ -std=c++20 -Ofast -o hajime hajime.cpp
```
Hajime requires at least **C++17** to work and C++20 for future compatibility.
   
# Using Hajime

## Important Note
You must use the command "screen" in the server execution command for Hajime to work. This limitation will be removed in a future update.

# Troubleshooting
If Hajime seems to not work after it's been running for a while, just restart it. This solves 99% of problems!
You may also open an issue in the repository if you find functionality you don't believe is intentional

# Proudly Supported by MacStadium
To develop Hajime for MacOS, MacStadium provides a Mac Mini to help support open source projects like this one.
<img src="MacStadium-developerlogo.png" alt="Hajime logo" width="300px"/>
