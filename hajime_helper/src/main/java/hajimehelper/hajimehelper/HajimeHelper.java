package hajimehelper.hajimehelper;

import org.bukkit.Bukkit;
import org.bukkit.command.CommandSender;
import org.bukkit.plugin.java.JavaPlugin;

public final class HajimeHelper extends JavaPlugin {
    static String secret = " ";
    @Override
    public void onEnable() {
        // Plugin startup logic
        getCommand("setsecret").setExecutor(new CommandSetsecret());
        getCommand("hajime").setExecutor(new CommandGeneric());
        getCommand("hajitime").setExecutor(new CommandGeneric());
        getCommand("hajihelp").setExecutor(new CommandGeneric());
        getCommand("die").setExecutor(new CommandGeneric());
        getCommand("d20").setExecutor(new CommandGeneric());
        getCommand("coinflip").setExecutor(new CommandGeneric());
        getCommand("hajidiscord").setExecutor(new CommandGeneric());
        getCommand("name").setExecutor(new CommandGeneric());
        getCommand("info").setExecutor(new CommandGeneric());
        getCommand("uptime").setExecutor(new CommandGeneric());
        getCommand("system").setExecutor(new CommandGeneric());
        getCommand("hajirestart").setExecutor(new CommandGeneric());
        getCommand("perf").setExecutor(new CommandGeneric());
        getCommand("hwperf").setExecutor(new CommandGeneric());
        getCommand("swperf").setExecutor(new CommandGeneric());
        getCommand("caperf").setExecutor(new CommandGeneric());

        int pluginId = 16604;
        Metrics metrics = new Metrics(this, pluginId);
    }
    @Override
    public void onDisable() {
        // Plugin shutdown logic
    }
    static public void sendCommand(String command, CommandSender sender, String[] args) {
        if (!command.equals("hajime")) {
            command = command.replace("haji", "");
        }
        String output = '.' + command + ' ' + secret + ' ' + sender.getName();
        if (args.length > 0) {
            for (String arg : args) {
                output += ' ' + arg;
            }
        }
        Bukkit.getServer().getConsoleSender().sendMessage(output);
    }
}