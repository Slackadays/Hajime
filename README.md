# Hajime 
<img src="HJ.png" alt="Hajime logo" width="100"/>
A fully-featured Minecraft server startup script suite that offers a friendly user interface, blazing fast speeds, and wide compatibility.

[![CodeFactor](https://www.codefactor.io/repository/github/slackadays/hajime/badge)](https://www.codefactor.io/repository/github/slackadays/hajime)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/Slackadays/Hajime/CI)
![GitHub top language](https://img.shields.io/github/languages/top/Slackadays/Hajime)
![Discord](https://img.shields.io/discord/891817791525629952?color=blue&logo=Discord)
![GitHub all releases](https://img.shields.io/github/downloads/slackadays/hajime/total)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/slackadays/hajime)
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/slackadays/hajime/latest)
![GitHub contributors](https://img.shields.io/github/contributors/slackadays/hajime)
![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/hajime?style=social)

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
Download the latest version available in the Releases page or from Discord. If you'd like to compile Hajime yourself, the code will work with the msys2 package.

# Why?

## The Problem
Please see "Why Hajime" at [the Hajime website.](https://slackadays.github.io/Hajime/)

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
Hajime will automtically start the installation wizard if it cannot find the configuation file. This is automatically the case when you download Hajime. To do the installer manually, use `-i` as a flag option.

### Using `systemd` to make Hajime a service 

If you would like to make a systemd service yourself, run `sudo ./hajime -S`
to make a systemd service file. By default, the file created is called **hajime.service**. If you would like a different name (for example, if you are running multiple servers) change the setting in the file **hajime.conf**. Now enable it using `sudo systemctl enable hajime`to run Hajime on startup. Before rebooting, you must change the settings in the **server.conf** and **hajime.conf** files. Make server.conf by running `sudo ./hajime -I` again. `server.conf` is the settings file for an individual server object. This is done for future-proofing for future versions that may implement multithreading. `hajime.conf` is the settings file for the main program.

## Compiling 

You can compile Hajime with a one-liner command. To make it easier, you can also do `sh fakemake` in the root folder if you clone this repo. `fakemake` is simple shell script that contains that one-liner command and a couple messages for clarity.

Hajime requires at least **C++17** to work and C++20 for future compatibility.
   
# Troubleshooting
If Hajime seems to not work after it's been running for a while, just restart it. This solves 99% of problems!
You may also open an issue in the repository if you find functionality you don't believe is intentional

# Proudly Supported by MacStadium
To develop Hajime for MacOS, MacStadium provides a Mac Mini to help support open source projects like this one.
<img src="MacStadium-developerlogo.png" alt="Hajime logo" width="300px"/>
