# DPDK 17.08 - Ubuntu 16.04 - Vagrant configuration
This project provides a Vagrant configuration for running DPDK 17.08.1 in an Ubuntu 16.04.3 LTS virtual machine. Also an additional test application is included for receiving packets using DPDK.

## Prerequisites
Tested using Vagrant 2.0.1 and VirtualBox 5.0.40 on my host operating system also running Ubuntu 16.04.

The version of Vagrant available in the Ubuntu package manager is not the most recent version. Visit https://www.vagrantup.com/downloads.html and download the appropriate file to get it.

At the time of writing, Vagrant was installed by running
```
wget https://releases.hashicorp.com/vagrant/2.0.1/vagrant_2.0.1_x86_64.deb
sudo dpkg -i vagrant_2.0.1_x86_64.deb
vagrant plugin install vagrant-triggers
```

Now to install VirtualBox run
```
sudo apt install virtualbox virtualbox-ext-pack
```

## Building the VM
Using Vagrant makes building the virtual machine very easy!
```
git clone https://github.com/lenigoor/vagrant-dpdk
cd vagrant-dpdk
vagrant up
```

This process will take a while to complete (~5 minutes). When finished, run `vagrant ssh` to enter the machine.

## VM structure
The DPDK library is installed at `/home/vagrant/dpdk-stable-17.08.1` and the folder `packet-reader` from this repository is  mapped to `/home/vagrant/packet-reader`. To compile the sample application, navigate to this folder and run `make`.

The sample application can now be executed by running
```
sudo ./capture
```

This will listen for incoming packets on the second virtual network adapter. This interface is available at the host system using the name `vboxnet0` (assuming there were no host-only adapters installed before). Packets can be injected by downloading a packet capture and using `tcpreplay` on the host system, for example.
