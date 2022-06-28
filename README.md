[![Build Status](https://github.com/HeliumProject/Core/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/HeliumProject/Core/actions/workflows/build.yml)

<a href="http://heliumproject.github.io/">![Helium Core](https://raw.github.com/HeliumProject/Core/master/Documentation/Helium.png)</a>

Helium Core aspires to be a fully-featured C++ platform abstraction and standard library:
* Permissively licensed (BSD-style)
* Designed to scale to desktop, console, and mobile applications
* Implement patterns like Reflection, IPC, and RPC that help build sophisticated, modern applications

# Documentation #

Introductions
* [History](Documentation/Intro-History.md)
* [Coding](Documentation/Intro-Coding.md)

Modules
* [Platform](Documentation/Module-1-Platform.md)
* [Foundation](Documentation/Module-2-Foundation.md)
* [Reflect](Documentation/Module-3-Reflect.md)
* [Persist](Documentation/Module-4-Persist.md)
* [Mongo](Documentation/Module-5-Mongo.md)

# Resources #

* Website: [http://heliumproject.github.io](http://heliumproject.github.io)
* GitHub: [http://github.com/HeliumProject/Core](http://github.com/HeliumProject/Core)
* Slack: [http://heliumproject.slack.com](http://heliumproject.slack.com) (ask @gorlak for an invite)

# Building #

Helium is built using [premake5](https://github.com/premake).  Premake interprets lua script and generates platform-specific IDE project files.

## Prerequisites ##

#### Windows ####
* [Visual Studio 2015 or greater](http://www.visualstudio.com)

#### OSX ####
[XCode](https://developer.apple.com/xcode) Command Line Tools (install from within XCode preferences):

    xcode-select --install

Prerequisites can be installed via:

    sudo Dependencies/install-packages-macos.sh

#### Linux ####
[GCC 6](https://gcc.gnu.org/gcc-6/changes.html)

Prerequisites can be installed via:

    sudo Dependencies/install-packages-linux.sh

## Compile ##

First, grab our source tree from git and ensure that you fetch all the submodules by doing:

    git submodule update --init --recursive

Next, generate the project files using premake.  An appropriate build of premake is includedin the repository.

On Windows, generate Visual Studio 201x projects (replace 2015 with your desired version):

    cd Dependencies
    ..\premake vs2015
    start Build\Dependencies.sln

In Visual Studio to go `Build` > `Batch Build` > Click `Select All` then `Build`

    cd ..
    premake vs2015
    start Build\Helium.sln

On OSX and Linux, use premake to generate makefiles (Xcode support inside premake is on hold as of late):

    cd Dependencies
    ../premake.sh gmake
    make -C Build -j8

    cd ..
    ./premake.sh gmake
    make -C Build -j8
