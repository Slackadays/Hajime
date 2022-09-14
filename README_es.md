# Hajime 
<img src="HJ.png" alt="Hajime logo" width="100"/>
¡El programa definitivo para el encendido de servidores de Minecraft!

[![](https://tokei.rs/b1/github/Slackadays/Hajime?category=lines)](https://github.com/Slackadays/Hajime)
[![CodeFactor](https://www.codefactor.io/repository/github/slackadays/hajime/badge)](https://www.codefactor.io/repository/github/slackadays/hajime)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/18effdc4e4ca4d62ae5d160314f6f200)](https://www.codacy.com/gh/Slackadays/Hajime/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Slackadays/Hajime&amp;utm_campaign=Badge_Grade)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/Slackadays/Hajime/CI)
![Discord](https://img.shields.io/discord/891817791525629952?color=blue&logo=Discord)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/slackadays/hajime)
![GitHub commits since latest release (by date)](https://img.shields.io/github/commits-since/slackadays/hajime/latest)
![GitHub Repo stars](https://img.shields.io/github/stars/slackadays/hajime?style=social)

# Discord 💬
¡Únete nuestro grupo de Discord en https://discord.gg/J6asnc3pEG!

# Instalación Rápida ⬇️
## Linux/macOS/\*BSD 🐧🍎👿🐡🏳‍🟧‍
Copia y pega este comando al terminal para instalar, o va a la página Releases y descarga la versión actual.
```
sh <(curl -L https://gethaji.me)
```
## Windows 🪟
Va a la página Releases y descarga la versión actual.

## Videos Tutoriales 🖵

¡Mira la serie oficial de videos tutoriales para Hajime!

**YouTube:** https://www.youtube.com/channel/UC0DeCW6yXXVr9DJctJVo7wg

**Odysee:** https://odysee.com/@TheHajimeProject

**Rumble:** https://rumble.com/user/TheHajimeProject

**BitChute:** https://www.bitchute.com/channel/DyRXhLP4Ghxd/

# Introducción 👋

Hajime completa un vacío en el mundo del servidor Minecraft. Otras herramientas, incluyendo startmc.sh, son muy fáciles pero no ofrecen ninguna característica útil. Sin embargo, otras como mark2 y Pterodactyl ofrecen tantas características, pero son difíciles usar. Hajime es diferente. Se ofrecen todas las características y el usar fácil que todos nosotros queremos. 

## Características 🎛️
- ¡Instalación y configuración súper fácil!
- Aikar's Flags, Hilltty's Flags y ZGC son listos para usar.
- ¡Se soportan español, inglés y portugués!
- ¡Funciona con un servidor o muchos!
- ¡Funciona son la mayoría de las plataformas!
- Se puede personalizar todo
- ¡Y más!

## ¿Por Qué Usa C++? 🤷
Otros lenguajes incluyendo Python podrían ser mejores, pero tienen algunos problemas que no necesitamos con Hajime. Además, C++ nos provee algunas características útiles y únicas.

## ¿Qué es "Hajime"? 🙋
En japonés, "Hajime" significa "comenzar". Yo lo sé porque sé el judo y se usa "hajime" mucho allí.

# Instrucciones ✅

## Binarios listos (desde [Releases](https://github.com/Slackadays/Hajime/releases)) 📦
No hay mucha duda que se soporta tu plataforma. Pero, si no se soporte, va a la página Releases.

## Configuración 🪛

Hajime iniciará el asistente de instalación si no pueda buscar ningún archivo de configuración. Cuando descargas Hajime, esto será el caso. Para iniciar el asistente a mano, usa `-i` como indicador.

## Compilar 📚

### CMake ⚙️
Se puede usar CMake para compilar Hajime. Haz estos comandos para hacerlo:
```
cmake source
cmake --build . -j 8
```
Si usas Windows, añada `--config Release` al fin para hacer un binario de Release.

### Fakemake 🤫
Se puede usar el scripto `fakemake` también, un scripto de terminal para POSIX. Usa fakemake por
`sh fakemake`.
   
# Resolución de Problemas 🆘
Usa GitHub Issues o únete nuestro servidor de Discord.

# MacStadium 🍎
Para desarrollar para macOS, MacStadium nos provee un Mac Mini. Y, ellos proveen los Mac Minis para otros proyectos de código abierto.
<img src="MacStadium-developerlogo.png" alt="Hajime logo" width="300px"/>
