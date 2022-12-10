/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/

help.push_back("\033[1m¡Bienvenido a Hajime " + hajime_version + ", el programa definitivo para el encendido de servidores de Minecraft!\033[0m");
help.push_back("Esta versión de Hajime fue compilada esta fecha: " __DATE__ ".");
help.push_back("\033[32mUso:\033[0m ");
help.push_back("hajime(.exe) [los siguientes indicadores]");
help.push_back("-f archivo o --server-file archivo | Especificar un archivo para usar.");
help.push_back("-h o --help | Mostrar este mensaje de ayuda.");
help.push_back("-hf archivo o --hajime-file archivo | Especificar el archivo de configuración que usa Hajime.");
help.push_back("-s  u --install-server | Crear un archivo estándar para la configuración de un servidor.");
help.push_back("-ih archivo u --install-hajime-config archivo | Instalar el archivo estándar que usa Hajime.");
help.push_back("-S  u --install-service | Instalar un archivo de encendido para empezar Hajime.");
help.push_back("-v o --verbose | Activar los registros verbosos.");
help.push_back("-i u --install-hajime | Procesar el asistente de instalación de Hajime.");
help.push_back("-m o --monochrome or --no-colors | Desactivar los colores de los registros.");
help.push_back("-d o --debug | Activar los registros de depurar.");
help.push_back("-l o --language | Escoger el lenguaje que usará Hajime.");
help.push_back("-np o --no-pauses | Desactivar las pausas artificiales.");
help.push_back("-tc o --thread-colors | Fijar los colores por la identificación de los threads en lugar del tipo del mensaje.");
help.push_back("-it o --show-info-type | Fijar el tipo de información en los registros explícitamente.");
help.push_back("\033[32m¿Necesitas más ayuda?\033[0m Únete nuestro grupo de Discord aquí: https://discord.gg/J6asnc3pEG");
eno.NotPermitted = "No está permitido. ¿Es correcto el aparato?";
eno.NoFileOrDir = "No hay ese tipo de archivo ni directorio.";
eno.PermissionDenied = "No tiene el permiso necesario. ¿Estás ejecutando Hajime como el usuario root?";
eno.InOut = "Error de entra/salida. ¿Está OK el lector?";
eno.Memory = "No hay suficiente memoria.";
eno.Unavailable = "El recurso no está disponible.";
eno.Address = "La dirección es incorrecta.";
eno.BlockDev = "No es un dispositivo de bloques. ¿Estás accediendo un dispositivo de almacenamiento masivo?";
eno.Busy = "Está ocupado. ¿Se está accediendo ahora mismo?";
prefix.VInfo = "Info";
prefix.VError = "Error";
prefix.VWarning = "Aviso";
prefix.VDebug = "Depurar";
#if defined(_WIN64) || defined (_WIN32)
prefix.VQuestion = "Pregunta"; //Windows doesn't support 24 bit terminal colors
#else
prefix.VQuestion = "Pregunta";
#endif
error.NotEnoughArgs = "No hay suficientes argumentos";
error.ConfDoesNotExist = "¡El archivo de configuración {} no existe!";
error.NoHajimeConfig = "Archivo estándar de configuation de Hajime no existe";
error.StartupServiceWindowsAdmin = "Hay que ejecutar Hajime como el administrador para que instale el servicio de encendido.";
error.SystemdRoot = "Hay que ejecutar Hajime como el user root para que instale un servicio de systemd.";
error.NoSystemd = "Parece que no haya ningún sistema de encendido compatible";
error.ServerFileNotPresent = "El archivo de configuración para el servidor ({}) no existe";
error.CouldntSetPath = "No se pudo cambiar la dirección";
error.Generic = "Ocurrió un error: {}";
error.MethodNotValid = "El método no es válido";
error.CreatingDirectory = "Error al crear el directorio";
error.FilesInPath = "Hay archivos en la dirección";
error.Mount = "Ocurrió un error, pero Hajime tratará a continuar. El error: ";
error.Code = "Número del error: ";
error.HajFileNotMade = "No se creó el archivo de configuración para Hajime";
error.ServerConfNotCreated = "No se creó el archivo de configuración para el servidor";
error.OptionNotAvailable = "Disculpa, esta opción no todavía está disponible.";
error.InvalidServerNumber = "Número de servidor inválido";
error.ServerSelectionInvalid = "Selección de servidor inválida";
error.DoesntSupportWindows = "Esta característica no es compatible con Windows";
error.InvalidCommand = "Comando inválido; lista de comandos válidos:";
error.InvalidHajCommand1 = "term, t [servidor] | conectar al servidor";
error.InvalidServerCommand1 = ".d - desconectar del servidor";
warning.FoundSysvinitService = "Encontró un servicio de sysVinit instalado ya";
warning.FoundSystemdService = "Encontró un servicio de systemd instalado ya";
warning.IsRunningFalse = "isRunning está falso ahora";
warning.TestingWindowsSupport = "¡Está probando ser compatible con Windows!";
warning.LaunchdServPresent = "Encontró un servicio de launchd";
warning.FoundServerConf = "¡Hay un archivo de configuración aquí!";
warning.FoundServerConfPlusFile = "Hay un archivo de servidor con el nombre ";
warning.FoundHajConf = "¡Hay un archivo de configuración para Hajime aquí!";
question.MakeLaunchdServ = "¿Te gustaría crear un archivo de configuración para launchd de todas maneras?";
question.Prompt = "[s/n]";
question.MakeHajimeConfig = "Parece que no hay un archivo de configuración para Hajime. ¿Te gustaría crearlo?";
question.MakeServerConfig = "¿Te gustaría crear un nuevo archivo de configuración de todas formas?";
question.MakeNewSysvinitService = "¿Te gustaría instalar un nuevo servicio de sysVinit?";
question.WizardServerFile = "¿Te gustaría hacer el archivo del servidor ahora?";
question.WizardStartupService = "¿Te gustaría instalar un servicio de encendido ahora?";
question.SysvinitUser = "Por favor, pon el USER que usará Hajime. ";
question.SysvinitGroup = "Por favor, pon el GRUPO del user. ";
question.DoSetupInstaller = "Parece que no has ejecutado Hajime ya. ¿Te gustaría hacer el asistente de instalación?";
question.UseFlags = "¿Te gustaría usar indicadores de Java ya hechos con el servidor?";
question.InstallNewOne = "¿Te gustaría instalarlo?";
question.InstallNewOneAgain = "¿Te gustaría instalarlo otra vez?";
question.CreateAnotherServerFile = "¿Te gustaría crear otro archivo de servidor?";
question.StartHajime = "¿Te gustaría ejecutar Hajime ahora? Pon \"n\" para salir.";
question.ApplyConfigToServerFile = "¿Te gustaría hacer una configuración para el archivo del servidor?";
question.UseDefaultServerFile = "¿Te gustaría usar el archivo estándar del servidor ({}) u otra cosa?";
question.EnterNewServerFile = "Pon un archivo nuevo para el servidor: ";
question.EnterCustomFlags = "Pon tus indicadores a medidas: ";
option.MakeServerFileManually = "Poner un archivo de servidor manualmente";
option.DoManually = "Hacerlo manualmente";
option.EnterManually = "Ponerlo manualmente";
option.LetHajimeDeduce = "Dejar Hajime deducirlo para mí";
option.SkipStep = "Faltar a este paso";
option.UseDefault = "Usar el estándar";
option.ChooseOptionBelow = "Elige una opción abajo.";
option.YourChoice = "Tu selección: ";
option.AttendedInstallation = "Hacer una instalación con supervisión";
option.UnattendedInstallation = "Hacer una instalación sin supervisión";
option.SkipSetup = "Faltar a esta instalación";
option.AikarFlags = "Usar Aikar's Flags";
option.HillttyFlags = "Usar Hilltty's Flags";
option.FroggeMCFlags = "Usar FroggeMC's ZGC Flags";
option.BasicZGCFlags = "Usar indicadores básicos de ZGC";
option.CustomFlags = "Usar indicadores a medidas";
info.InstallingSysvinit = "Se está instalando el servicio de sysVinit";
info.InstallingNewSysvinit = "Se está instalando un nuevo servicio de sysVinit";
info.InstalledSysvinit = "Instaló el servicio de sysVinit en /etc/init.d/hajime.sh";
info.CreatedServerConfig = "El archivo de configuración ({}) se ha creado";
info.AbortedSysvinit = "Se terminó la instalación de un servicio de sysVinit";
info.NoLogFile = "No hay nada en el archivo de registros; está poniendo los registros al terminal.";
info.ReadingServerSettings = "Está leyendo las configuraciones del servidor...";
info.ServerFile = "Archivo del servidor: ";
info.ServerPath = "Dirección del servidor: ";
info.ServerCommand = "Comando del servidor: ";
info.ServerMethod =  "Método del encendido del servidor: ";
info.ServerDevice = "Dispositivo: ";
info.ServerDebug = "Número de depurar: ";
info.ServerIsRunning = "¡Está ejecutando el programa!";
info.TryingToStartProgram = "Está tratando a ejecutar el programa";
info.StartingServer = "¡Está ejecutando el servidor!";
info.ServerStartCompleted = "Está completado el encendido del servidor";
info.POSIXdriveMount = "Hay que montar con los sistemas de POSIX solamente";
info.TryingFilesystem = "Está probando el sistema de archivo {}";
info.TryingMount = "Está tratando a montar";
info.CreatingDirectory = "Está creando un nuevo directorio";
info.NoMount = "No hay nada aparato; ¡no tiene que montar!";
info.DeviceMounted = "¡Está montado el aparato!";
info.wizard.HajimeFile = "Empezamos hacer el archivo que usará Hajime para las configuraciones básicas.";
info.wizard.ServerFile = "Necesitamos un archivo de servidor para las configuraciones de un servidor.";
info.wizard.StartupService = "Finalmente, necesitamos ejecutar Hajime cuando enciende el SO.";
info.wizard.Complete = "¡Está completada la configuración!";
info.wizard.NextStepServerFile = "El Próximo Paso: Poner las configuraciones del servidor en {}";
info.InstallingDefServConf = "Está instalando un archivo estándar para el servidor de configuración en ";
info.InstallingNewServConf = "Está instalando un nuevo archivo de configuración para el servidor con el nombre ";
info.InstallingDefHajConf = "Está instalando un archivo estándar de configuración para Hajime ";
info.CheckingExistingFile = "Está buscando archivos actuales...";
info.HajConfigMade = "¡El archivo de configuración para Hajime ({}) se ha creado!";
info.InstallingWStartServ = "Está instalando un servicio de encendido para Windows";
info.TipAdministrator = "Consejo: Haz clic derecho el ícono del terminal y haz clic \"Run as administrator\"";
info.InstallingLaunchdServ = "Está instalando un servicio de encendido para launchd";
info.InstallingNewLaunchdServ = "Está instalando un nuevo servicio de encendido para launchd";
info.InstalledLaunchServ = "Instaló un servicio para launchd";
info.AbortedLaunchServ = "Terminó la instalación del servicio para launchd";
info.MakingSystemdServ = "Está haciendo un servicio de systemd en ";
info.EnterNewNameForServer = "Elige un nuevo nombre para el próximo arhcivo del servidor (el anterior fue {}): ";
debug.HajDefConfNoExist = "Intentó leer las configuraciones de {} pero el archivo no existe";
debug.ReadingReadsettings = "Está leyendo las configuraciones en readSettings()";
debug.ReadReadsettings = "Leyó las configuraciones con éxito del archivo ";
debug.flag.VecInFor = "flagVector[0] en el bucle For =";
debug.flag.VecOutFor = "flagVector[0] afuera el bucle For =";
debug.UsingOldMethod = "Está usando el método viejo";
debug.UsingNewMethod = "Está usando el nuevo método";
debug.ValidatingSettings = "Está verificando las configuraciones del servidor";
server.restart.minutes5 = "[Hajime] Este servidor reiniciará en 5 minutos";
server.restart.minutes15 = "[Hajime] Este servidor reiniciará en 15 minutos";
server.command.hajime.regex = ".hajime";
server.command.hajime.output = "[\"§6[Hajime] §fEste servidor está usando \",{\"text\":\"Hajime 0.1.9\",\"underlined\":true,\"color\":\"aqua\",\"clickEvent\":{\"action\":\"open_url\",\"value\":\"https://hajime.sh\"}}]";
server.command.time.regex = ".tiempo";
server.command.time.output = "[Hajime] El tiempo local de este servidor es §b";
server.command.help.regex = ".a(yuda)";
server.command.help.message.coinflip = "Lanzar una moneda.";
server.command.help.message.die = "Tirar un dado.";
server.command.help.message.discord = "Ver la invitación de Discord de Hajime.";
server.command.help.message.hajime = "Ver la versión de Hajime.";
server.command.help.message.help = "Ver este mensaje de ayuda.";
server.command.help.message.name = "Ver el nombre de este servidor en Hajime.";
server.command.help.message.time = "Ver el tiempo local de este servidor.";
server.command.help.message.uptime = "Ver el tiempo de funcionamiento de este servidor.";
server.command.help.output = "[Hajime] Posa el cursor sobre un comando para ver la acción.";
server.command.die.regex = ".dado";
server.command.die.output = "[Hajime] Tiró un dado y consiguió ";
server.command.coinflip.regex = ".lanzar";
server.command.coinflip.output.heads = "[Hajime] Lanzó una moneda y consiguió §bcara";
server.command.coinflip.output.tails = "[Hajime] Lanzó una moneda y consiguió §bcruz";
server.command.discord.regex = ".discord";
server.command.discord.output = "[\"[Hajime] Únete el grupo oficial Discord de Hajime en \",{\"text\":\"https://discord.gg/J6asnc3pEG\",\"underlined\":true,\"color\":\"aqua\",\"clickEvent\":{\"action\":\"open_url\",\"value\":\"https://discord.gg/J6asnc3pEG\"}}]";
server.command.name.regex = ".nombre";
server.command.name.output = "[Hajime] El nombre de este servidor es §b";
server.command.uptime.regex = ".tiempofun";
server.command.uptime.output = "[Hajime] El tiempo de funcionamiento de este servidor es {} minutos ({} horas)";
fileServerConfComment = "# Todo que sigue un # es un comentario.";
