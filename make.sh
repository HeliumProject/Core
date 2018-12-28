#!/bin/bash

pushd Build

if [ `uname` == "Darwin" ]; then
	make -j4 config=$1
elif [ `uname` == "Linux" ]; then
	make -j4 config=$1
else
	VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property installationPath`
	VSINSTALL_BASH=`cygpath -a "$VSINSTALL"`
	SOLUTION=`cat solution.txt`
	#cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& devenv.com Dependencies.sln //build debug
	cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& msbuild $SOLUTION.sln //p:Configuration=$1 //verbosity:minimal
fi

popd