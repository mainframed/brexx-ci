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

typedef enum e_dsorg {
    VSAM,
    PO,
    PS,
    UNKNOWN_DOSORG
} ND_DSORG;

typedef struct s_nd_date_time {
    char            year    [4];
    char            month   [2];
    char            day     [2];
    char            hour    [2];
    char            minute  [2];
    char            second  [2];
} ND_DATE_TIME, *P_ND_DATE_TIME;

/* INMR01 */
typedef struct s_nd_header_record {
    /* mandatory fields*/
    char            INMFNODE      [8];  /* Origin node name                             */
    ND_DATE_TIME    INMFTIME         ;  /* Origin timestamp                             */
    char            INMFUID       [8];  /* Origin user ID                               */
    unsigned int    INMLRECL         ;  /* Length of physical control record segments   */
    char            INMTNODE      [8];  /* Target node name                             */
    char            INMTUID       [8];  /* Target user ID                               */
    /* optional fields*/
    char            INMFACK      [64];  /* Receipt notification requested               */
    unsigned int    INMFVERS         ;  /* Origin version number                        */
    unsigned int    INMNUMF          ;  /* Number of files in this transmission         */
    char            INMUSERP    [251];  /* User parameter string                        */
} ND_HEADER_RECORD, *P_ND_HEADER_RECORD;

/* INMR02 */
typedef struct s_nd_file_util_record {
    /* mandatory fields */
    ND_DSORG        INMDSORG         ;  /* File organization                            */
    unsigned int    INMLRECL         ;  /* Logical record length                        */
    BYTE            INMRECFM      [2];  /* Record format                                */
    unsigned int    INMSIZE          ;  /* Approximate size of file in bytes            */
    char            INMUTILN      [8];  /* Utility program name                         */
    /* optional fields */
    int             INMBLKSZ         ;  /* File block size                              */
    ND_DATE_TIME    INMCREAT         ;  /* Creation date                                */
    unsigned int    INMDIR           ;  /* Number of directory blocks                   */
    char            INMDSNAM     [44];  /* File name                                    */
    ND_DATE_TIME    INMEXPDT         ;  /* Expiration date                              */
    int             INMFFM           ;  /* Filemode number                              */
    ND_DATE_TIME    INMLCHG          ;  /* Last change date                             */
    ND_DATE_TIME    INMLREF          ;  /* Last reference date                          */
    unsigned int    INMTERM          ;  /* Mail file                                    */
    char            INMUSERP    [251];  /* User parameter string                        */
    unsigned int    INMMEMBRN        ;  /* Member name number                           */
    char            INMMEMBR[1][8]   ;  /* Member name list                             */
} ND_FILE_UTIL_RECORD, *P_ND_FILE_UTIL_RECORD;

typedef struct s_nd_text_unit {
    unsigned short  key;
    unsigned short  number;
    short           length;
    BYTE            data[1];
} ND_TEXT_UNIT, *P_ND_TEXT_UNIT;

typedef struct s_nd_ctrl_record {
    BYTE            identifier[6];
    BYTE            data[247];
} ND_CTRL_RECORD, *P_ND_CTRL_RECORD;

typedef struct s_nd_data_record {
    BYTE            data[253];
} ND_DATA_RECORD, *P_ND_DATA_RECORD;

typedef struct s_nd_segment {
    BYTE            length;
    BYTE            flags;
    BYTE            data[253];
} ND_SEGMENT, *P_ND_SEGMENT;

typedef enum e_ctrl_rec_format {
    INMR01,
    INMR02,
    INMR03,
    INMR04,
    INMR06,
    INMR07,
    UNKNOWN_CTRL_REC_FORMAT
} ND_CTRL_RECORD_FORMAT;

/* SEGMENT DESCRIPTOR FLAGS                                             */
#define ND_FIRST_SEGMENT    0x80u /* 1000                               */
#define ND_LAST_SEGMENT     0x40u /* 0100                               */
#define ND_CONTROL_RECORD   0x20u /* 0010                               */
#define ND_RECORD_NUMBER    0x10u /* 0001                               */

