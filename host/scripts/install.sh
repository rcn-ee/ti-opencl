#!/bin/sh
sudo -v

#---------------------------------------------------------------------
# Set variable to the installation path of the product
#---------------------------------------------------------------------
cd ..
ocl_path=$(pwd)
cd -

#-----------------------------------------------------------------------------
# Remember that this installation was first to install cmem, thus uninstall 
# will remove it
#----------------------------------------------------------------------------*/
rm -f $ocl_path/.install_log
touch $ocl_path/.install_log
sudo chmod ugo+rw $ocl_path/.install_log

#---------------------------------------------------------------------
# Load driver if not already installed
#---------------------------------------------------------------------
if [ ! -e /dev/cmem ]
then
sudo insmod $ocl_path/cmem/cmem_dev.ko
echo "INSMOD $ocl_path/cmem/cmem_dev.ko" >> $ocl_path/.install_log
fi

#---------------------------------------------------------------------
# Set permissions
#---------------------------------------------------------------------
sudo chmod ugo+rw /dev/cmem

#---------------------------------------------------------------------
# Copy cmem driver to kernel driver directory
#---------------------------------------------------------------------
kernel_name=$(uname -r)
if [ ! -e /lib/modules/$kernel_name/kernel/drivers/cmem ]
then
sudo mkdir /lib/modules/$kernel_name/kernel/drivers/cmem
echo "MKDIR /lib/modules/$kernel_name/kernel/drivers/cmem" >> $ocl_path/.install_log
fi

if [ ! -e /lib/modules/$kernel_name/kernel/drivers/cmem/cmem_dev.ko ]
then
sudo cp $ocl_path/cmem/cmem_dev.ko /lib/modules/$kernel_name/kernel/drivers/cmem
echo "CP $ocl_path/cmem/cmem_dev.ko /lib/modules/$kernel_name/kernel/drivers/cmem" >> $ocl_path/.install_log
fi

cmem_hits=$(sudo grep -c "cmem_dev" /etc/modules)
if [ $cmem_hits -eq 0 ]
then
sudo sed -i '$ a cmem_dev' /etc/modules
echo "MODULE ADD cmem_dev to /etc/modules" >> $ocl_path/.install_log
fi

#---------------------------------------------------------------------
#---------------------------------------------------------------------
sudo depmod -a

#---------------------------------------------------------------------
# Set pcie window permissions
#---------------------------------------------------------------------
$ocl_path/bin/init_global_shared_mem

#---------------------------------------------------------------------
#copy files to udev area
#---------------------------------------------------------------------
if [ ! -e /etc/udev/rules.d/c6678_udev.sh ]
then
    echo "UDEV files copied to /etc/udev/rules.d" >> $ocl_path/.install_log
    sudo cp $ocl_path/scripts/c6678_udev.sh /etc/udev/rules.d/.
    sudo cp $ocl_path/scripts/20-c6678.rules /etc/udev/rules.d/.
    sudo touch /var/log/c6678_udev.log
    sudo chmod ugo+rw /etc/udev/rules.d/20-c6678.rules
    sudo chmod ugo+x /etc/udev/rules.d/c6678_udev.sh
    sudo chmod ugo+rw /var/log/c6678_udev.log
fi
