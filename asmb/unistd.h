/** @file unistd.h
*
* @brief Compatibility typedefs
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _UNISTD_H
#define _UNISTD_H    1

/* This file intended to serve as a drop-in replacement for 
 *  unistd.h on Windows
 *  Please add functionality as neeeded 
 */
#if defined(_WIN32) || defined(WIN32)
//#include <stdlib.h>
#include <io.h>
//#include <getopt.h>
#include <process.h> /* for getpid() and the exec..() family */

#define srandom srand
#define random rand

const int W_OK = 2;
const int R_OK = 4;

#define access _access
#define ftruncate _chsize

#define ssize_t int

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
/* should be in some equivalent to <sys/types.h> */
typedef __int8            int8_t;
typedef __int16           int16_t; 
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#endif /* _WIN32 */
#endif /* unistd.h  */