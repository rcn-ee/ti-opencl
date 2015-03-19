/*
 *
 * Copyright (C) 2012-2014 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>

#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>

#include <linux/dmaengine.h>
#include <linux/time.h>
#include <linux/slab.h>

#include "cmemcfg.h"

#ifdef CMEM_CFG_USE_DMA_COHERENT_ALLOC
#define PCI_ALLOC
#endif
#ifdef PCI_ALLOC /* TODO: Need to remove dependance on PCI */
#include <linux/pci.h>
#endif

#include "cmem.h"

//#define DEBUG_ON
static dev_t cmem_dev_id;
static struct cdev cmem_cdev;
static struct class *cmem_class;
static int cmem_major;
static int cmem_minor;

struct device *cmem_dev;
cmem_host_buf_info_t *cmem_pers_host_buf_info;
cmem_host_buf_info_t *cmem_dyn_host_buf_info;
  

static spinlock_t l_lock;
static wait_queue_head_t l_read_wait;

#ifdef PCI_ALLOC   /* TODO : temporarily keeping it here to get DMA buffer associated with device driver */
struct pci_dev *ti667x_pci_dev[1];
#define TI667X_PCI_VENDOR_ID               0x104c   /* TI */
#define TI667X_PCI_DEVICE_ID               0xb005   /* C6678 */
#define TI667X_PCIE_DRVNAME     "ti6678_pcie_ep"

/**
* ti667x_ep_find_device() - Look-up for available TI667X Endpoint
*
* Since we could even be running on another TI667X device acting as RC, we need
* to skip it - this is done based on checking device class to be set as "PCI
* Bridge" for RC as the RC driver does this setting during enumeration.
*
* Note: This checking needs to be updated if RC driver is changed to set (or
* not to set) class differently.
*/
static int ti667x_ep_find_device(void)
{
    struct pci_dev *dev;

    ti667x_pci_dev[0] = NULL;

    dev = pci_get_device(TI667X_PCI_VENDOR_ID, TI667X_PCI_DEVICE_ID, NULL);
    if (NULL != dev)
    {

        pr_info(TI667X_PCIE_DRVNAME ": Found TI667x PCIe EP @0x%p\n", dev);
        while ((dev->class >> 8) == PCI_CLASS_BRIDGE_PCI)
        {
            pr_warning(TI667X_PCIE_DRVNAME ": skipping TI667x PCIe RC...\n");
            dev = pci_get_device(TI667X_PCI_VENDOR_ID, TI667X_PCI_DEVICE_ID, dev);
            if(NULL==dev) {
              pr_info(TI667X_PCIE_DRVNAME ": No non bridge TI PCI device found @0x%p\n", dev);
              return (-1);
            }
            continue;
        }

        ti667x_pci_dev[0] = dev;
        return 0;
    }

    pr_info(TI667X_PCIE_DRVNAME ": No TI PCI device found @0x%p\n", dev);
    return -1;
}
#else

typedef struct _reserved_mem_area_t {
   uint64_t start_addr;
   size_t size;
} reserved_mem_area_t;

static reserved_mem_area_t persistent_mem_area;
static reserved_mem_area_t dynamic_mem_area;
 
uint64_t host_buf_alloc_ptr;
uint64_t host_dyn_buf_alloc_ptr;
#endif

