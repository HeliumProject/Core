#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

VCPKG_INSTALLED=vcpkg-installed

VCPKG_ARCHITECTURE=$1

if [ "$VCPKG_ARCHITECTURE" == "" ]; then
  VCPKG_ARCHITECTURE=`uname -m`

  if [ `uname` == "Darwin" ]; then
    if [ `sysctl -in sysctl.proc_translated` == "1" ]; then
      VCPKG_ARCHITECTURE=`arm64`
    fi 
  fi
fi

VCPKG_TRIPLETS=()
if [ `uname` == "Linux" ] && [ "$VCPKG_ARCHITECTURE" == "arm64" ]; then
  VCPKG_TRIPLETS+=( vcpkg-linux-arm64 )
elif [ `uname` == "Linux" ] && [ "$VCPKG_ARCHITECTURE" == "x86_64" ]; then
  VCPKG_TRIPLETS+=( vcpkg-linux-x86_64 )
elif [ `uname` == "Darwin" ] && [ "$VCPKG_ARCHITECTURE" == "arm64" ]; then
  VCPKG_TRIPLETS+=( vcpkg-macosx-arm64 )
elif [ `uname` == "Darwin" ] && [ "$VCPKG_ARCHITECTURE" == "x86_64" ]; then
  VCPKG_TRIPLETS+=( vcpkg-macosx-x86_64 )
fi

VCPKG_PORTS="glfw3 gtest imgui libbson mongo-c-driver rapidjson zlib"

echo
echo === Bootstrapping vcpkg ===
"$DIR/vcpkg/bootstrap-vcpkg.sh" -disableMetrics

for VCPKG_TRIPLET in ${VCPKG_TRIPLETS[@]}
do
  echo
  echo === Running vcpkg for $VCPKG_TRIPLET ===
  "$DIR/vcpkg/vcpkg" install --x-install-root="$DIR/$VCPKG_INSTALLED" --overlay-triplets=./ --triplet=$VCPKG_TRIPLET $VCPKG_PORTS
done