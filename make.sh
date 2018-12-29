#!/bin/bash

pushd Build

if [ `uname` == "Darwin" ]; then
	JOBS=`sysctl -n hw.ncpu`
	echo Building with $JOBS jobs
	make -j$JOBS config=$1
elif [ `uname` == "Linux" ]; then
	JOBS=`nproc`
	echo Building with $JOBS jobs
	make -j$JOBS config=$1
else
	VSINSTALL=`"/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -property installationPath`
	VSINSTALL_BASH=`cygpath -a "$VSINSTALL"`
	SOLUTION=`cat solution.txt`
	cmd //c "$VSINSTALL_BASH/VC/Auxiliary/Build/vcvars64.bat" \&\& msbuild $SOLUTION.sln //p:Configuration=$1 //verbosity:minimal
fi

popd