/**
* cmem_ioctl() - Application interface for cmem module
*
* 
*/
long cmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
  int ret = 0 ;

  switch (cmd) {
    case  CMEM_IOCTL_ALLOC_HOST_BUFFERS:
    {
      int i;

      cmem_ioctl_host_buf_info_t *host_buf_info = &(((cmem_ioctl_t *) arg)->host_buf_info);
      if(host_buf_info->type == 0) {
        /* Consistent buffer allocation */
        
#ifndef PCI_ALLOC
        /* All previous allocations removed */
        host_buf_alloc_ptr = (persistent_mem_area.start_addr);
#endif
        if(!cmem_pers_host_buf_info)
        {
          cmem_pers_host_buf_info = kmalloc(sizeof(cmem_host_buf_info_t), GFP_KERNEL);
          if(cmem_pers_host_buf_info == NULL)
          {
            dev_err(cmem_dev, "kmalloc 1 failed ");
            return(-1);
          }
          memset(cmem_pers_host_buf_info, 0, sizeof(cmem_host_buf_info_t));
          cmem_pers_host_buf_info->buf_info 
            = kmalloc((host_buf_info->num_buffers*sizeof(cmem_host_buf_entry_t)), GFP_KERNEL);
          if(cmem_pers_host_buf_info->buf_info == NULL)
          {
            dev_err(cmem_dev, "kmalloc 2 failed ");
            goto err_kmalloc2;
          }
          memset(cmem_pers_host_buf_info->buf_info, 0, 
            (host_buf_info->num_buffers*sizeof(cmem_host_buf_entry_t)));
        }
        /* If already allocated , just return the already allocated addresses */
        if(cmem_pers_host_buf_info->num_buffers != 0) 
        {
          if(host_buf_info->num_buffers > cmem_pers_host_buf_info->num_buffers) 
          {
            dev_err(cmem_dev, "Failed  number of buffer exceed from previous allocation; Previous %d, current %x\n", cmem_pers_host_buf_info->num_buffers, host_buf_info->num_buffers);
            return(-1);
          }
          for(i =0; i < host_buf_info->num_buffers; i++) {
            if( host_buf_info->buf_info[i].length >  cmem_pers_host_buf_info->buf_info[i].length)
            {
              dev_err(cmem_dev, "Failed  length mismatch with previous allocation %d\n", i);
              return(-1);
            }
            host_buf_info->buf_info[i].virtAddr = cmem_pers_host_buf_info->buf_info[i].virtAddr;
            host_buf_info->buf_info[i].dmaAddr = cmem_pers_host_buf_info->buf_info[i].dmaAddr;
            dev_info(cmem_dev, 
              "Returning previously allocated Persistent Host memory in Pcie space %d: Base Address: 0x%llx: Virtual Address 0x%p : Size 0x%x \n",
              i, host_buf_info->buf_info[i].dmaAddr,
              host_buf_info->buf_info[i].virtAddr, 
              (unsigned int)host_buf_info->buf_info[i].length);
          }
          break;
        }
        /* Do New allocation */
        for(i =0; i < host_buf_info->num_buffers; i++) {
#ifdef PCI_ALLOC 
          host_buf_info->buf_info[i].virtAddr = (uint8_t *)dma_alloc_coherent(&ti667x_pci_dev[0]->dev,  host_buf_info->buf_info[i].length, (dma_addr_t *)&host_buf_info->buf_info[i].dmaAddr, GFP_KERNEL);
#else
          host_buf_info->buf_info[i].virtAddr = 0;
          if((host_buf_alloc_ptr+host_buf_info->buf_info[i].length) > 
            (persistent_mem_area.start_addr + persistent_mem_area.size)) {
            host_buf_info->buf_info[i].dmaAddr = 0;
          } else {
            host_buf_info->buf_info[i].dmaAddr = host_buf_alloc_ptr;
          }
          host_buf_alloc_ptr += host_buf_info->buf_info[i].length;
#endif
          if(host_buf_info->buf_info[i].dmaAddr == 0 ) {
#ifdef PCI_ALLOC 
          dev_err(&ti667x_pci_dev[0]->dev, "Failed allocation of Persistent Host memory %d\n", i);
#else
          dev_err(cmem_dev, "Failed allocation of Persistent Host memory %d\n", i);
#endif
          return (-1);
          } else {
            dev_info(cmem_dev, 
              "Allocated Persistent Host memory in Pcie space %d: Base Address: 0x%llx: Virtual Address 0x%p : Size 0x%x \n",
              i, host_buf_info->buf_info[i].dmaAddr,
              host_buf_info->buf_info[i].virtAddr, 
              (unsigned int)host_buf_info->buf_info[i].length);
          }
          cmem_pers_host_buf_info->buf_info[i] = host_buf_info->buf_info[i];
        }
        
       cmem_pers_host_buf_info->num_buffers = host_buf_info->num_buffers;
      } else {
        /* Dynamic buffer allocation */
        if(cmem_dyn_host_buf_info)
        {
          cmem_host_buf_entry_t *tmp_buf_infoP;

          tmp_buf_infoP 
            = kmalloc(((cmem_dyn_host_buf_info->num_buffers+host_buf_info->num_buffers)
            *sizeof(cmem_host_buf_entry_t)), GFP_KERNEL);
          if(tmp_buf_infoP == NULL)
          {
            dev_err(cmem_dev, "kmalloc 5 failed ");
            return(-1);
          }
          memset(tmp_buf_infoP, 0, 
            ((cmem_dyn_host_buf_info->num_buffers+host_buf_info->num_buffers)
            *sizeof(cmem_host_buf_entry_t)));
          /* copy and free old buffer  list */
          memcpy(tmp_buf_infoP, cmem_dyn_host_buf_info->buf_info, 
            cmem_dyn_host_buf_info->num_buffers*sizeof(cmem_host_buf_entry_t));
          kfree(cmem_dyn_host_buf_info->buf_info);
          cmem_dyn_host_buf_info->buf_info = tmp_buf_infoP;
          
        }
        else
        {
          cmem_dyn_host_buf_info = kmalloc(sizeof(cmem_host_buf_info_t), GFP_KERNEL);
          if(cmem_dyn_host_buf_info == NULL)
          {
            dev_err(cmem_dev, "kmalloc 3 failed ");
            return(-1);
          }

          memset(cmem_dyn_host_buf_info, 0, sizeof(cmem_host_buf_info_t));
          cmem_dyn_host_buf_info->buf_info
            = kmalloc((host_buf_info->num_buffers*sizeof(cmem_host_buf_entry_t)), GFP_KERNEL);
          if(cmem_dyn_host_buf_info->buf_info == NULL)
          {
            dev_err(cmem_dev, "kmalloc 4 failed ");
            goto err_kmalloc4;
          }
           memset(cmem_dyn_host_buf_info->buf_info, 0, 
            (host_buf_info->num_buffers*sizeof(cmem_host_buf_entry_t)));
        }
        for(i =0; i < host_buf_info->num_buffers; i++) 
        {
#ifdef PCI_ALLOC 
          host_buf_info->buf_info[i].virtAddr = (uint8_t *)dma_alloc_coherent(&ti667x_pci_dev[0]->dev,  host_buf_info->buf_info[i].length, (dma_addr_t *)&host_buf_info->buf_info[i].dmaAddr, GFP_KERNEL);
#else
          host_buf_info->buf_info[i].virtAddr = 0;
          if((host_dyn_buf_alloc_ptr+host_buf_info->buf_info[i].length) > 
             (dynamic_mem_area.start_addr + dynamic_mem_area.size)) {
            host_buf_info->buf_info[i].dmaAddr = 0;
          } else {
            host_buf_info->buf_info[i].dmaAddr = host_dyn_buf_alloc_ptr;
          }

          host_dyn_buf_alloc_ptr += host_buf_info->buf_info[i].length;
          
#endif
          if(host_buf_info->buf_info[i].dmaAddr == 0 ) {
#ifdef PCI_ALLOC 
          dev_err(&ti667x_pci_dev[0]->dev, "Failed allocation of Dynamic Host memory %d\n", i);
#else
          dev_err(cmem_dev, "Failed allocation of Dynamic Host memory %d\n", i);

#endif
          return (-1);
          } else {
            dev_info(cmem_dev, 
              "Allocated Host memory in Pcie space %d: Base Address: 0x%llx: Virtual Address 0x%p : Size 0x%x \n",
              i, host_buf_info->buf_info[i].dmaAddr,
              host_buf_info->buf_info[i].virtAddr, 
              (unsigned int)host_buf_info->buf_info[i].length);
          }

          /* Keep a local copy of this in driver */
          cmem_dyn_host_buf_info->buf_info[cmem_dyn_host_buf_info->num_buffers] = host_buf_info->buf_info[i];
          dev_info(cmem_dev, " Copied buffer %d, Base address: 0x%llx: Size 0x%x \n",
             cmem_dyn_host_buf_info->num_buffers,  cmem_dyn_host_buf_info->buf_info[cmem_dyn_host_buf_info->num_buffers].dmaAddr, 
             cmem_dyn_host_buf_info->buf_info[cmem_dyn_host_buf_info->num_buffers].length);
	  cmem_dyn_host_buf_info->num_buffers++;
        }
      }
    }
    break;

    case CMEM_IOCTL_GET_HOST_BUF_INFO:
    {
      cmem_host_buf_info_t *host_buf_info
                = (cmem_host_buf_info_t *) arg;
      if(cmem_pers_host_buf_info)
      {
        int i=0;
        /* TODO: copy the buffer info */
        *(host_buf_info) = *cmem_pers_host_buf_info;
        memcpy(host_buf_info->buf_info, &cmem_pers_host_buf_info->buf_info[i],
            CMEM_MAX_BUF_PER_ALLOC*sizeof(cmem_host_buf_entry_t));
      }
    }
    break;

    case  CMEM_IOCTL_FREE_HOST_BUFFERS:
    {
      /* TODO: Currently free clears off all buffers : 
              May need to provide dynamic alloc and free later*/
      cmem_host_buf_info_t *host_buf_info = (cmem_host_buf_info_t *) arg;
      if(host_buf_info->type == 0) {
#ifdef PCI_ALLOC 
        int i;
     
        if(cmem_pers_host_buf_info) {
          for(i =0; i < cmem_pers_host_buf_info->num_buffers; i++) {
            if(cmem_pers_host_buf_info->buf_info[i].dmaAddr != 0) {
              dma_free_coherent(&ti667x_pci_dev[0]->dev, 
                cmem_pers_host_buf_info->buf_info[i].length,
                cmem_pers_host_buf_info->buf_info[i].virtAddr,
                cmem_pers_host_buf_info->buf_info[i].dmaAddr);
              dev_info(&ti667x_pci_dev[0]->dev,
                "Freed  Host memory  %d: Base Address: 0x%llx Size : 0x%x\n", 
                i, cmem_pers_host_buf_info->buf_info[i].dmaAddr, 
                cmem_pers_host_buf_info->buf_info[i].length);
            }
          }
        }
#else
        host_buf_alloc_ptr = (persistent_mem_area.start_addr);
#endif
     
        if(cmem_pers_host_buf_info) {
          if(cmem_pers_host_buf_info->buf_info)
            kfree(cmem_pers_host_buf_info->buf_info);
          kfree(cmem_pers_host_buf_info);
          cmem_pers_host_buf_info = NULL;
        }
        dev_info(cmem_dev,
          "Freed  all Persistent memory allocation \n");
       
      } else {
#ifdef PCI_ALLOC 
        int i;
        if(cmem_dyn_host_buf_info) {
          for(i =0; i < cmem_dyn_host_buf_info->num_buffers; i++) {
            if(cmem_dyn_host_buf_info->buf_info[i].dmaAddr != 0) {
              dma_free_coherent(&ti667x_pci_dev[0]->dev, 
                cmem_dyn_host_buf_info->buf_info[i].length,
                cmem_dyn_host_buf_info->buf_info[i].virtAddr,
                cmem_dyn_host_buf_info->buf_info[i].dmaAddr);
             dev_info(&ti667x_pci_dev[0]->dev,
                "Freed Host memory  %d: Base Address: 0x%llx Size : 0x%x\n", 
                i, cmem_dyn_host_buf_info->buf_info[i].dmaAddr, 
                cmem_dyn_host_buf_info->buf_info[i].length);
            }
          }   
        }    
#else
            host_dyn_buf_alloc_ptr = (dynamic_mem_area.start_addr);
#endif
        if(cmem_dyn_host_buf_info)
        {
          if(cmem_dyn_host_buf_info->buf_info)
            kfree(cmem_dyn_host_buf_info->buf_info);
          kfree(cmem_dyn_host_buf_info);
          cmem_dyn_host_buf_info = NULL;
        }
        dev_info(cmem_dev,
          "Freed  all Dynamic memory allocation \n");

      }
    }
    break;

    default:
      ret = -1;
    break;
  }
  return ret;
