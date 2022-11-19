# Hajime <img src="HJ.png" alt="Hajime logo" width="50"/>

An advanced Minecraft server startup system

[![CodeFactor](https://www.codefactor.io/repository/github/slackadays/hajime/badge)](https://www.codefactor.io/repository/github/slackadays/hajime)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/18effdc4e4ca4d62ae5d160314f6f200)](https://www.codacy.com/gh/Slackadays/Hajime/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Slackadays/Hajime&amp;utm_campaign=Badge_Grade)
![](https://tokei.rs/b1/github/Slackadays/Hajime?category=lines)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/Slackadays/Hajime/CI)
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/slackadays/hajime/latest)
![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/hajime?style=social)

[Hajime en español](README_es.md)

## 💬 Join our Discord server at https://discord.gg/J6asnc3pEG


## ⬇️ Get Hajime 
### 🐧🍎😈 Linux, macOS, & FreeBSD
Copy and paste the below command into your terminal to install. Or, go to [GitHub Releases](https://github.com/Slackadays/Hajime/releases/) and download the latest release.
```
sh <(curl -L https://gethaji.me)
```
### 🪟 Windows 
Go to [GitHub Releases](https://github.com/Slackadays/Hajime/releases/) and download the latest release.
## 🖵 Tutorial Videos 
Check out the official Hajime tutorial video series!
[YouTube | ](https://www.youtube.com/channel/UC0DeCW6yXXVr9DJctJVo7wg)
[Odysee | ](https://odysee.com/@TheHajimeProject)
[Rumble | ](https://rumble.com/user/TheHajimeProject)
[BitChute ](https://www.bitchute.com/channel/DyRXhLP4Ghxd/)

## 👋 Introduction 

Imagine your Minecraft server working fine, until it isn't. 
Or, your server is running too slow for your liking. 
Or, you want to gain insights into performance. 
Or, you just want to get playing on Minecraft.
Introducing Hajime.

## ❓ Why Hajime

Hajime fills in a gaping void in the Minecraft server management world. On one end of the spectrum, you have startmc.sh. startmc.sh is super easy to use but offers no features at all. On the other end there are mark2 and Pterodactyl. These offer a lot of features, but are difficult to install and use. Hajime changes all of this by aiming to be extremely easy to use while offering all the features you could possibly want. 

As a comparison, remember the original iPhone presentation in 2007 when Steve Jobs showed off a graph of the iPhone putting all competitors to shame in smartness and ease-of-use at the same time.

## 🎛️ Features 
- Keeps your server running in the background!
- Super simple setup and installation!
- Monitor your server with advanced performance counters!
- Optimize your server with Aikar's Flags and more!
- Works with English, Spanish, and Portuguese!
- Run multiple servers at once!
- Works with almost all server platforms available!
- Super customizable!

### 🙋 What is "Hajime?" 
"Hajime" is simply "begin" in Japanese when talking about games. I know this because I hear it every time I train with my judo instructor.

## ✅ Instructions 

### 🪛 Setup 
Hajime will start the installation wizard if it can't find the configuation file, which will happen if this is your first time. Add `-i` as a flag option to do it manually.

For more info, check out the [Hajime Wiki!](https://github.com/Slackadays/Hajime/wiki)

### ⚙️ Compiling with CMake 
Step 0 (If Required): **Install** library dependencies

**Ubuntu/Debain Linux:** `sudo apt install libncurses-dev libsensors-dev libboost-all-dev libz-dev`

**Other Linux Versions:** Change `apt` to what your package manager uses.

**MacOS Homebrew:** `brew install boost ncurses`

**Windows:** Use Chocolatey to install Boost.

Step 1: **Clone** Hajime
```
git clone --recursive https://github.com/slackadays/Hajime
```
Step 2: **Compile** Hajime
```
cmake Hajime/source
cmake --build Hajime -j 8
```
The resulting `hajime` file should now appear.

If you're on Windows, add `--config Release` to the end of the last command to get a faster Release build.
   
## 🆘 Troubleshooting 
Open an issue in GitHub Issues or get quick support in our Discord server.

## 🍎 Supported by MacStadium 
To develop Hajime for MacOS, MacStadium provides a Mac Mini to help support open source projects like this one.
<img src="MacStadium-developerlogo.png" alt="Hajime logo" width="300px"/>
