/* Maciej Andrejczuk */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "err.h"
#include "mesg.h"

int init_queue(int id, int flag) {
	int newid;
    if ((newid = msgget(id, flag)) == -1)
		syserr("msgget");
    return newid;
}

int close_queue(int id) {
    int ret = msgctl(id, IPC_RMID, 0);
    if (ret == -1)
        syserr("msgctl RMID");
    return ret;
}

int send_order(int id, const MesgOrder* message, int len) {
    int ret = msgsnd(id, message, len, 0);
    if (ret == -1)
        syserr("msgsnd");
    return ret;
}

int receive_order(int id, MesgOrder* message, int who) {
    int ret = msgrcv(id, message, (MAXMESGDATA + 2) * sizeof(int), who, 0);
    if (ret == -1)
        syserr("msgrcv");
    return ret;
}

int send_info(int id, const MesgInfo* message) {
    int ret = msgsnd(id, message, sizeof(int), 0);
    if (ret == -1)
        syserr("msgsnd");
    return ret;
}

int receive_info(int id, MesgInfo* message, int who) {
    int ret = msgrcv(id, message, sizeof(int), who, 0);
    if (ret == -1)
        syserr("msgrcv");
    return ret;
}