err_kmalloc2: 
  kfree(cmem_pers_host_buf_info);
  cmem_pers_host_buf_info = NULL;
  return(-1);
err_kmalloc4: 
  kfree(cmem_dyn_host_buf_info);
  cmem_dyn_host_buf_info = NULL;
  return(-1);


}

/**
 * ti667x_ep_pcie_mmap() - Provide userspace mapping for specified kernel memory
 * @filp: File private data - ignored
 * @vma: User virtual memory area to map to
 *
 * At present, only allows mapping BAR1 & BAR2 spaces. It is assumed that these
 * BARs are internally translated to access ti667x L2SRAM and DDR RAM
 * respectively (application can ensure this using TI667X_PCI_SET_BAR_WINDOW
 * ioctl to setup proper translation on ti667x EP).
 *
 * Note that the application has to get the physical BAR address as assigned by
 * the host code. One way to achieve this is to use ioctl
 * TI667X_PCI_GET_BAR_INFO.
 */
int cmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
  int ret = -EINVAL;
  unsigned long sz = vma->vm_end - vma->vm_start;
  unsigned long long addr = (unsigned long long)vma->vm_pgoff << PAGE_SHIFT;

  sscanf(filp->f_path.dentry->d_name.name, CMEM_MODFILE);

  dev_info(cmem_dev, "Mapping %#lx bytes from address %#llx\n",
    sz, addr);

