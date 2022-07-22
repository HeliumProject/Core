#!/bin/bash

# local packages
rm -rf mongodb/
curl -L -omongodb-linux-x86_64-ubuntu2004-5.0.9.tgz https://fastdl.mongodb.org/linux/mongodb-linux-x86_64-ubuntu2004-5.0.9.tgz
tar -xvf mongodb-linux-x86_64-ubuntu2004-5.0.9.tgz
rm mongodb-linux-x86_64-ubuntu2004-5.0.9.tgz
mv mongodb-linux-x86_64-ubuntu2004-5.0.9 mongodb
mkdir mongodb/data