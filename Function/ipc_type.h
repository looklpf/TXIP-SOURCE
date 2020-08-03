
#ifndef __IPC_TYPE_H__
#define __IPC_TYPE_H__



typedef unsigned char           U8;
typedef char          			CHAR;
typedef unsigned short          U16;
typedef unsigned int            U32;
typedef unsigned long           ULONG;
typedef unsigned long long      U64;

typedef signed char             S8;
typedef short                   S16;
typedef int                     S32;
typedef long                    LONG;
typedef long long               S64;
typedef float                   FLOAT;
typedef double                  DOUBLE;
typedef void           			VOID;

#define PI						3.1416

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef BOOL
#define BOOL U32
#endif


#endif 
