~~Finish transition to installer.h~~ DONE

Add an interactive installation option

Make it a true daemon (to not depend on systemd) by using std::thread

Try to move away from system() to compensate for non-Bash shells

Add a start.sh compatibility option (offer to be middleware for existing scripts or programs that depend on start.sh)

Add "intelligent configuration" which detects for existing server .jar files and automagically configures the files

~~Add colors to the output messages~~ DONE

Add more file logging features (log rotation, auto-zip)

Add compatibility for sysVinit

~~Add one-step installation from command line (curl, wget?)~~ DONE: see command in readme

Add multi-server support with multithreading (tricky, requires dynamic allocation of objects)

Add bStats support (create a dummy Java file just for bStats and run it upon startup)

~~Change the default server.conf to server.server for more descriptive filenames~~ WON'T ADD

Add a feature that explains each flag when prefixed with -h or --help

Add a feature to install server .jar files with one step
