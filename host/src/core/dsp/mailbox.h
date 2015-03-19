#ifndef _MAILBOX_H_
#define _MAILBOX_H_
#include "u_locks_pthread.h"
#include "driver.h"

extern "C"
{
    #include "mailBox.h"
}

class Mailbox
{
  public:
    int32_t init(int32_t node_writer, int32_t node_reader)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_init(node_writer, node_reader); 
        Driver::instance()->release();
        return result;
    }

    int32_t create(int32_t node_writer, int32_t node_reader, int32_t mem_space, int32_t max_size, int32_t depth)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_create(node_writer, node_reader, mem_space, max_size, depth); 
        Driver::instance()->release();
        return result;
    }

    int32_t open(int32_t node_writer, int32_t node_reader)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_open(node_writer, node_reader); 
        Driver::instance()->release();
        return result;
    }

    int32_t write (int32_t mailBox_id, uint8_t *buf, uint32_t size, uint32_t trans_id)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_write (mailBox_id, buf, size, trans_id); 
        Driver::instance()->release();
        return result;
    }

    int32_t read (int32_t mailBox_id, uint8_t *buf, uint32_t *size, uint32_t *trans_id)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_read (mailBox_id, buf, size, trans_id); 
        Driver::instance()->release();
        return result;
    }

    int32_t query (int32_t mailBox_id, uint32_t *read_cnt, uint32_t *write_cnt)
    { 
        Driver::instance()->reserve();
        int32_t result = mailBox_query (mailBox_id, read_cnt, write_cnt); 
        Driver::instance()->release();
        return result;
    }

    /*-------------------------------------------------------------------------
    * Thread safe instance function for singleton behavior
    *------------------------------------------------------------------------*/
    static Mailbox* instance () 
    {
        static Mutex Mailbox_instance_mutex;
        Mailbox* tmp = pInstance;

        __sync_synchronize();

        if (tmp == 0) 
        {
            ScopedLock lck(Mailbox_instance_mutex);

            tmp = pInstance;
            if (tmp == 0) 
            {
                tmp = new Mailbox;
                __sync_synchronize();
                pInstance = tmp;
            }
        }
        return tmp;
    }

  private:
    static Mailbox* pInstance;

    Mailbox() { }                         // ctor private
    Mailbox(const Mailbox&);              // copy ctor disallowed
    Mailbox& operator=(const Mailbox&);   // assignment disallowed
};

#endif // _MAILBOX_H_
