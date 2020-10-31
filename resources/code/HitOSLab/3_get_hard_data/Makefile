AS86	=as86 -0 -a
LD86	=ld86 -0

CFLAGS	=-m32 -g -Wall -fomit-frame-pointer 

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, /dev/xxxx or empty, in which case the
# default of /dev/hd6 is used by 'build'.
#
ROOT_DEV= #FLOPPY 

BootImage: boot/bootsect boot/setup tools/build
	tools/build boot/bootsect boot/setup none $(ROOT_DEV) > Image
	sync

tools/build: tools/build.c
	gcc $(CFLAGS) \
	-o tools/build tools/build.c

boot/setup: boot/setup.s
	$(AS86) -o boot/setup.o boot/setup.s
	$(LD86) -s -o boot/setup boot/setup.o

boot/bootsect:	boot/bootsect.s
	$(AS86) -o boot/bootsect.o boot/bootsect.s
	$(LD86) -s -o boot/bootsect boot/bootsect.o

clean:
	rm -f Image boot/bootsect boot/setup
	rm -f tools/build boot/*.o

