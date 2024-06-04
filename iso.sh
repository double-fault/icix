#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/icix.kernel isodir/boot/icix.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "icix" {
	multiboot /boot/icix.kernel
}
EOF
grub-mkrescue -o icix.iso isodir
