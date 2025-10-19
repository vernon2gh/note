#!/bin/bash

SHARE=$(pwd)/make_rootfs/share
NET="-netdev user,id=eth0,smb=$SHARE -device virtio-net,netdev=eth0"
EXTRA="-m 8G -smp 8 $NET"

ARCH=$1
DIR=$2

if [ ! $ARCH ]; then
	ARCH=x86_64
fi

if [ ! $DIR ]; then
	if [ $ARCH = "x86_64" ]; then
		DIR=linux/build/x86_64
	elif [ $ARCH = "arm64" ]; then
		DIR=linux/build/arm64
	elif [ $ARCH = "riscv64" ]; then
		DIR=linux/build/riscv64
	fi
fi

if [ $ARCH = "x86_64" ]; then
	#qemu-system-x86_64 -hda make_rootfs/out/rootfs.ext4	\
	#	-kernel $DIR/arch/x86/boot/bzImage		\
	#	-append "root=/dev/sda rw console=ttyS0"	\
	#	-enable-kvm -nographic $EXTRA

	qemu-system-x86_64 -hda make_rootfs/out/ubuntu.qcow2	\
		-kernel $DIR/arch/x86/boot/bzImage		\
		-append "root=/dev/sda1 rw console=ttyS0"	\
		-enable-kvm -nographic $EXTRA

	#qemu-system-x86_64 -hda make_rootfs/out/fedora.qcow2			\
	#	-kernel $DIR/arch/x86/boot/bzImage				\
	#	-initrd make_rootfs/out/fedora-initramfs.img			\
	#	-append "root=/dev/mapper/systemVG-LVRoot rw console=ttyS0"	\
	#	-enable-kvm -nographic $EXTRA
fi

if [ $ARCH = "arm64" ]; then
	if [ -f x.dtb ]; then
		DTB="-dtb x.dtb"
	fi

	qemu-system-aarch64 -M virt -cpu cortex-a57		\
		-hda make_rootfs/out/rootfs.ext4		\
		-kernel $DIR/arch/arm64/boot/Image $DTB		\
		-append "root=/dev/vda rw console=ttyAMA0"	\
		-nographic $EXTRA
fi

if [ $ARCH = "riscv64" ]; then
	qemu-system-riscv64 -M virt						\
		-drive file=make_rootfs/out/rootfs.ext4,format=raw,id=hd0 -device virtio-blk-device,drive=hd0	\
		-kernel $DIR/arch/riscv/boot/Image				\
		-append "root=/dev/vda rw console=ttyS0"			\
		-nographic $EXTRA
fi
