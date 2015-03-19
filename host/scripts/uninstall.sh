#!/bin/sh
sudo -v

cd ..
ocl_path=$(pwd)
cd -

initial_path=$(pwd)
kernel_name=$(uname -r)

if [ ! -e $ocl_path/.install_log ]
then
   echo "Install log not found"
   exit
fi

INSMOD=$(sudo grep -c "INSMOD" $ocl_path/.install_log)
if [ $INSMOD -ne 0 ]
then
    sudo rmmod cmem_dev
fi

MKDIR=$(sudo grep -c "MKDIR /lib/modules" $ocl_path/.install_log)
if [ $MKDIR -ne 0 ]
then
    sudo rm -rf /lib/modules/$kernel_name/kernel/drivers/cmem
fi

MODULES_ADD=$(sudo grep -c "MODULE ADD" $ocl_path/.install_log)
if [ $MODULES_ADD -ne 0 ]
then
    sudo sed -i '/cmem_dev/d' /etc/modules
fi

UDEV=$(sudo grep -c "UDEV" $ocl_path/.install_log)
if [ $UDEV -ne 0 ]
then
    sudo rm /etc/udev/rules.d/20-c6678.rules
    sudo rm /etc/udev/rules.d/c6678_udev.sh
fi

cd $initial_path
rm $ocl_path/.install_log
