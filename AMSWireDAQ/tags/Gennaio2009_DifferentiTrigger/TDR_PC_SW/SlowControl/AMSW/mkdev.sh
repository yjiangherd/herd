#!/bin/sh

module="amswire"
device="amsw"

#remove old module
/sbin/rmmod ${module}

#make device file for card 0 if not existing
if ! [ -a /dev/amsw0 ]; then
	echo "make device file /dev/amsw0"
	mknod --mode=666 /dev/amsw0 c 241 0
fi

#make device file for card 1 if not existing
if ! [ -a /dev/amsw1 ]; then
	echo "make device file /dev/amsw1"
	mknod --mode=666 /dev/amsw1 c 241 1
fi

#make device file for card 2 if not existing
if ! [ -a /dev/amsw2 ]; then
	echo "make device file /dev/amsw2"
	mknod --mode=666 /dev/amsw2 c 241 2
fi

#make device file for card 3 if not existing
if ! [ -a /dev/amsw3 ]; then
	echo "make device file /dev/amsw3"
	mknod --mode=666 /dev/amsw3 c 241 3
fi


# invoke insmod 
/sbin/insmod -o ${module} ${module}.mod || exit 1

# show information
echo "Module information:"
lsmod | grep ${module}
echo "Device information:"
ls -l /dev/${device}?
