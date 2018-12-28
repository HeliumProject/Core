#!/bin/bash

echo before_install...

# Suppress apt-get confirmation prompts and update
printf "APT::Get::Assume-Yes \"true\";" | sudo tee -a /etc/apt/apt.conf
sudo apt-get update
sudo DEBIAN_FRONTEND=noninteractive Dependencies/install-packages-linux.sh
sudo Dependencies/install-packages-linux-update-alternatives.sh