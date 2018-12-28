#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

$DIR/make.sh debug
if [ "$?" -ne "0" ]; then
	echo "Debug failed!"
	exit 1
fi

$DIR/make.sh intermediate
if [ "$?" -ne "0" ]; then
	echo "Intermediate failed!"
	exit 1
fi

$DIR/make.sh profile
if [ "$?" -ne "0" ]; then
	echo "Profile failed!"
	exit 1
fi

$DIR/make.sh release
if [ "$?" -ne "0" ]; then
	echo "Release failed!"
	exit 1
fi
