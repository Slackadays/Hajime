package hajimehelper.hajimehelper;

import org.bukkit.command.CommandExecutor;
import org.bukkit.plugin.java.JavaPlugin;

public final class HajimeHelper extends JavaPlugin {
    @Override
    public void onEnable() {
        // Plugin startup logic
        getCommand("hajime").setExecutor(new CommandHajime());
    }

    @Override
    public void onDisable() {
        // Plugin shutdown logic
    }
}