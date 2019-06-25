# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
	config.vm.box = "bento/ubuntu-18.04"

	# Install DPDK by using the given script
	config.vm.provision :shell, privileged: false, 
                        :env => {http_proxy: ENV['http_proxy'],
                                 https_proxy: ENV['https_proxy']}, 
                        :path => "install-dpdk.sh"

	# Synchronize DPDK sample application folder with VM
    config.vm.synced_folder ".", "/vagrant", type: "rsync"
	config.vm.synced_folder "packet-reader", "/home/vagrant/packet-reader", type: "rsync"

	# Create a private network, which allows host-only access to the machine using a
	# specific IP. This option is needed because DPDK takes over the NIC.
	config.vm.network "private_network", ip: "10.0.0.10"

	# Switch control of the secondary NIC to DPDK after "vagrant up" completed
	#config.trigger.after :up do
	#	run_remote "$RTE_SDK/usertools/dpdk-devbind.py --force --bind=igb_uio enp0s8"
	#end

	# VirtualBox-specific configuration
	config.vm.provider "virtualbox" do |vb|
		# Set machine name, memory and CPU limits
		vb.name = "ubuntu-16.04-dpdk-18.11"
		vb.memory = 4096
		vb.cpus = 4

		# Configure VirtualBox to enable passthrough of SSE 4.1 and SSE 4.2 instructions,
		# according to this: https://www.virtualbox.org/manual/ch09.html#sse412passthrough
		# This step is fundamental otherwise DPDK won't build. It is possible to verify in
		# the guest OS that these changes took effect by running `cat /proc/cpuinfo` and
		# checking that `sse4_1` and `sse4_2` are listed among the CPU flags.
		vb.customize ["setextradata", :id, "VBoxInternal/CPUM/SSE4.1", "1"]
		vb.customize ["setextradata", :id, "VBoxInternal/CPUM/SSE4.2", "1"]

		# Allow promiscuous mode for host-only adapter
		vb.customize ["modifyvm", :id, "--nicpromisc2", "allow-all"]
	end
end
