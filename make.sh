#!/bin/bash

pushd Build

if [ `uname` == "Darwin" ]; then

	JOBS=`sysctl -n hw.ncpu`
	echo Building with $JOBS jobs
	make -j$JOBS config=$1

	if [ "$?" -ne "0" ]; then
		exit 1
	fi

elif [ `uname` == "Linux" ]; then

	JOBS=`nproc`
	echo Building with $JOBS jobs
	make -j$JOBS config=$1

	if [ "$?" -ne "0" ]; then
		exit 1
	fi

else

	VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property installationPath`
	if [ -z "$VSINSTALL" ]; then
		VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -products 'Microsoft.VisualStudio.Product.BuildTools' -property installationPath`
	fi

	VSINSTALL_BASH=`cygpath -a "$VSINSTALL"`
	SOLUTION=`cat solution.txt`
	PLATFORM=`cat platform.txt`
	cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& msbuild $SOLUTION.sln //p:Configuration=$1 //p:Platform=$PLATFORM //verbosity:minimal

	if [ "$?" -ne "0" ]; then
		exit 1
	fi

fi

popd