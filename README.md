# Hajime
A versatile, high-performance Minecraft server startup script suite.

# The Problem
Most Minecraft server startup scripts are either one of these two: simple systemd services, or complicated Bash scripts. The problems with a systemd service are that they're inflexible, unintelligable, and systemd-reliant. Bash scripts are slightly better, as they provide more features. However, you still need a shell to run it on and you don't have as many language features and speed as with "regular" programs. This new, improved script fixes all of this.

# Requirements
There are several prerequisites as of this writing. You'll need a Linux, or at least POSIX-compliant, system. Hajime was designed to be used with servers that have all server info on an external mass storage device such as a USB flash drive, although this feature is now optional. Your system will also need to be 64-bit x86 to use the included binaries, but it can be compiled to any other system.

# What's up With That Name?
"Hajime" is just "begin" in Japanese. I know this because I hear it every time I train with my judo instructor.

# Instructions
If you are using a precompiled binary, stay in this section. If you are compiling, skip this section. Download the version appropriate for your platform. Next, place it in a simple, memorable location. In my own server, I use /media. Now, run 

    sudo ./hajime -I
to install the initial configuration file. If you would like to make a systemd service, now run

    sudo ./hajime -S
to make a systemd service file. By default, the file created is called **hajime.service**. If you would like a different name (for example, if you are running multiple servers) change the setting in the file **hajime.conf**. Now enable it using

    sudo systemctl enable hajime
to run Hajime on startup. Before rebooting, you must change the settings in **server.conf** and **hajime.conf**. Make server.conf by doing 

    sudo ./hajime -I
    
again. Server.conf is the settings file for an individual server object. This is done for future-proofing for future versions that may implement multithreading. Hajime.conf is the settings file for the main program.

# Compiling Your Own
It's easy to compile Hajime. First, download the files in the **source** section. Then, run this command:

    sudo g++ -std=c++20 -O3 -o hajime hajime.cpp -lstdc++fs
The "-std=c++20" is required because GCC likes to revert to old versions of C++.
   
# Using Hajime
This script is a cinch to actually use. To check its status, run

    sudo systemctl status hajime
and substitute "hajime" if you have renamed the systemd service file. This will display the messages sent by Hajime. For debugging, you may also run it manually.

# Troubleshooting
If Hajime seems to not work after it's been running for a while, just restart it. This solves 99% of problems!

# How It Works
Hajime is a fairly simple program, all things considered. It starts by performing file checks followed by a scan of the /proc virtual filesystem to get a maximum PID. This is done for optimization. Then, a device is mounted (if set to do so) and the specified command is run. A perpetual check of /proc is now done to search for a PID that has a particular keyword in it. If the Minecraft server stops, then the keyword is no longer present and the cycle starts over again.
