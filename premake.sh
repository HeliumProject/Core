#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ ! -f $DIR/Dependencies/premake/bin/release/premake5 ]; then

	pushd $DIR/Dependencies/premake > /dev/null

	if [ `uname` == "Darwin" ]; then

		echo Calling premake bootstrap...
		make -f Bootstrap.mak osx
		if [ "$?" -ne "0" ]; then
			exit 1
		fi

	elif [ `uname` == "Linux" ]; then

		echo Calling premake bootstrap...
		make -f Bootstrap.mak linux
		if [ "$?" -ne "0" ]; then
			exit 1
		fi

	else

		VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property installationPath`
		VSVERSION=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property catalog_productLineVersion`

		if [ -z "$VSINSTALL" ]; then
			VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -products 'Microsoft.VisualStudio.Product.BuildTools' -property installationPath`
			VSVERSION=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -products 'Microsoft.VisualStudio.Product.BuildTools' -property catalog_productLineVersion`
		fi

		echo Calling premake bootstrap...
		VSINSTALL_BASH=`cygpath -a "$VSINSTALL"`
		cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& nmake -f Bootstrap.mak MSDEV=vs$VSVERSION windows-msbuild
		if [ "$?" -ne "0" ]; then
			exit 1
		fi

	fi

	popd > /dev/null

fi

$DIR/Dependencies/premake/bin/release/premake5 "$@"
