#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

VCPKG_INSTALLED=vcpkg-installed

VCPKG_TRIPLETS=()
if [ `uname` == "Linux" ]; then
  VCPKG_TRIPLETS+=( vcpkg-linux )
elif [ `uname` == "Darwin" ]; then
  VCPKG_TRIPLETS+=( vcpkg-macosx )
fi

VCPKG_PORTS=glfw3 gtest imgui mongo-c-driver rapidjson

echo
echo === Bootstrapping vcpkg ===
"$DIR/vcpkg/bootstrap-vcpkg.sh"

for VCPKG_TRIPLET in ${VCPKG_TRIPLETS[@]}
do
  echo
  echo === Running vcpkg for $VCPKG_TRIPLET ===
  "$DIR/vcpkg/vcpkg" install --x-install-root="$DIR/$VCPKG_INSTALLED" --overlay-triplets=./ --triplet=$VCPKG_TRIPLET $VCPKG_PORTS
done