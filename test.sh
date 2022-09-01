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
    # capitalize first character of CONFIG
    $DIR/Bin/"$(tr a-z A-Z <<< ${CONFIG:0:1})${CONFIG:1}"/$line
    if [ "$?" -ne "0" ]; then
        exit 1
    fi
done

exit 0