//  vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

  ret = remap_pfn_range(vma, vma->vm_start,
          vma->vm_pgoff,
          sz, vma->vm_page_prot);

  return ret;
}


unsigned int cmem_poll(struct file *filp, poll_table *wait)
{
  return(0);
}

/**
* cmem_fops - Declares supported file access functions
*/
static const struct file_operations cmem_fops = {
    .owner          = THIS_MODULE,
    .mmap           = cmem_mmap,
    .unlocked_ioctl = cmem_ioctl,
    .poll           = cmem_poll
};
#if 0 
/**
* cmem_err_cleanup() - Error Cleanup DMA_MEm driver
*
*/

static void cmem_err_cleanup(int ti667_dev_temp_num)
{
    int minor;
    minor=0;
    {
        device_destroy(cmem_class, MKDEV(cmem_major,minor));
    }
    class_destroy(cmem_class);
    cdev_del(&cmem_cdev);
    unregister_chrdev_region(cmem_dev_id, 1);

#if PCI_ALLOC /* TODO : Remove once make driver independant of PCI */
    {
        pci_dev_put(ti667x_pci_dev[minor]);
    }
#endif
}
#endif
/**
* cmem_init() - Initialize DMA Buffers device
*
* Initialize DMA buffers device.
*/

static int __init cmem_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&cmem_dev_id, 0, 1, CMEM_DRVNAME);
    if (ret) {
        pr_err(CMEM_DRVNAME ": could not allocate the character driver");
        return -1;
    }

    cmem_major = MAJOR(cmem_dev_id);
    cmem_minor = 0;

    cmem_class = class_create(THIS_MODULE, CMEM_DRVNAME);
    if (!cmem_class) {
        unregister_chrdev_region(cmem_dev_id, 1);   
        pr_err(CMEM_DRVNAME ": Failed to add device to sys fs\n");
        goto err_class_create ;
    }
    cdev_init(&cmem_cdev, &cmem_fops);
    cmem_cdev.owner = THIS_MODULE;
    cmem_cdev.ops = &cmem_fops;

    ret = cdev_add(&cmem_cdev, MKDEV(cmem_major, cmem_minor), 1);
    if (ret) {
      pr_err(CMEM_DRVNAME ": Failed creation of node\n");
      goto err_dev_add;
    }

    pr_info(CMEM_DRVNAME ": Major %d Minor %d assigned\n",
            cmem_major, cmem_minor);


    cmem_dev = device_create(cmem_class, NULL, MKDEV(cmem_major, cmem_minor),
            NULL, CMEM_MODFILE);
    if(cmem_dev < 0) {
      pr_info(CMEM_DRVNAME ": Error creating device \n");
      goto err_dev_create;
    }
   
    dev_info(cmem_dev, "Added device to the sys file system\n");
    cmem_pers_host_buf_info = NULL;
    cmem_dyn_host_buf_info = NULL;

