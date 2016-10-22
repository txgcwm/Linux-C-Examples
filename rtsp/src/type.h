#ifndef __TYPE_H__
#define __TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           U8;
typedef unsigned char           UCHAR;
typedef unsigned short          U16;
typedef unsigned int            U32;

typedef signed char             S8;
typedef short                   S16;
typedef int                     S32;

#ifndef _M_IX86
typedef unsigned long long      U64;
typedef long long               S64;
#else
typedef __int64                   U64;
typedef __int64                   S64;
#endif

typedef char                    CHAR;
typedef char*                   PCHAR;

typedef float                   FLOAT;
typedef double                  DOUBLE;
typedef void                    VOID;

typedef unsigned long           UL64;
typedef long                    L64;


/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    FALSE    = 0,
    TRUE     = 1,
} BOOL;

#ifndef NULL
#define NULL             0L
#endif

//#define NULL          0L
#define NULL_PTR      0L

#define SUCCESS          0
#define FAILURE          (-1)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __TYPE_H__ */

