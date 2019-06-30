# DPDK - Ubuntu 18.04 - Vagrant configuration
This project provides a Vagrant configuration for running DPDK 19 in an Ubuntu 16.04 LTS virtual machine. Also an additional test application is included for receiving packets using DPDK.

## Prerequisites
Tested using Vagrant 2.2.4 and VirtualBox 6.0.8.

## Building the VM
Using Vagrant makes building the virtual machine very easy!
```
git clone https://github.com/lenigoor/vagrant-dpdk
cd vagrant-dpdk
vagrant up
```

This process will take a while to complete (~5 minutes). When finished, run `vagrant ssh` to enter the machine.

## VM structure
The DPDK library is installed at `/home/vagrant/DPDK` and the folder `packet-reader` from this repository is  mapped to `/home/vagrant/packet-reader`. To compile the sample application, navigate to this folder and run `make`.

The sample application can now be executed by running
```
sudo ./capture
```

This will listen for incoming packets on the second virtual network adapter. This interface is available at the host system using the name `vboxnet0` (assuming there were no host-only adapters installed before). Packets can be injected by downloading a packet capture and using `tcpreplay` on the host system, for example.

## Building Examples 
Some examples are also provided with DPDK releases (see: https://doc.dpdk.org/guides-19.05/sample_app_ug/intro.html).

Compiling sample applications that come with DPDK is documented here: https://doc.dpdk.org/guides-19.05/sample_app_ug/compiling.html

Here's the tl;dr:
```bash
cd $RTE_SDK/
export RTE_TARGET=build
make -C examples
```
