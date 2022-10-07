package hajimehelper.hajimehelper;

import org.bukkit.command.CommandExecutor;
import org.bukkit.command.CommandSender;
import org.bukkit.command.Command;
import org.bukkit.command.ConsoleCommandSender;
import org.bukkit.entity.Player;
class CommandSetsecret implements CommandExecutor {
    @Override
    public boolean onCommand(CommandSender sender, Command command, String label, String[] args) {
        if (sender instanceof ConsoleCommandSender) {
            if (args.length == 1) {
                HajimeHelper.secret = args[0];
            } else {
                return false;
            }
            sender.sendMessage("Setting secret to " + HajimeHelper.secret);
        } else {
            sender.sendMessage("§4[Error]§b Only the console can run this command.");
        }
        return true;
    }
}
class CommandGeneric implements CommandExecutor {
    @Override
    public boolean onCommand(CommandSender sender, Command command, String label, String[] args) {
        if (sender instanceof Player) {
            HajimeHelper.sendCommand(command.getName(), sender, args);
        }
        return true;
    }
}