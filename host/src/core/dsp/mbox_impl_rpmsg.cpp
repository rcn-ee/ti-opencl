/******************************************************************************
 * Copyright (c) 2015, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "u_locks_pthread.h"
#include "driver.h"
#include "../platform.h"
#include "device.h"
#include "message.h"

#include "mbox_impl_rpmsg.h"

extern "C"
{
#if defined(KERNEL_INSTALL_DIR)

#ifdef linux
#define _linux_ linux
#undef linux
#endif

#define linux_include(kd,m) <kd/include/uapi/linux/m.h>

#include linux_include(KERNEL_INSTALL_DIR,rpmsg_rpc)

#ifdef _linux_
#define linux _linux
#undef _linux_
#endif

#else
#error KERNEL_INSTALL_DIR must be defined!
#endif

}

using namespace Coal;

MBoxRPMsg::MBoxRPMsg(Coal::DSPDevice *device)
    : p_fd(0), p_device(device)
{
    char    dev_node[128];

    sprintf(p_rpmsg_dev,"ocl-dsp%d",p_device->dspID());
    sprintf(dev_node, "/dev/%s", p_rpmsg_dev);

    p_fd = open(dev_node, O_RDWR);

    assert (p_fd > 0);


    struct  rppc_create_instance connect;

    strncpy(connect.name, p_rpmsg_dev, (RPPC_MAX_INST_NAMELEN - 1));
    connect.name[RPPC_MAX_INST_NAMELEN - 1] = '\0';

    int result = ioctl(p_fd, RPPC_IOC_CREATE, &connect);
    assert (result >= 0);
}

int32_t MBoxRPMsg::write (uint8_t *buf, uint32_t size, uint32_t trans_id)
{ 
    int    status, result;
    struct rppc_function rpfxn;
    DSPDevicePtr64 msg_addr;
    uint8_t *Msg;

    msg_addr = p_device->malloc_global( size, true);
    Msg = (uint8_t *)Driver::instance()->map(p_device, msg_addr, size, false);

    memcpy(Msg, buf, size);

    ((Msg_t *)Msg)->trans_id = trans_id;

    rpfxn.fxn_id = 0; 
    rpfxn.num_params = 1;
    rpfxn.num_translations = 0;
    rpfxn.params[0].type = RPPC_PARAM_TYPE_ATOMIC;
    rpfxn.params[0].size = sizeof(uint32_t);
    rpfxn.params[0].data = (uint32_t)msg_addr;
    rpfxn.params[0].base = 0;
    rpfxn.params[0].fd   = 0;

    Driver::instance()->unmap(p_device, Msg, msg_addr, size, true);

    result = ::write(p_fd, &rpfxn, sizeof(struct rppc_function));
    if ( result < 0 )
    {
        perror("MBoxRPMsg::write()");
        printf("MBoxRPMsg::write(%d) : write() failed!\n", p_fd);
        return false;
    }
    else
    {
        //printf("MBoxRPMsg::write() = %d\n", result);
    }

    return true;
}

int32_t MBoxRPMsg::read (uint8_t *buf, uint32_t *size, uint32_t *trans_id)
{ 
    int32_t                      result;
    struct rppc_function_return  reply_msg;
    DSPDevicePtr64               msg_addr;
    void                        *msg;

    result = ::read(p_fd, &reply_msg, sizeof(struct rppc_function_return));
    if ( result < 0 )
    {
        perror("MBoxRPMsg::read()");
        printf("MBoxRPMsg::read : read() failed!\n");
    }
    else if (result != sizeof(struct rppc_function_return))
    {
        printf("MBoxRPMsg::read() : read returned %d bytes, expected %d bytes!\n", result, sizeof(struct rppc_function_return));
        result = -1;
    }
    else
    {
      //printf("MBoxRPMsg::read() - Found message at 0x%08X\n", reply_msg.status);
      msg_addr = reply_msg.status;
      *size    = sizeof(Msg_t);
      msg      = Driver::instance()->map(p_device, msg_addr, *size, true);
      memcpy( buf, msg, *size);
      *trans_id = ((Msg_t *)buf)->trans_id;
      Driver::instance()->unmap(p_device, msg, msg_addr, *size, false);
      p_device->free_global(msg_addr);
      result = 0;
      //printf("MBoxRPMsg::read() - Command = %d\n", ((Msg_t *)buf)->command);
    }
    //printf("MBoxRPMsg::read() = %d\n", result);

    return result;
}


MBoxRPMsg::~MBoxRPMsg(void)
{
    close(p_fd);
    p_rpmsg_dev[0] = '\0';
}


