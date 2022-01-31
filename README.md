# Hajime 
<img src="HJ.png" alt="Hajime logo" width="100"/>
The ultimate Minecraft server startup script!

[![](https://tokei.rs/b1/github/Slackadays/Hajime?category=lines)](https://github.com/Slackadays/Hajime)
[![CodeFactor](https://www.codefactor.io/repository/github/slackadays/hajime/badge)](https://www.codefactor.io/repository/github/slackadays/hajime)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/Slackadays/Hajime/CI)
![Discord](https://img.shields.io/discord/891817791525629952?color=blue&logo=Discord)
![GitHub all releases](https://img.shields.io/github/downloads/slackadays/hajime/total)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/slackadays/hajime)
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/slackadays/hajime/latest)
![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/hajime?style=social)

# Discord 💬
Join our Discord server at https://discord.gg/J6asnc3pEG!

# Quick Installation ⬇️
## Linux/macOS/\*BSD 🐧🍎👿🐡🏳‍🟧‍
Copy and paste this command into the terminal to install. Or, go to the Releases page and download the latest release.
```
sh <(curl -L https://gethaji.me)
```

## Windows 🪟
Go to the Releases page and download the latest release.

## Tutorial Videos 🖵

Check out the official Hajime tutorial video series!

**YouTube:** https://www.youtube.com/channel/UC0DeCW6yXXVr9DJctJVo7wg

**Odysee:** https://odysee.com/@TheHajimeProject

**Rumble:** https://rumble.com/user/TheHajimeProject

**BitChute:** https://www.bitchute.com/channel/DyRXhLP4Ghxd/

# An Introduction 👋
Hajime fills in a gaping void in the Minecraft server management world. On one end of the spectrum, you've got tools like startmc.sh. These are super easy to use but offer no features at all. On the other end, you have mark2 and Pterodactyl Panel. These offer all the features you could possibly want, but are difficult to install and use. Hajime changes all of this by aiming to be as easy to use as a shell script yet offering all the features you could possibly want. Imagine the original iPhone presentation in 2007 when Steve Jobs showed off a graph of the iPhone putting all competitors to shame in "smartness" and ease-of-use.

## Features 🎛️
- Dead-simple guided installation and setup!
- Supports Aikar's Flags, Hiltty's Flags, and ZGC flags out of the box!
- Supports English, Spanish, and Portuguese!
- Works with as many servers as you want all at once!
- Works with almost all platforms available!
- Ultra-customizable!
- Memory-safe!

## Why C++? 🤷
Other languages like Python may seem technically better, but using Python creates unnecessary dependencies and is a pain to deal with. Plus, with C++, we get access to certain features that other languages don't offer.

## What is "Hajime?" 🙋
"Hajime" is simply "begin" in Japanese when talking about games. I know this because I hear it every time I train with my judo instructor.

# Instructions (WIP) ✅

## Pre-compiled binaries (from the [Releases](https://github.com/Slackadays/Hajime/releases) page) 📦
Your platform is probably supported here. If it isn't, CMake is here to help.

## Setup 🪛
Hajime will automtically start the installation wizard if it cannot find the configuation file. This is automatically the case when you download Hajime. To do the installer manually, use `-i` as a flag option.

## Compiling 📚

### CMake ⚙️
Hajime can use CMake to compile. If you've just cloned the repo, these two commands will do the trick:
```
cmake source
cmake --build . -j 8
```
If you're on Windows, add `--config Release` to the end of the second command to get a Release build.

### Fakemake 🤫
Hajime can also use what we call the `fakemake` script. It's nothing more than a shell script that can compile Hajime in one step. Use fakemake by simply running
`sh fakemake`. Fakemake will only work on POSIX systems.
   
# Troubleshooting 🆘
Open a GitHub Issue or get quick support in our Discord server.

# Supported by MacStadium 🍎
To develop Hajime for MacOS, MacStadium provides a Mac Mini to help support open source projects like this one.
<img src="MacStadium-developerlogo.png" alt="Hajime logo" width="300px"/>
