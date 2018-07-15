# Suppress apt-get confirmation prompts and update
printf "APT::Get::Assume-Yes \"true\";" | sudo tee -a /etc/apt/apt.conf
sudo apt-get update

# Install linux prerequisites
sudo DEBIAN_FRONTEND=noninteractive Dependencies/install-packages-linux.sh

# Update compiler
sudo Dependencies/install-packages-linux-update-alternatives.sh