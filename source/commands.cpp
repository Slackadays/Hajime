#include "server.hpp"

#include <random>

void Server::commandHajime() {
	writeToServerTerminal(formatWrapper(text.server.command.hajime.output));
}

void Server::commandTime() {
	std::time_t timeNow = std::time(nullptr);
	string stringTimeNow = std::asctime(std::localtime(&timeNow));
	stringTimeNow.erase(std::remove(stringTimeNow.begin(), stringTimeNow.end(), '\n'), stringTimeNow.end());
	string hajInfo = text.server.command.time.output + stringTimeNow + '\"';
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandHelp() {
	writeToServerTerminal(formatWrapper(text.server.command.help.output));
	writeToServerTerminal(formatWrapper("[{\"text\":\"" + text.server.command.coinflip.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.coinflip + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.coinflip.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.die.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.die + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.die.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.d20.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.d20 + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.d20.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.discord.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.discord + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.discord.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.hajime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hajime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.hajime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.help.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.help + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.help.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.name.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.name + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.name.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.info.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.info + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.info.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.time.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.time + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.time.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.uptime.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.uptime + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.uptime.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.system.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.system + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.system.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.perf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.perf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.perf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.hwperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.hwperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.hwperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.swperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.swperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.swperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.caperf.regex + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.caperf + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.caperf.regex, std::regex("(\\(|\\))"), "") + "\"}},"
	"{\"text\":\"" + text.server.command.restart.regex + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.help.message.restart + "\"},\"clickEvent\":{\"action\":\"suggest_command\",\"value\":\"" + std::regex_replace(text.server.command.restart.regex, std::regex("(\\(|\\))"), "") + "\"}}]"));
}

void Server::commandDie() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> die(1, 6);
	string hajInfo = text.server.command.die.output + std::to_string(die(mt));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandD20() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> die(1, 20);
	string hajInfo = text.server.command.d20.output + std::to_string(die(mt));
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
	//switch this to C++20 format when it becomes supported
}

void Server::commandCoinflip() {
	std::random_device rand;
	std::mt19937 mt(rand());
	std::uniform_int_distribution<int> flip(1, 2);
	if (flip(mt) == 1) {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.heads));
	} else {
		writeToServerTerminal(formatWrapper(text.server.command.coinflip.output.tails));
	}
}

void Server::commandDiscord() {
	writeToServerTerminal(formatWrapper(text.server.command.discord.output));
}

void Server::commandName() {
	string hajInfo = text.server.command.name.output + name;
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandInfo() {
	if (customMessage != "") {
		string hajInfo = "[Hajime] " + customMessage;
		writeToServerTerminal(formatWrapper(hajInfo));
	} else {
		writeToServerTerminal(formatWrapper("[Hajime] This server does not have a custom message set."));
	}
}

void Server::commandUptime() {
	string hajInfo = text.server.command.uptime.output1 + std::to_string(uptime) + text.server.command.uptime.output2 + std::to_string(uptime / 60.0) + text.server.command.uptime.output3;
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandRestart() {
	string hajInfo;
	if (restartMins > 0) {
		hajInfo = text.server.command.restart.output1 + std::to_string(restartMins - uptime) + text.server.command.restart.output2 + std::to_string((restartMins - uptime) / 60.0) + text.server.command.restart.output3;
	} else {
		hajInfo = text.server.command.restart.outputDisabled;
	}
	writeToServerTerminal(formatWrapper(addNumberColors(hajInfo)));
}

void Server::commandSystem() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.system.key.os + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getOS() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.cpu + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPU() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.ram + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAM() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.swap + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getSwap() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.uptime + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getUptime() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.processes + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getProcesses() + "\"}},"
	"{\"text\":\"" + text.server.command.system.key.loadavg + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLoadavg() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandPerf() {
	writeToServerTerminal(formatWrapper("[Hajime] Roll over an item to show its explanation."));
	string hajInfo;
	hajInfo = "[{\"text\":\"" + text.server.command.perf.key.cpuusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cpuusage + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ramusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ramusage + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cpumigrations + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cpumigrations + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ipc + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ipc + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cps + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cps + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ips + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.ips + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.minorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.minorpagefaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.majorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.majorpagefaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.contextswitches + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.contextswitches + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledfrontend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.stalledfrontend + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledbackend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.stalledbackend + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.buscycles + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.buscycles + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.branchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.branchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cachemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.cachemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.emufaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.emufaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.alignfaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.alignfaults + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1dprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1ireadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1ireadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1iprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.l1iprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.llcprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbwritemisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.dtlbprefetchmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.itlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.itlbreadmisses + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.bpureadmisses + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + text.server.command.perf.value.bpureadmisses + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandHWPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.cpuusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUusage() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ramusage + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getRAMusage() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ipc + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPC() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cps + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPS() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.ips + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getIPS() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.branchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBranchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cachemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCacheMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledfrontend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesFrontend() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.stalledbackend + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getStalledCyclesBackend() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.buscycles + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBusCycles() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandSWPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.contextswitches + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getContextSwitches() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.cpumigrations + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getCPUmigs() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.emufaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getEmulationFaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.alignfaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getAlignmentFaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.minorpagefaults + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getMinorPagefaults() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.majorpagefaults + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getMajorPagefaults() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}

void Server::commandCAPerf() {
	string hajInfo;
	hajInfo = "[{\"text\":\"[Hajime] \"},{\"text\":\"" + text.server.command.perf.key.l1dreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1dwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1dWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1ireadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1iReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.l1iprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getL1iPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLLWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.llcprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getLLPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbwritemisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBWriteMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.dtlbprefetchmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getdTLBPrefetchMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.itlbreadmisses + ", \",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getiTLBReadMisses() + "\"}},"
	"{\"text\":\"" + text.server.command.perf.key.bpureadmisses + "\",\"hoverEvent\":{\"action\":\"show_text\",\"value\":\"§b" + getBPUReadMisses() + "\"}}]";
	writeToServerTerminal(formatWrapper(hajInfo));
}