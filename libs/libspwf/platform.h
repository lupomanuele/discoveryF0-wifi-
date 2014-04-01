/**
 * @file platform.h
 * @author Manuele Lupo
 * @date 9 Sep 2013
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "spwf_wifi_err.h"

#define wait usleep
#define STRCPY strcpy
#define PRINT printf
#define SPRINTF sprintf

/* Adjust for your target CPU. These are ok for 32bit CPUs */
typedef unsigned int u32t;
typedef unsigned long long u64t;
typedef double f64t;
typedef char s8t;
typedef unsigned short u16_t;
typedef s8t err_t;


/* Definitions for error constants. */

#define ERR_OK          0	/* No error, everything OK. */
#define ERR_MEM        -1	/* Out of memory error.     */
#define ERR_BUF        -2	/* Buffer error.            */
#define ERR_TIMEOUT    -3	/* Timeout.                 */
#define ERR_RTE        -4	/* Routing problem.         */
#define ERR_INPROGRESS -5	/* Operation in progress    */
#define ERR_VAL        -6	/* Illegal value.           */
#define ERR_WOULDBLOCK -7	/* Operation would block.   */

#define ERR_IS_FATAL(e) ((e) < ERR_WOULDBLOCK)

#define ERR_ABRT       -8	/* Connection aborted.      */
#define ERR_RST        -9	/* Connection reset.        */
#define ERR_CLSD       -10	/* Connection closed.       */
#define ERR_CONN       -11	/* Not connected.           */

#define ERR_ARG        -12	/* Illegal argument.        */

#define ERR_USE        -13	/* Address in use.          */

#define ERR_IF         -14	/* Low-level netif error    */
#define ERR_ISCONN     -15	/* Already connected.       */
#define ERROR_UART     -16

#endif /* __PLATFORM_H__ */
