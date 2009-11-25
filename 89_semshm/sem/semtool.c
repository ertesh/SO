#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "err.h"
#include "semun.h"

int semget_wrapper(key_t key) {
  int opperm, nsems, opperm_flag, flags;

  printf( "\nEnter attributed permissions in octal = ");
  scanf(  "%o", &opperm );

  printf( "\nEnter corresponding number to set desired flags:\n");
  printf( "0 --> No flags \n");
  printf( "1 --> IPC_CREAT \n");
  printf( "2 --> IPC_EXCL \n");
  printf( "3 --> IPC_CREAT and IPC_EXCL \n");
  printf( "Flags = ");
  scanf(  "%d", &flags );

  printf( "\nkey = 0x%x, opperm = 0%o, flags = 0%o\n", key, opperm, flags );

  switch ( flags ) {
  case 0:
    opperm_flag = opperm | 0;
    break;
  case 1:
    opperm_flag = opperm | IPC_CREAT;
    break;
  case 2:
    opperm_flag = opperm | IPC_EXCL;
    break;
  case 3:
    opperm_flag = opperm | IPC_EXCL | IPC_CREAT;
    break;
  }

  printf( "\nEnter the number of desired semaphores for this set (25 max) = ");
  scanf(  "%d", &nsems );
  printf( "\nNsems = %d\n", nsems );

  /*********************************************************/

  return semget( key, nsems, opperm_flag );

  /*********************************************************/
}

int semop_wrapper(int semid) {
  struct sembuf sembuf[10], *sops;
  int sem_num, i, op, flags;
  unsigned nsops;

  sops = sembuf;

  printf( "\nEnter the number of operations on this set = ");
  scanf(  "%d", &nsops );

  for ( i = 0; i < nsops; i++, sops++) {
    printf( "\nEnter semaphore number = ");
    scanf( "%d", &sem_num );
    sops->sem_num = sem_num;

    printf( "\nEnter the operation = ");
    scanf( "%d", &op );
    sops->sem_op = op;
    printf( "\nsem_num = %d, sem_op = %+d \n", sops->sem_num, sops->sem_op);

    printf( "Flags:\n");
    printf( " 0 ---> No flags \n");
    printf( " 1 ---> IPC_NOWAIT \n");
    printf( " 2 ---> SEM_UNDO \n");
    printf( " 3 ---> IPC_NOWAIT and SEM_UNDO \n");
    printf( "flag = ");
    scanf( "%d", &flags );
    switch ( flags ) {
      case 0:
        sops->sem_flg = 0;
	break;
      case 1:
	sops->sem_flg = IPC_NOWAIT;
	break;
      case 2:
	sops->sem_flg = SEM_UNDO;
	break;
      case 3:
	sops->sem_flg = SEM_UNDO | IPC_NOWAIT;
	break;
    }
    printf( "\nFlags = %o\n", sops->sem_flg );
  } /* for */

  sops = sembuf;

  /*************************************************************/

  return semop( semid, sops, nsops );

  /*************************************************************/
}

