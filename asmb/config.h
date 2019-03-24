/** @file config.h
*
* @brief asmb configuration.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _CONFIG_H_
#define _CONFIG_H_

//acceptable length of a section name
#define MAXSECTIONNAME (32)

//acceptable length of a symbole name
#define MAXSYMBOLENAME (255)

//Output bin and addressable max memory size
#define MAXMEMORY (1<<12)

//Textual buffer length
#define BUFLEN 512

//Filename buffer length
#define MAXFNAMELEN 255


#ifndef _WIN32
#define _strdup strdup
#define strncpy_s(DST, DSI, SRC, SSI) strncpy(DST, SRC, SSI)
#define sprintf_s sprintf
#define snprintf_s snprintf
#define fopen_s(PTR, FN, FM)  *(PTR) = fopen(FN, FM)
#endif



#endif