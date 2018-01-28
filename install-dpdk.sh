#!/bin/sh
#
# This script downloads, installs and configure the Intel DPDK framework
# on a clean Ubuntu 16.04 installation running in a virtual machine.
#
# This script has been created based on the following scripts:
#  * https://github.com/lorenzosaino/ubuntu-dpdk
#  * https://gist.github.com/ConradIrwin/9077440
#  * http://dpdk.org/doc/quick-start
#

###########################
# Variables
###########################

# Path to the DPDK directory
export RTE_SDK=${HOME}/dpdk-stable-17.08.1

# Name of network interface provisioned for DPDK to bind
export NET_IF_NAME=enp0s8

###########################
# Install DPDK
###########################

# Install dependencies
sudo apt-get -q update
sudo apt-get -q install -y build-essential linux-headers-`uname -r` libnuma-dev python

# Download DPDK version 17.08 and extract archive
wget -q https://fast.dpdk.org/rel/dpdk-17.08.1.tar.xz
tar xf dpdk-17.08.1.tar.xz
rm dpdk-17.08.1.tar.xz

# Build the DPDK library
cd dpdk-stable-17.08.1
make config T=x86_64-native-linuxapp-gcc
make

# Build sample packet capture application
make -C ${HOME}/packet-reader

###########################
# Temporary Configuration
###########################

# Load kernel modules
sudo modprobe uio
sudo insmod ${RTE_SDK}/build/kmod/igb_uio.ko

# Allocate 512 hugepages of 2 MB
# Change can be validated by executing 'cat /proc/meminfo | grep Huge'
echo 512 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages > /dev/null

###########################
# Persistent Configuration
###########################

# Add igb_uio to list of available modules
sudo ln -sf ${RTE_SDK}/build/kmod/igb_uio.ko /lib/modules/`uname -r`
sudo depmod -a

# Load modules at startup
echo "uio" | sudo tee -a /etc/modules > /dev/null
echo "igb_uio" | sudo tee -a /etc/modules > /dev/null

# Allocate 512 hugepages of 2 MB at startup
echo "vm.nr_hugepages = 512" | sudo tee -a /etc/sysctl.conf > /dev/null

# Add environment variable to system settings
echo "RTE_SDK=${RTE_SDK}" | sudo tee -a /etc/environment > /dev/null

# Binding the secondary NIC to DPDK is done by the Vagrant after "vagrant up" is executed
# Notify the user that configuration has completed
echo "Configuration completed, use 'vagrant ssh' to access the virtual machine"