int semctl_wrapper(int semid, int cmd) {
  struct semid_ds semid_ds;
  int c, i, length, uid, gid, mode, retrn, semnum, choice;
  union semun arg;

  arg.buf = &semid_ds;
  retrn = 0;

  switch ( cmd ) {
    case 2:
      printf( "\nEnter semnum = ");
      scanf( "%d", &semnum );
      retrn = semctl(semid, semnum, GETVAL, arg);
      printf( "\nThe semval = %d\n", retrn );
      break;
    case 3:
      printf( "\nEnter semnum = ");
      scanf( "%d", &semnum );
      printf( "\nEnter the value = ");
      scanf( "%d", &arg.val );
      retrn = semctl( semid, semnum, SETVAL, arg);
      break;
    case 4:
      printf( "\nEnter semnum = ");
      scanf( "%d", &semnum );
      retrn = semctl( semid, semnum, GETPID, arg);
      printf( "\nThe sempid = %d\n", retrn );
      break;
    case 5:
      printf( "\nEnter semnum = ");
      scanf( "%d", &semnum );
      retrn = semctl( semid, semnum, GETNCNT, arg);
      printf( "\nThe semcnt = %d\n", retrn );
      break;
    case 6:
      printf( "\nEnter semnum = ");
      scanf( "%d", &semnum );
      retrn = semctl( semid, semnum, GETZCNT, arg);
      printf( "\nThe semzcnt = %d\n", retrn );
      break;
    case 7:
      if ((retrn = semctl( semid, 0, IPC_STAT, arg)) == -1)
        break;
      length = arg.buf->sem_nsems;
      arg.array = (ushort *) malloc(length * sizeof(ushort));
      retrn = semctl( semid, 0, GETALL, arg);
      printf( "\nSemvals = " );
      for ( i = 0; i < length; i++ )
	printf( "%d ", arg.array[i] );
      free(arg.array);
      break;
    case 8:
      if ((retrn = semctl(semid, 0, IPC_STAT, arg)) == -1)
        break;
      length = arg.buf->sem_nsems;
      arg.array = (ushort *) malloc(length * sizeof(ushort));
      for (i = 0; i < length; i++) {
	scanf( "%d", &c ); 
	arg.array[i] = c;
      }
      retrn = semctl( semid, 0, SETALL, arg);
      free(arg.array);
      break;
    case 9:
      retrn = semctl( semid, 0, IPC_STAT, arg);
      printf( "\nUser id = %d\n", arg.buf->sem_perm.uid );
      printf( "Group id = %d\n", arg.buf->sem_perm.gid );
      printf( "Operation permissions = 0%o\n", arg.buf->sem_perm.mode );
      printf( "Semaphore number = %ld\n", arg.buf->sem_nsems );
      printf( "Last semop time = %ld\n", arg.buf->sem_otime );
      printf( "Last change time = %ld\n", arg.buf->sem_ctime );
      break;
    case 10:
      printf( "Changing:\n" );
      printf( " sem_perm.uid  ---> 1\n");
      printf( " sem_perm.gid  ---> 2\n");
      printf( " sem_perm.mode ---> 3\n");
      printf( "Entry = ");
      scanf ( "%d", &choice );
      
      switch ( choice )	{
        case 1:
	  printf( "\nEnter user ID = ");
	  scanf( "%d", &uid );
	  arg.buf->sem_perm.uid = uid;
	  break;
	case 2:
	  printf( "\nEnter group ID = ");
	  scanf( "%d", &gid );
	  arg.buf->sem_perm.gid = gid;
	  break;
	case 3:
	  printf( "\nMode = ");
	  scanf( "%o", &mode );
	  arg.buf->sem_perm.mode = mode;
	  break;
      }

      retrn = semctl( semid, 0, IPC_SET, arg);
      break;
    case 11:
      retrn = semctl( semid, 0, IPC_RMID, arg);
      break;
    default:
      printf( "\nWrong command = %d\n", cmd );
      break;
  }

  return retrn;
}

int main ()
{
  char key_id[16];

  key_t key;
  int semid;
  int cmd;

  printf( "Semaphore sets present in the system\n" );
  system("ipcs -s");

  printf( "\nEnter desired key in hex (form: 0x0807633d, 0x0 for IPC_PRIVATE)\n      or semaphore's id in decimal: ");
  scanf(  "%15s", key_id);
  if ( sscanf(  key_id, "0x%x", &key ) == 0 ) { // key_id represents an ID
	sscanf( key_id, "%d", &semid );
  } else { // key_id is a key, otherwise
	semid = semget_wrapper( key );	
	if ( semid == -1 )
	    syserr("semget");
  }
  printf( "\nsemid = %d\n", semid );

  while (1) {
	printf( "\nSelect an activity:");
	printf( "\n  0 ---> exit program ");
	printf( "\n  1 ---> perform semaphore operation");
	printf( "\nor a command:");
	printf( "\n  2 ---> GETVAL ");
	printf( "\n  3 ---> SETVAL");
	printf( "\n  4 ---> GETPID ");
	printf( "\n  5 ---> GETNCNT ");
	printf( "\n  6 ---> GETZCNT ");
	printf( "\n  7 ---> GETALL ");
	printf( "\n  8 ---> SETALL ");
	printf( "\n  9 ---> IPC_STAT ");
	printf( "\n 10 ---> IPC_SET ");
	printf( "\n 11 ---> IPC_RMID ");
	printf( "\n cmd = ");
	scanf( "%d", &cmd );
	
	switch ( cmd ) {
	int retrn;
	case 0 :
	  return 0;
	case 1 :
	  retrn = semop_wrapper(semid);
	  if ( retrn == -1 )
	  syserr("semop");
	
	  printf( "\nSemop was successful! \n");
	  printf( "Semop return ----> %d\n", retrn);
	  break;
	default :
	  retrn = semctl_wrapper(semid, cmd);
	  if ( retrn == -1 )
	  syserr("semctl");
	
	  printf( "\n\nThe system call semctl() was successful! \n");
	  printf( "Semctl return ----> %d\n", retrn);
	  break;
	}
  }
  return 0;
}
