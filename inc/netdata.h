#ifndef __NETDATA_H
#define __NETDATA_H

#include <stdio.h>

#ifndef BYTE
#   define BYTE unsigned char
#endif

#ifndef TRUE
#   define TRUE  1
#   define FALSE 0
#endif

typedef struct s_nd_time {
    char    year[4];
    char    month[2];
    char    day[2];
    char    hour[2];
    char    minute[2];
    char    second[2];
    char    fraction[1];
} NETDATA_TIME, *P_NETDATA_TIME;

typedef struct s_text_unit_header {
    unsigned short key;
    unsigned short number;
} TEXT_UNIT_HEADER, *P_TEXT_UNIT_HEADER;

typedef struct s_text_unit_data {
    short   length;
    BYTE    data[1];
} TEXT_UNIT_DATA, *P_TEXT_UNIT_DATA;

typedef struct s_text_unit {
    TEXT_UNIT_HEADER    header;
    TEXT_UNIT_DATA      data[1];
} TEXT_UNIT, *P_TEXT_UNIT;

typedef struct s_control_record_data {
    BYTE        identifier[6];
    BYTE        data[247];
} CONTROL_RECORD_DATA, *P_CONTROL_RECORD_DATA;

typedef struct s_data_record_data {
    BYTE        data[253];
} DATA_RECORD_DATA, *P_DATA_RECORD_DATA;

typedef struct s_segment {
    BYTE    length;
    BYTE    flags;
    BYTE    data[253];
} SEGMENT, *P_SEGMENT;

typedef enum e_control_rec_format {
    INMR01,
    INMR02,
    INMR03,
    INMR04,
    INMR06,
    INMR07,
    UNKNOWN
} CONTROL_RECORD_FORMAT;

/* SEGMENT DESCRIPTOR FLAGS                               */
#define SDF_FIRST_SEGMENT  0x80u /* 1000                  */
#define SDF_LAST_SEGMENT   0x40u /* 0100                  */
#define SDF_CONTROL_RECORD 0x20u /* 0010                  */
#define SDF_RECORD_NUMBER  0x10u /* 0001                  */

/* KEYS FOR NETWORK USER IDENTIFICATION (INMR01 RECORD)   */
#define INMTNODE 0x1001 /* TARGET NODE NAME               */
#define INMTUID  0x1002 /* TARGET USERID                  */
#define INMFNODE 0x1011 /* ORIGIN NODE NAME               */
#define INMFUID  0x1012 /* ORIGIN NODE NAME               */
#define INMFVERS 0x1023 /* ORIGIN VERSION NUMBER          */
#define INMFTIME 0x1024 /* ORIGIN TIME STAMP              */
#define INMTTIME 0x1025 /* DESTINATION TIME STAMP         */
#define INMNUMF  0x102F /* # OF FILES IN TRANSMISSION     */

/* KEYS FOR GENERAL CONTROL                               */
#define INMFACK  0x1026 /* ACKNOWLEDGEMENT REQUEST        */
#define INMERRCD 0x1027 /* RECEIVE ERROR CODE             */
#define INMUTILN 0x1028 /* NAME OF UTILITY PROGRAM        */
#define INMUSERP 0x1029 /* USER PARAMETER STRING          */
#define INMRECCT 0x102A /* TRANSMITTED RECORD COUNT       */

/* KEYS FOR DATASET IDENTIFICATION (INMR02,INMR03 RECORDS)*/
#define INMDDNAM 0x0001 /* DDNAME FOR FILE                */
#define INMDSNAM 0x0002 /* DATASET NAME FOR FILE          */
#define INMMEMBR 0x0003 /* TRANSMITTED MEMBER LIST        */
#define INMSECND 0x000B /* SECONDARY SPACE QUANTITY       */
#define INMDIR   0x000C /* DIRECTORY SPACE QUANTITY       */
#define INMEXPDT 0x0022 /* EXPIRATION DATE                */
#define INMTERM  0x0028 /* TERMINAL ALLOCATION            */
#define INMBLKSZ 0x0030 /* BLOCKSIZE                      */
#define INMDSORG 0x003C /* DATA SET ORGANIZATION          */
#define INMLRECL 0x0042 /* LOGICAL RECORD  LENGTH         */
#define INMRECFM 0x0049 /* RECORD FORMAT                  */
#define INMLREF  0x1020 /* LAST REFERENCE DATE            */
#define INMLCHG  0x1021 /* LAST CHANGE DATE               */
#define INMCREAT 0x1022 /* CREATION DATE                  */
#define INMSIZE  0x102C /* FILE SIZE IN BYTES             */
#define INMTYPE  0x8012 /* DATA SET TYPE                  */

static const unsigned char e2a[256] = {
        0,  1,  2,  3,156,  9,134,127,151,141,142, 11, 12, 13, 14, 15,
        16, 17, 18, 19,157,133,  8,135, 24, 25,146,143, 28, 29, 30, 31,
        128,129,130,131,132, 10, 23, 27,136,137,138,139,140,  5,  6,  7,
        144,145, 22,147,148,149,150,  4,152,153,154,155, 20, 21,158, 26,
        32,160,161,162,163,164,165,166,167,168, 91, 46, 60, 40, 43, 33,
        38,169,170,171,172,173,174,175,176,177, 93, 36, 42, 41, 59, 94,
        45, 47,178,179,180,181,182,183,184,185,124, 44, 37, 95, 62, 63,
        186,187,188,189,190,191,192,193,194, 96, 58, 35, 64, 39, 61, 34,
        195, 97, 98, 99,100,101,102,103,104,105,196,197,198,199,200,201,
        202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,208,
        209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,215,
        216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,
        123, 65, 66, 67, 68, 69, 70, 71, 72, 73,232,233,234,235,236,237,
        125, 74, 75, 76, 77, 78, 79, 80, 81, 82,238,239,240,241,242,243,
        92,159, 83, 84, 85, 86, 87, 88, 89, 90,244,245,246,247,248,249,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57,250,251,252,253,254,255
};

void parseXMI(FILE *pFile);









#endif //__NETDATA_H
