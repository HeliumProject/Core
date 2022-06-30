#!/bin/bash

pushd Build

if [ `uname` == "Darwin" ]; then

	JOBS=`sysctl -n hw.ncpu`
	CONFIG=`echo "$1" | tr '[:upper:]' '[:lower:]'`
	echo Building with $JOBS jobs
	make -j$JOBS config=$CONFIG

	if [ "$?" -ne "0" ]; then
		exit 1
	fi

elif [ `uname` == "Linux" ]; then

	JOBS=`nproc`
	CONFIG=`echo "$1" | tr '[:upper:]' '[:lower:]'`
	echo Building with $JOBS jobs
	make -j$JOBS config=$CONFIG

	if [ "$?" -ne "0" ]; then
		exit 1
	fi

fi

popd