/* KEYS FOR NETWORK USER IDENTIFICATION (INMR01 RECORD)                 */
#define ND_INMTNODE         0x1001 /* TARGET NODE NAME                  */
#define ND_INMTUID          0x1002 /* TARGET USERID                     */
#define ND_INMFNODE         0x1011 /* ORIGIN NODE NAME                  */
#define ND_INMFUID          0x1012 /* ORIGIN NODE NAME                  */
#define ND_INMFVERS         0x1023 /* ORIGIN VERSION NUMBER             */
#define ND_INMFTIME         0x1024 /* ORIGIN TIME STAMP                 */
#define ND_INMTTIME         0x1025 /* DESTINATION TIME STAMP            */
#define ND_INMNUMF          0x102F /* # OF FILES IN TRANSMISSION        */

/* KEYS FOR GENERAL CONTROL                                             */
#define ND_INMFACK          0x1026 /* ACKNOWLEDGEMENT REQUEST           */
#define ND_INMERRCD         0x1027 /* RECEIVE ERROR CODE                */
#define ND_INMUTILN         0x1028 /* NAME OF UTILITY PROGRAM           */
#define ND_INMUSERP         0x1029 /* USER PARAMETER STRING             */
#define ND_INMRECCT         0x102A /* TRANSMITTED RECORD COUNT          */

/* KEYS FOR DATASET IDENTIFICATION (INMR02,INMR03 RECORDS)              */
#define ND_INMDDNAM         0x0001 /* DDNAME FOR FILE                   */
#define ND_INMDSNAM         0x0002 /* DATASET NAME FOR FILE             */
#define ND_INMMEMBR         0x0003 /* TRANSMITTED MEMBER LIST           */
#define ND_INMSECND         0x000B /* SECONDARY SPACE QUANTITY          */
#define ND_INMDIR           0x000C /* DIRECTORY SPACE QUANTITY          */
#define ND_INMEXPDT         0x0022 /* EXPIRATION DATE                   */
#define ND_INMTERM          0x0028 /* TERMINAL ALLOCATION               */
#define ND_INMBLKSZ         0x0030 /* BLOCKSIZE                         */
#define ND_INMDSORG         0x003C /* DATA SET ORGANIZATION             */
#define ND_INMLRECL         0x0042 /* LOGICAL RECORD  LENGTH            */
#define ND_INMRECFM         0x0049 /* RECORD FORMAT                     */
#define ND_INMLREF          0x1020 /* LAST REFERENCE DATE               */
#define ND_INMLCHG          0x1021 /* LAST CHANGE DATE                  */
#define ND_INMCREAT         0x1022 /* CREATION DATE                     */
#define ND_INMSIZE          0x102C /* FILE SIZE IN BYTES                */
#define ND_INMFFM           0x102D /* FILE MODE NUMBER                  */
#define ND_INMTYPE          0x8012 /* DATA SET TYPE                     */

#define ND_DSORG_VSAM       0x0008 /* VSAM                              */
#define ND_DSORG_PO         0x0200 /* partitioned organisation          */
#define ND_DSORG_PS         0x4000 /* physical sequential               */

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

int readSegment(FILE *pFile, P_ND_SEGMENT pSegment);
int isControlRecord(P_ND_SEGMENT pSegment);
ND_CTRL_RECORD_FORMAT getControlRecordFormat(P_ND_SEGMENT pSegment);
int getHeaderRecord(P_ND_SEGMENT pSegment, P_ND_HEADER_RECORD pHeaderRecord);
int getFileUtilRecord(P_ND_SEGMENT pSegment, P_ND_FILE_UTIL_RECORD pFileUtilRecord);

#ifdef __CROSS__
    void ebcdicToAscii(unsigned char *s, unsigned int length);
#endif


#endif //__NETDATA_H