#ifdef PCI_ALLOC 
    if(0 != ti667x_ep_find_device()) {
      pr_err(TI667X_PCIE_DRVNAME ": Unable to find PCI device\n");
      goto pci_error_cleanup ;
    }
#else
    persistent_mem_area.size = RESERVED_CONSISTENT_MEM_SIZE;
    persistent_mem_area.start_addr = RESERVED_CONSISTENT_MEM_START_ADDR;
   
    if(persistent_mem_area.start_addr ==0)
    {
      pr_info(CMEM_DRVNAME "Request mem region failed: persistent \n");
      goto persistent_reserve_fail_cleanup;
    }
    pr_info(CMEM_DRVNAME "Memory start Addr : %llu Size: %lu \n",
      persistent_mem_area.start_addr, (unsigned long)persistent_mem_area.size);
    dynamic_mem_area.size = RESERVED_DYNAMIC_MEM_SIZE;
    
    dynamic_mem_area.start_addr = RESERVED_DYNAMIC_MEM_START_ADDR;
    if( dynamic_mem_area.start_addr== 0)
    {
      pr_info(CMEM_DRVNAME "Request mem region failed: dynamic \n");
      goto dynamic_reserve_fail_cleanup;
    }
    pr_info(CMEM_DRVNAME "Dynamic Memory start Addr : %llu Size: %lu\n",dynamic_mem_area.start_addr,
      (unsigned long)dynamic_mem_area.size);
    host_buf_alloc_ptr =  (persistent_mem_area.start_addr);
    host_dyn_buf_alloc_ptr = (dynamic_mem_area.start_addr);
