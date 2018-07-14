[![Build Status](https://travis-ci.org/HeliumProject/Core.svg?branch=master)](https://travis-ci.org/HeliumProject/Core)

<a href="http://heliumproject.org/">![Helium Core](https://raw.github.com/HeliumProject/Core/master/Documentation/Helium.png)</a>

Helium Core aspires to be a fully-featured C++ platform abstraction and standard library:
* Permissively licensed (BSD-style)
* Designed to scale to desktop, console, and mobile applications
* Implement patterns like Reflection, IPC, and RPC that help build sophisticated, modern applications

# Documentation #

Introductions
* [History](Documentation/Intro-History.md)
* [Coding](Documentation/Intro-Coding.md)

# Resources #

* Website: [http://heliumproject.org](http://heliumproject.org)
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

On Windows, generate Visual Studio 201x projects (replace 201x with your desired version):

    cd Dependencies
    ..\premake vs2015
    start Build\Dependencies.sln
    
    cd ..
    premake vs2015
    start Build\Helium.sln

On OSX and Linux, use premake to generate makefiles (Xcode support inside premake is on hold as of late):

    cd Dependencies/Build
    ../premake.sh gmake
    make -j8
    
    cd ../Build
    ./premake.sh gmake
    make -j8
