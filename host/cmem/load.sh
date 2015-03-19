#!/bin/sh

insmod cmem_dev.ko

filelist=$(ls /dev | grep cmem)
for filename in $filelist
do
	chmod 666 "/dev/$filename"
done


