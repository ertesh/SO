#define	SHMKEY	((key_t) 7890) /* base value for shmem key */

#define	SEMKEY1	((key_t) 7891) /* client semaphore key */
#define	SEMKEY2	((key_t) 7892) /* server semaphore key */

#define	PERMS	0666

#define	MAXMESGDATA     4000 	

#define	MESGHDRSIZE	(sizeof(Mesg) - MAXMESGDATA)
				/* length of mesg_len and mesg_type */

typedef struct {
  int	mesg_len;	/* #bytes in mesg_data, can be 0 or > 0 */
  long	mesg_type;	/* message type, must be > 0 */
  char	mesg_data[MAXMESGDATA];
} Mesg;
