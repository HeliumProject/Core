#!/bin/bash

echo before_install...

choco install visualstudio2017community visualstudio2017-workload-nativedesktop

find "/c/Program Files (x86)/Microsoft Visual Studio/"
