help.push_back("\033[1m¡Bienvenido a Hajime, el programa definitivo para el encendido de servidores de Minecraft!\033[0m");
help.push_back("Esta versión de Hajime fue compilada esta fecha: " __DATE__ ".");
help.push_back("\033[1m\033[32mUso:\033[0m ");
help.push_back(" [los siguientes indicadores]");
help.push_back("\033[1m-f \033[3marchivo\033[0m o \033[1m--server-file \033[3marchivo \033[0m\033[1m|\033[0m Especificar un archivo para usar.");
help.push_back("\033[1m-h \033[0mo\033[1m --help |\033[0m Mostrar este mensaje de ayuda.");
help.push_back("\033[1m-hf \033[1m\033[3marchivo \033[0mo\033[1m --hajime-file\033[0m \033[1m\033[3marchivo \033[0m\033[1m|\033[0mEspecificar el archivo de configuración que usa Hajime.");
help.push_back("\033[1m-s  \033[0mu\033[1m --install-server \033[1m|\033[0m Crear un archivo estándar para la configuración de un servidor.");
help.push_back("\033[1m-ih \033[1m\033[3marchivo \033[0mu\033[1m --install-hajime-config \033[1m\033[3marchivo \033[0m\033[1m|\033[0m Instalar el archivo estándar que usa Hajime.");
help.push_back("\033[1m-S  \033[0mu\033[1m --install-service \033[1m|\033[0m Instalar un archivo de encendido para empezar Hajime.");
help.push_back("\033[1m-ss \033[0mu\033[1m --install-servers-file \033[1m|\033[0m Instalar un archivo para apuntar los servidores.");
help.push_back("\033[1m-v \033[0mo\033[1m --verbose \033[1m|\033[0m Activar los registros verbosos.");
help.push_back("\033[1m-i \033[0mu\033[1m --install-hajime \033[1m|\033[0m Procesar el asistente de instalación de Hajime.");
help.push_back("\033[1m-m \033[0mo\033[1m --monochrome \033[0mor\033[1m --no-colors \033[1m|\033[0m Desactivar los colores de los registros.");
help.push_back("\033[1m-d \033[0mo\033[1m --debug \033[1m|\033[0m Activar los registros de depurar.");
help.push_back("\033[1m-l \033[0mo\033[1m --language \033[1m|\033[0m Escoger el lenguaje que usará Hajime.");
help.push_back("\033[1m\033[32m¿Necesitas más ayuda?\033[0m Únete nuestro grupo de Discord aquí: https://discord.gg/J6asnc3pEG");
errnoNotPermitted = "No es permitido. ¿Es correcto el aparato?";
errnoNoFileOrDir = "No hay ese tipo de archivo ni directorio.";
errnoPermissionDenied = "No tiene el permiso necesario. ¿Estás ejecutando Hajime como el usuario root?";
errnoInOut = "Error de entra/salida. ¿Está OK el lector?";
errnoMemory = "No hay suficiente memoria.";
errnoUnavailable = "El recurso no está disponible.";
errnoAddress = "La dirección está mala.";
prefixVInfo = "\033[46m[Info ";
prefixVError = "\033[41m\033[33m[Error ";
prefixVWarning = "\033[33m[Aviso ";
prefixVDebug = "\033[105m[Depurar ";
#if defined(_WIN64) || defined (_WIN32)
prefixVQuestion = "\033[102m[Pregunta "; //Windows doesn't support 24 bit terminal colors
#else
prefixVQuestion = "\033[48;2;0;255;0m\033[38;2;108;104;161m[Pregunta ";
#endif
errorNotEnoughArgs = "No hay suficientes argumentos";
errorConfDoesNotExist1 = "¡El archivo de configuración ";
errorConfDoesNotExist2 = " no existe!";
errorNoHajimeConfig = "Archivo estándar de configuation de Hajime no existe";
errorStartupServiceWindowsAdmin = "Hay que ejecutar Hajime como el administrador para que instale el servicio de encendido.";
errorSystemdRoot = "Hay que ejecutar Hajime como el user root para que instale un servicio de systemd.";
errorNoSystemd = "Parece que no haya ningún sistema de encendido compatible";
errorServersFilePresent = "¡Hay un archivo de servidor ya!";
errorServerFileNotPresent1 = "El archivo de configuración para el servidor (";
errorServerFileNotPresent2 = ") no existe";
errorCouldntSetPath = "No pudo cambiar la dirección";
errorGeneric = "Ocurrió un error desconocido.";
errorMethodNotValid = "El método no es válido";
errorCreatingDirectory = "Error al crear el directorio";
errorFilesInPath = "Hay archivos en la dirección";
errorNoServersFile = "No hay nada archivo de servidor";
errorMount = "Ocurrió un error, pero Hajime tratará a continuar. El error: ";
errorCode = "Número del error: ";
warningFoundSysvinitService = "Encontró un servicio de sysVinit instalado ya";
warningFoundSystemdService = "Encontró un servicio de systemd instalado ya";
warningIsRunningFalse = "isRunning está falso ahora";
warningTestingWindowsSupport = "¡Está probando ser compatible con Windows!";
warningLaunchdServPresent = "Encontró un servicio de launchd";
warningFoundServerConf = "¡Hay un archivo de configuración aquí!";
warningFoundHajConf = "¡Hay un archivo de configuración para Hajime aquí!";
questionMakeLaunchdServ = "¿Te gustaría crear un archivo de configuración para launchd de todas maneras?";
questionPrompt = "[s/n]";
questionMakeHajimeConfig = "Parece que no haya un archivo de configuración para Hajime. ¿Te gustaría crearlo?";
questionMakeServerConfig = "¿Te gustaría crear un nuevo archivo de configuración de todas formas?";
questionMakeNewSysvinitService = "¿Te gustaría instalar un nuevo servicio de sysVinit?";
questionWizardServersFile = "¿Te gustaría instalarlo ahora?";
questionWizardServerFile = "¿Te gustaría hacer el archivo del servidor ahora?";
questionWizardStartupService = "¿Te gustaría instalar un servicio de encendido ahora?";
questionSysvinitUser = "Por favor, pon el USER que usará Hajime. ";
questionSysvinitGroup = "Por favor, pon el GRUPO del user. ";
questionDoSetupInstaller = "Parece que no has ejecutado Hajime ya. ¿Te gustaría hacer el asistente de instalación?";
questionStartHajime = "¿Te gustía ejecutar Hajime ahora? Pon \"n\" para exitar.";
infoInstallingSysvinit = "Se está instalando el servicio de sysVinit";
infoInstallingNewSysvinit = "Se está instalando un nuevo servicio de sysVinit";
infoInstalledSysvinit = "Instaló el servicio de sysVinit en /etc/init.d/hajime.sh";
infoCreatedServerConfig1 = "El archivo de configuración (";
infoCreatedServerConfig2 = ") se ha creado";
infoAbortedSysvinit = "Se terminó la instalación de un servicio de sysVinit";
infoNoLogFile = "No hay nada archivo de registros; está poniendo los registros al terminal.";
infoReadingServerSettings = "Está leyendo las configuraciones del servidor...";
infoServerFile = "Archivo del servidor: ";
infoServerPath = "Dirección del servidor: ";
infoServerCommand = "Comando del servidor: ";
infoServerMethod =  "Método del encendido del servidor: ";
infoServerDevice = "Dispositivo: ";
infoServerDebug = "Número de depurar: ";
infoServerIsRunning = "¡Está ejecutando el programa!";
infoTryingToStartProgram = "Está tratando a ejecutar el programa";
infoStartingServer = "¡Está ejecutando el servidor!";
infoServerStartCompleted = "Está completado el encendido del servidor";
infoPOSIXdriveMount = "Hay que montar con los sistemas de POSIX solamente";
infoTryingFilesystem1 = "Está probando el sistema de archivo ";
infoTryingFilesystem2 = "";
infoTryingMount = "Está tratando a montar";
infoCreatingDirectory = "Está creando un nuevo directorio";
infoNoMount = "No hay nada aparato; ¡no tiene que montar!";
infoDeviceMounted = "¡Está montado el aparato!";
infoWizardHajimeFile = "Empezamos hacer el archivo que usará Hajime para las configuraciones básicas.";
infoWizardServersFile = "Entonces, necesitamos instalar el \"archivo de servidores,\" donde Hajime buscará los servidores.";
infoWizardServerFile = "Necesitamos un archivo de servidor para las configuraciones de un servidor.";
infoWizardStartupService = "Finalmente, necesitamos ejecutar Hajime cuando enciende el SO.";
infoWizardComplete = "¡Está completada la configuración!";
infoWizardNextStepServerFile1 = "El Próximo Paso: Poner las configuraciones del servidor en ";
infoInstallingDefServConf = "Está instalando un archivo estándar para el servidor de configuración en ";
infoInstallingNewServConf = "Está instalando un nuevo archivo de configuración para el servidor con el nombre ";
infoInstallingDefHajConf = "Está instalando un archivo estándar de configuración para Hajime ";
infoCheckingExistingFile = "Está buscando archivos actuales...";
infoHajConfigMade1 = "¡El archivo de configuración para Hajime (";
infoHajConfigMade2 = ") se ha creado!";
infoInstallingWStartServ = "Está instalando un servicio de encendido para Windows";
infoTipAdministrator = "Consejo: Haz clic derecho el ícono del terminal y haz clic \"Run as administrator\"";
infoInstallingLaunchdServ = "Está instalando un servicio de encendido para launchd";
infoInstallingNewLaunchdServ = "Está instalando un nuevo servicio de encendido para launchd";
infoInstalledLaunchServ = "Instaló un servicio para launchd";
infoAbortedLaunchServ = "Terminó la instalación del servicio para launchd";
infoInstallingServersFile = "Está instalando el archivo de servidores en ";
infoCheckingExistingServersFile = "Está buscando archivos actuales de servidores...";
infoMadeServersFile = "¡Instaló el archivo de servidores!";
debugHajDefConfNoExist1 = "Intentó leer las configuraciones de ";
debugHajDefConfNoExist2 = " pero el archivo no existe";
debugReadingReadsettings = "Está leyendo las configuraciones en readSettings()";
debugReadReadsettings = "Leyó las configuraciones con éxito del archivo ";
debugFlagVecInFor = "flagVector[0] en el bucle For =";
debugFlagVecOutFor = "flagVector[0] afuera el bucle For =";
debugUsingOldMethod = "Está usando el método viejo";
debugUsingNewMethod = "Está usando el nuevo método";
debugValidatingSettings = "Está verificando las configuraciones del servidor";
fileServerConfComment = "# Todo que sigue un # es un comentario.";
