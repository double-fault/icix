#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/icix.kernel isodir/boot/icix.kernel
cp fs/icix.initrd isodir/boot/icix.initrd
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "icix" {
	multiboot /boot/icix.kernel
	module /boot/icix.initrd
}
EOF
grub-mkrescue -o icix.iso isodir
