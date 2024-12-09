#!/bin/bash

SHARE=$(pwd)/make_rootfs/share
NET="-netdev user,id=eth0,smb=$SHARE -device virtio-net,netdev=eth0"

EXTRA="-m 8G -smp 6 $NET -hdb make_rootfs/out/extdisk"

DIR=$2
if [ ! $DIR ]; then
	if [ $1 = "x86_64" ]; then
		DIR=linux/build/x86_64
	elif [ $1 = "arm64" ]; then
		DIR=linux/build/arm64
	elif [ $1 = "riscv64" ]; then
		DIR=linux/build/riscv64
	fi
fi

if [ $1 = "x86_64" ]; then
	./make_rootfs/make_rootfs.sh -a x86_64
	qemu-system-x86_64 -hda make_rootfs/out/rootfs.ext4	\
		-kernel $DIR/arch/x86/boot/bzImage		\
		-append "root=/dev/sda rw console=ttyS0"	\
		-nographic $EXTRA
fi

if [ $1 = "arm64" ]; then
	./make_rootfs/make_rootfs.sh -a arm64

	if [ -f x.dtb ]; then
		DTB="-dtb x.dtb"
	fi

	qemu-system-aarch64 -M virt -cpu cortex-a57		\
		-hda make_rootfs/out/rootfs.ext4		\
		-kernel $DIR/arch/arm64/boot/Image $DTB		\
		-append "root=/dev/vda rw console=ttyAMA0"	\
		-nographic $EXTRA
fi

if [ $1 = "riscv64" ]; then
	./make_rootfs/make_rootfs.sh -a riscv64
	qemu-system-riscv64 -M virt						\
		-drive file=make_rootfs/out/rootfs.ext4,format=raw,id=hd0 -device virtio-blk-device,drive=hd0	\
		-kernel $DIR/arch/riscv/boot/Image				\
		-append "root=/dev/vda rw console=ttyS0"			\
		-nographic $EXTRA
fi