#endif

    spin_lock_init(&l_lock);
    init_waitqueue_head(&l_read_wait);
    return 0 ;

#ifdef PCI_ALLOC /* TODO : Remove once make driver independant of PCI */
pci_error_cleanup:
#else
 
dynamic_reserve_fail_cleanup:
  
persistent_reserve_fail_cleanup:
#endif

err_dev_create:
    cdev_del(&cmem_cdev);

err_dev_add:
    class_destroy(cmem_class);
err_class_create:
    unregister_chrdev_region(cmem_dev_id, 1);

    return(-1);
}
module_init(cmem_init);

/**
* cmem_cleanup() - Perform cleanups before module unload
*/
static void __exit cmem_cleanup(void)
{

  if(cmem_pers_host_buf_info)
  {
#ifdef PCI_ALLOC 
  int i;
  for(i =0; i < cmem_pers_host_buf_info->num_buffers; i++) {
    if(cmem_pers_host_buf_info->buf_info[i].dmaAddr != 0) {
      dma_free_coherent(&ti667x_pci_dev[0]->dev, 
        cmem_pers_host_buf_info->buf_info[i].length,
        cmem_pers_host_buf_info->buf_info[i].virtAddr,
        cmem_pers_host_buf_info->buf_info[i].dmaAddr);
      dev_info(&ti667x_pci_dev[0]->dev,
        "Freed Host memory  %d: Base Address: 0x%x Size : 0x%x\n", 
         i, (unsigned int)cmem_pers_host_buf_info->buf_info[i].dmaAddr, 
         (unsigned int)cmem_pers_host_buf_info->buf_info[i].length);
     }
  }
#else
    host_buf_alloc_ptr = (persistent_mem_area.start_addr);
#endif
    if(cmem_pers_host_buf_info->buf_info)
      kfree(cmem_pers_host_buf_info->buf_info);
    kfree(cmem_pers_host_buf_info);
  }
  if(cmem_dyn_host_buf_info) 
  {
#ifdef PCI_ALLOC 
  int i;
  for(i =0; i < cmem_dyn_host_buf_info->num_buffers; i++) {
    if(cmem_dyn_host_buf_info->buf_info[i].dmaAddr != 0) {
      dma_free_coherent(&ti667x_pci_dev[0]->dev, 
        cmem_dyn_host_buf_info->buf_info[i].length,
        cmem_dyn_host_buf_info->buf_info[i].virtAddr,
        cmem_dyn_host_buf_info->buf_info[i].dmaAddr);
      dev_info(&ti667x_pci_dev[0]->dev,
        "Freed Host memory  %d: Base Address: 0x%x Size : 0x%x\n", 
         i, (unsigned int)cmem_dyn_host_buf_info->buf_info[i].dmaAddr, 
         (unsigned int)cmem_dyn_host_buf_info->buf_info[i].length);
     }
  }
#else
    host_dyn_buf_alloc_ptr = (dynamic_mem_area.start_addr);
#endif
    if(cmem_dyn_host_buf_info->buf_info)
      kfree(cmem_dyn_host_buf_info->buf_info);
    kfree(cmem_dyn_host_buf_info);
  }
  /* Free memory reserved */
   device_destroy(cmem_class, MKDEV(cmem_major,0));

  class_destroy(cmem_class);
  cdev_del(&cmem_cdev);
  unregister_chrdev_region(cmem_dev_id, 1);
#ifdef PCI_ALLOC  /* TODO : Remove once make driver independant of PCI */
    {
        pci_dev_put(ti667x_pci_dev[0]);
    }
  pr_info(TI667X_PCIE_DRVNAME ": Finished put device \n");
#endif
  pr_info(CMEM_DRVNAME "Module removed  \n");
}
module_exit(cmem_cleanup);
MODULE_LICENSE("Dual BSD/GPL");
