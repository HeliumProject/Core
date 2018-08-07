#!/bin/bash

make -C Build $@ config=debug
if [ "$?" -ne "0" ]; then
	echo "Debug failed!"
	exit 1
fi

make -C Build $@ config=intermediate
if [ "$?" -ne "0" ]; then
	echo "Intermediate failed!"
	exit 1
fi

make -C Build $@ config=profile
if [ "$?" -ne "0" ]; then
	echo "Profile failed!"
	exit 1
fi

make -C Build $@ config=release
if [ "$?" -ne "0" ]; then
	echo "Release failed!"
	exit 1
fi

popd
