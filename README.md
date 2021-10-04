# Hajime
A versatile, high-performance Minecraft server startup script suite.

# Discord
For rapid support and announcements about Hajime, join our Discord at https://discord.gg/J6asnc3pEG.

# The Big Problem
Most other startup scripts are either bare-bones shell scripts or systemd services. Some utilities let you create a custom shell script for your server, but then you have to go a website and enter your server's information, copy that script, download it to the server, and more. Yuck! This "script" is a little different because it provides way more features and is blazing fast thanks to its compiled nature.

# Requirements
There are a couple prerequisites. You'll need a POSIX-compliant system, so this means Linux or FreeBSD or MacOS. Also, to use the one-step installation option, your platform will need to support Git and G++.

# Why C++?
I decided to make this script in C++ to help me learn it, as well as get at least acceptable performance no matter how many features I add. 
There's also a lot of standard libraries available for C++ and that might be an advantage over something like shell script. Plus, "Modern C++" features are great for memory safety and beautifying code.

# What's up With That Name?
"Hajime" is "begin" in Japanese. I know this because I hear it every time I train with my judo instructor.

# Quick Start
Use this command to install Hajime in one step. It will download and compile the latest version available on GitHub.
```
curl https://raw.githubusercontent.com/Slackadays/Hajime/master/install.sh | sh
```
Like with all shell scripts, before running, make sure to review the install.sh file to check for any potentially malicious commands.

# Instructions (WIP)

## Pre-compiled binaries (from the [Releases](https://github.com/Slackadays/Hajime/releases) page)
If you are using a precompiled binary, stay in this section. If you are compiling, skip this section. Download the latest version for your platform. Next, place it in a simple, memorable location. In my own server, I use `./media`. 
Now, run `sudo ./hajime -I` to install the initial configuration file. 

### Using `systemd` to make Hajime a service 

If you would like to make a systemd service, now run `sudo ./hajime -S`
to make a systemd service file. By default, the file created is called **hajime.service**. If you would like a different name (for example, if you are running multiple servers) change the setting in the file **hajime.conf**. Now enable it using `sudo systemctl enable hajime`to run Hajime on startup. Before rebooting, you must change the settings in the **server.conf** and **hajime.conf** files. Make server.conf by running `sudo ./hajime -I` again. `server.conf` is the settings file for an individual server object. This is done for future-proofing for future versions that may implement multithreading. `hajime.conf` is the settings file for the main program.

# Compiling Your Own
It's easy to compile Hajime. First, download the files in the [**source**]./source/) section. Then, run this command:
```
g++ -std=c++20 -Ofast -o hajime hajime.cpp
```
Hajime requires at least **C++17** to work.
   
# Using Hajime
This script is a cinch to actually use. To check its status, run `sudo systemctl status hajime` (or substitute "hajime" if you have renamed the systemd service file). This will display the messages sent by Hajime. For debugging, you may also run it manually.

# Troubleshooting
If Hajime seems to not work after it's been running for a while, just restart it. This solves 99% of problems!
You may also open an issue in the repository if you find functionality you don't believe is intentional
