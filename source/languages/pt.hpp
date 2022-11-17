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

help.push_back("\033[1mBem-vindo ao Hajime" + hajime_version + ", o melhor script para a inicialização de servidores de Minecraft!\033[0m");
help.push_back("Esta versão do Hajime foi compilada em " __DATE__ ".");
help.push_back("\033[32mUtilização:\033[0m ");
help.push_back("hajime(.exe) [as seguintes flags]"); //1 and 2 sandwich a variable
help.push_back("-f arquivo ou --server-file arquivo | Especificar um arquivo de configuração do servidor a ser usado manualmente.");
help.push_back("-h ou --help | Mostrar essa mensagem de ajuda.");
help.push_back("-hf arquivo ou --hajime-file arquivo | Especificar manualmente o arquivo de configuração utilizado.");
help.push_back("-s  ou --install-server | Criar um arquivo de configuração padrão para o servidor.");
help.push_back("-ih arquivo ou --install-hajime-config arquivo | Instalar o arquivo de configuração padrão do Hajime.");
help.push_back("-S  ou --install-service | Instalar um arquivo de inicialização de serviço para que sua plataforma inicie o Hajime automaticamente.");
help.push_back("-v ou --verbose | Habilitar logs exageradas.");
help.push_back("-i ou --install-hajime | Execute o assistente de instalação Hajime.");
help.push_back("-m ou --monochrome or --no-colors | Desabilitar a saída de cores.");
help.push_back("-d ou --debug | Habilitar mensagens de debug.");
help.push_back("-l ou --language | Escolher manualmente o idioma que será usado.");
help.push_back("-np ou --no-pauses | Desabilitar as pausas artificiais.");
help.push_back("-tc ou --thread-colors | Mostrar cores por ID ao invés de tipo de mensagem.");
help.push_back("-it ou --show-info-type | Exibir explicitamente o tipo de informação nas logs.");
help.push_back("\033[32mPrecisa de mais ajuda?\033[0m Junte-se ao nosso servidor do Discord em https://discord.gg/J6asnc3pEG");
eno.NotPermitted = "Não permitido. O dispositivo está certo?";
eno.NoFileOrDir = "Não existe tal arquivo ou diretório.";
eno.PermissionDenied = "Permissão negada. O Hajime está sendo administrado pelo root?";
eno.InOut = "Erro de entrada/saída. O disco está bem?";
eno.Memory = "Não há memória suficiente. Será que falta memória?";
eno.Unavailable = "Recurso indisponível.";
eno.Address = "Endereço inadequado.";
eno.BlockDev = "Não é um aparelho de blocos. Você se certificou de estar montando um dispositivo de armazenamento em massa?";
eno.Busy = "Ocupado. O dispositivo está sendo acessado neste momento?";
eno.Directory = "É um diretório. Você se certificou de que está montando um dispositivo de armazenamento em massa?";
eno.BadArgs = "Argumentos errados. A configuração está definida corretamente?";
eno.UnknownDev = "Dispositivo desconhecido. O sistema de arquivos é suportado?";
eno.UnknownGeneric = "Erro desconhecido.";
prefix.Info = "*";
prefix.Error = "X";
prefix.Warning = "!";
prefix.Debug = "+";
prefix.VInfo = "Info";
prefix.VError = "Erro";
prefix.VWarning = "Aviso";
prefix.VDebug = "Debug";
prefix.VQuestion = "Pergunta";
prefix.Question = "?";
error.NotEnoughArgs = "Não foram fornecidos argumentos suficientes";
error.ConfDoesNotExist = "Arquivo de configuração {} não existe!";
error.NoHajimeConfig = "Arquivo de configuração padrão do Hajime não encontrado.";
error.StartupServiceWindowsAdmin = "Você precisa executar a Hajime como administrador para instalar o serviço de inicialização.";
error.SystemdRoot = "Você precisa ser o usuário root para instalar o serviço systemd.";
error.NoSystemd = "Parece que não há systemd; em vez disso, use outra opção de instalação.";
error.ServerFileNotPresent = "O arquivo de configuração do servidor ({}) não existe.";
error.CouldntSetPath = "Não foi possível definir o caminho.";
error.Generic = "Whoops! Ocorreu um erro: {}";
error.MethodNotValid = "O método não é um tipo válido";
error.CreatingDirectory = "Erro ao criar diretório!";
error.FilesInPath = "Há arquivos no caminho";
error.Mount = "Ocorreu um erro, mas o roteiro vai continuar tentando montar. Erro: ";
error.Code = "Código de erro: ";
error.HajFileNotMade = "Arquivo de configuração do Hajime não foi criado";
error.ServerConfNotCreated = "Arquivo de configuração do servidor não foi criado";
error.OptionNotAvailable = "Desculpe, essa opção ainda não está disponível.";
error.InvalidServerNumber = "Número de servidor inválido";
error.ServerSelectionInvalid = "Seleção do servidor inválida";
error.DoesntSupportWindows = "O Windows não suporta esta função.";
error.InvalidCommand = "Comando inválido; lista de comandos válidos:";
error.InvalidHajCommand1 = "term, t [servidor] | anexar ao servidor";
error.InvalidServerCommand1 = ".d - detach from server";
warning.FoundSysvinitService = "O serviço sysVinit instalado foi encontrado.";
warning.FoundSystemdService = "O serviço systemd instalado foi encontrado.";
warning.IsRunningFalse = "isRunning está desativado.";
warning.TestingWindowsSupport = "Testando o suporte a Windows!";
warning.LaunchdServPresent = "Encontrei um serviço de launchd existente.";
warning.FoundServerConf = "Encontrei um arquivo de configuração de servidor existente.";
warning.FoundServerConfPlusFile = "Encontrei um arquivo de servidor existente chamado ";
warning.FoundHajConf = "O arquivo de configuração do Hajime já se encontra disponível";
question.MakeLaunchdServ = "Você gostaria de fazer um novo mesmo assim?";
question.Prompt = "[s/n]";
question.MakeHajimeConfig = "Você gostaria de fazer o arquivo de configuração para o Hajime?";
question.MakeServerConfig = "Você gostaria de criar um novo arquivo de configuração mesmo assim?";
question.MakeNewSysvinitService = "Você gostaria de instalar um novo serviço sysVinit?";
question.WizardServerFile = "Você deseja fazer um arquivo de servidor agora?";
question.WizardStartupService = "Você deseja instalar um serviço de inicialização?";
question.SysvinitUser = "Por favor, digite o USUÁRIO sob o qual você quer que a Hajime funcione. ";
question.SysvinitGroup = "Favor digite o GRUPO do usuário que você inseriu. ";
question.DoSetupInstaller = "Parece que é a primeira vez que você usa o Hajime. Você deseja executar o instalador?";
question.StartHajime = "Você quer iniciar o Hajime agora? Digite \"n\" para sair.";
question.UseFlags = "Você gostaria de aplicar alguns argumentos java previamente feitos ao seu servidor?";
question.InstallNewOne = "Você quer instalar um novo?";
question.InstallNewOneAgain = "Você quer tentar criar um novamente?";
question.CreateAnotherServerFile = "Você quer criar outro arquivo de servidor?";
question.ApplyConfigToServerFile = "Você gostaria de aplicar uma configuração ao arquivo do servidor?";
question.UseDefaultServerFile = "Você gostaria de usar o arquivo padrão do servidor ({}) ou alguma outra coisa?";
question.EnterNewServerFile = "Digite um novo arquivo de servidor: ";
question.EnterCustomFlags = "Digite aqui suas flags personalizadas: ";
option.MakeServerFileManually = "Digite um arquivo de servidor manualmente";
option.DoManually = "Faça isso manualmente";
option.EnterManually = "Digite manualmente";
option.LetHajimeDeduce = "Deixe o Hajime deduzir para mim [Não implementado]";
option.SkipStep = "Pular esta etapa";
option.UseDefault = "Utilizar o padrão";
option.ChooseOptionBelow = "Escolha uma das opções abaixo.";
option.YourChoice = "Sua escolha: ";
option.AttendedInstallation = "Fazer uma instalação acompanhada";
option.UnattendedInstallation = "Fazer uma instalação desacompanhada";
option.SkipSetup = "Pular setup";
option.AikarFlags = "Utilizar as flags de Aikar";
option.HillttyFlags = "Utilizar as flags de Hilltty";
option.FroggeMCFlags = "Utilizar as flags de FroggeMC com ZGC";
option.BasicZGCFlags = "Utilizar as flags básicas de ZGC";
option.CustomFlags = "Utilizar flags customizadas";
info.InstallingSysvinit = "Instalando o serviço sysVinit";
info.InstallingNewSysvinit = "Instalando o novo serviço sysVinit";
info.InstalledSysvinit = "Serviço sysVinit instalado em /etc/init.d/hajime.sh";
info.CreatedServerConfig = "O arquivo de configuração ({}) foi criado";
info.AbortedSysvinit = "Abortar a instalação do serviço sysVinit";
info.NoLogFile = "Nenhum arquivo de log será feito; enviando mensagens para o console.";
info.ReadingServerSettings = "Lendo as configurações do servidor...";
info.ServerFile = "Arquivo do servidor: ";
info.ServerPath = "Caminho do servidor: ";
info.ServerCommand = "Comando do servidor: ";
info.ServerMethod =  "Método de inicialização do servidor: ";
info.ServerDevice = "Dispositivo: ";
info.ServerDebug = "Valor de debug: ";
info.ServerIsRunning = "O programa está funcionando!";
info.TryingToStartProgram = "Tentando iniciar o programa";
info.StartingServer = "Iniciando o servidor!";
info.ServerStartCompleted = "Início do programa concluído";
info.POSIXdriveMount = "A montagem do disco só é necessária em sistemas POSIX";
info.TryingFilesystem = "Tentando o sistema de arquivos {}";
info.TryingMount = "Tentando montar";
info.CreatingDirectory = "Fazendo um novo diretório";
info.NoMount = "Nenhum dispositivo solicitado; nenhuma montagem dessa vez!";
info.DeviceMounted = "Dispositivo montado!";
info.wizard.HajimeFile = "Vamos começar com a criação do arquivo que o Hajime utilizará para suas configurações básicas.";
info.wizard.ServerFile = "Agora precisamos de um arquivo de servidor para definir um de seus servidores a ser executado.";
info.wizard.StartupService = "Por último, precisamos fazer com que o Hajime inicie na inicialização do host.";
info.wizard.Complete = "Instalação completa!";
info.wizard.NextStepServerFile = "Próximos Passos: Digite as configurações de seu servidor em {}.";
info.InstallingDefServConf = "Instalando o arquivo de configuração padrão do servidor em ";
info.InstallingNewServConf = "Instalação de um novo arquivo de configuração de servidor chamado ";
info.InstallingDefHajConf = "Instalando o arquivo de configuração padrão do Hajime ";
info.CheckingExistingFile = "Procurando por um arquivo existente...";
info.HajConfigMade = "Arquivo de configuração do Hajime ({}) feito!";
info.InstallingWStartServ = "Instalando o serviço de inicialização do Windows";
info.TipAdministrator = "Dica: Clique com o botão direito do mouse no ícone do terminal e depois clique em \"Executar como Administrador\"";
info.InstallingLaunchdServ = "Instalando o serviço launchd";
info.InstallingNewLaunchdServ = "Instalando o novo serviço launchd";
info.InstalledLaunchServ = "Serviço sysVinit instalado";
info.AbortedLaunchServ = "Instalação do serviço de inicialização abortado";
info.MakingSystemdServ = "Criando o serviço systemd em ";
info.EnterNewNameForServer = "Digite um novo nome para o próximo arquivo de servidor (o anterior foi {}): ";
debug.HajDefConfNoExist = "Tentei ler as configurações de {} mas elas não existem";
debug.ReadingReadsettings = "Configurações de leitura em readSettings()";
debug.ReadReadsettings = "Configurações lidas com sucesso de ";
debug.flag.VecInFor = "flagVector[0] no loop For =";
debug.flag.VecOutFor = "flagVector[0] fora do loop For  =";
debug.UsingOldMethod = "Utilizando o método antigo";
debug.UsingNewMethod = "Utilizando o novo método";
debug.Flags = "Flags =";
debug.flag.Array0 = "flagArray[0] =";
debug.flag.Array1 = "flagArray[1] =";
debug.ValidatingSettings = "Verificando as configurações do servidor";
fileServerConfComment = "# Qualquer coisa depois de um # é um comentário.";
