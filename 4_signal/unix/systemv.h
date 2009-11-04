#ifndef _SYSTEMV_H_
#define _SYSTEMV_H_

/* Redefine singnal() to force System V semantics  */
/* Needed on some distributions (e.g.: Redhat 5.0) */

#define signal(a,b) __sysv_signal((a),(b))

#endif
