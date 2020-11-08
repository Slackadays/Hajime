Finish transition to installer.h

Add an interactive installation option

Make it a true daemon (to not depend on systemd) by using std::thread

Try to move away from system() to compensate for non-Bash shells

Add a start.sh compatibility option (offer to be middleware for existing scripts or programs that depend on start.sh)

Add "intelligent configuration" which detects for existing server .jar files and automagically configures the files

Add colors to the output messages

Add more file logging features (log rotation, auto-zip)

Add compatibility for sysVinit

Add one-step installation from command line (curl, wget?)
