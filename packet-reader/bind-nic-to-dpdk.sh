#!/bin/bash

interface="enp0s8"

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   echo "Usage: sudo ./bind-nic-to-dpdk.sh"
   exit 1
fi

if [[ $(ifconfig -a | grep $interface) ]]; then
	ifconfig $interface down
	$RTE_SDK/usertools/dpdk-devbind.py --bind=igb_uio $interface
	echo "Interface $interface is now assigned to DPDK"
else
	echo "Interface $interface is already assigned to DPDK"
fi