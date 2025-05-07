#!/bin/bash
#
# sudo su root
# echo "username ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

DIR=~/autotest
PREFIX="Advanced options for Ubuntu>Ubuntu, with Linux"

function updatekernel() {
	KERNEL=$(head -n 1 $DIR/kernelversion)
	DEFAULT="$PREFIX $KERNEL"

	if [ ! $KERNEL ]; then
		exit
	fi

	if [ $(uname -r) != $KERNEL ]; then
		sudo sed -i "s/^GRUB_DEFAULT=.*/GRUB_DEFAULT=\"${DEFAULT}\"/" /etc/default/grub
		sudo update-grub
		sudo reboot
	else
		sed -i 1d $DIR/kernelversion
	fi
}

updatekernel
sleep 300

$DIR/test.sh

updatekernel
