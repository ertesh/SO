#ifndef _MESG_
#define _MESG_

    #define	MKEY1	7272L
    #define MKEY2   7273L
    #define MKEY3   7274L
    #define MKEY4   7275L
    #define MKEY5   7276L
    #define	MAXMESGDATA 50
    #define CREATE 0666 | IPC_CREAT | IPC_EXCL

    #define ADD 1
    #define DEL 2
    #define FIND 3
    #define FINISH 4

    #define NEW_WORK 1
    #define CHANGE 2

    #define READY 1
    #define EXITING 0
    #define FINISHED -1
    typedef struct {
        long type;
        int command;
        int len;
        int data[MAXMESGDATA];
    } MesgOrder;

    typedef struct {
        long type;
        int command;
    } MesgInfo;

    int init_queue(int id, int flag);
    int close_queue(int id);

    int send_order(int id, const MesgOrder* message, int len);
    int receive_order(int id, MesgOrder* message, int who);

    int send_info(int id, const MesgInfo* message);
    int receive_info(int id, MesgInfo* message, int who);
#endif
