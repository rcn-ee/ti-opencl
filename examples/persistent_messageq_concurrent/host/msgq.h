#ifndef _MSGQ_H_
#define _MSGQ_H_
#include <stdio.h>
#include <stdexcept>

#include <ti/ipc/Std.h>
#define Void void               // Work around IPC usage of typedef void
#include <ti/ipc/MultiProc.h>
#include <ti/ipc/MessageQ.h>
#undef Void

class MsgQ
{
    public:
        MsgQ(const char * Qname) : Qname_((const String) Qname)
        {
            MessageQ_Params params;
            MessageQ_Params_init(&params);

            Q_ = MessageQ_create(Qname_, &params);

            if (!Q_) throw std::runtime_error("MessageQ could not be created");
        }

        ~MsgQ()
        {
            MessageQ_unblock(Q_);

            int status = MessageQ_delete(&Q_);
            if (status != MessageQ_S_SUCCESS)
                throw std::runtime_error("MessageQ could not be deleted");
        }

        int receive()
        {
            MessageQ_Msg msg;

            int status = MessageQ_get(Q_, &msg, MessageQ_FOREVER);
            //int status;
            //do status = MessageQ_get(Q_, &msg, 10); // 10 usec
            //while (status == MessageQ_E_TIMEOUT);

            if (status != MessageQ_S_SUCCESS)
                throw std::runtime_error("MessageQ could not be received");

            int msgId = MessageQ_getMsgId(msg);

            status = MessageQ_free(msg);
            if (status != MessageQ_S_SUCCESS)
                throw std::runtime_error("MessageQ could not be freed");

            return msgId;
        }

        uint32_t id()
        {
            return MessageQ_getQueueId(Q_);
        }

        uint32_t count()
        {
            return MessageQ_count(Q_);
        }

    private:
        const String Qname_;
        MessageQ_Handle Q_;
};

#endif // _MSGQ_H_
