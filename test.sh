#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CONFIG=$1

TESTS=$(cat << EOF
PlatformTests
FoundationTests
MathTests
ReflectTests
PersistTests
InspectTests
ApplicationTests
MongoTests
EOF
)

for line in $TESTS; do
    $DIR/Bin/$CONFIG/$line
    if [ "$?" -ne "0" ]; then
        exit 1
    fi
done

exit 0