#!/bin/bash

# local packages
rm -rf mongodb/
curl -L -omongodb-macos-x86_64-5.0.9.tgz https://fastdl.mongodb.org/osx/mongodb-macos-x86_64-5.0.9.tgz
tar -xvf mongodb-macos-x86_64-5.0.9.tgz
rm mongodb-macos-x86_64-5.0.9.tgz
mv mongodb-macos-x86_64-5.0.9 mongodb
mkdir mongodb/data