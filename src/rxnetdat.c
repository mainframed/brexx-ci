#include <math.h>

#ifdef JCC
extern char* _style;
#else
#include "jccdummy.h"
#endif

#include "rexx.h"
#include "rxdefs.h"
#include "rxmvsext.h"
#include "lstring.h"
#include "netdata.h"
#include "util.h"

extern RX_ENVIRONMENT_CTX_PTR environment;

/* internal function prototypes */
int receive(char sFileNameIn[45], char sFileNameOut[45]);
int transmit(char sFileNameIn[45], char sFileNameOut[45]);

int checkHeaderRecord(P_ND_SEGMENT pSegment);
int checkFileUtilCtrlRecord(P_ND_SEGMENT pSegment);
int checkDataCtrlRecord(P_ND_SEGMENT pSegment);
int checkDataRecord(P_ND_SEGMENT pSegment);

int readHeaderRecord(FILE *pFile, P_ND_HEADER_RECORD pHeaderRecord);
int readFileUtilCtrlRecord(FILE *pFile, P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord);
int readDataCtrlRecord(FILE *pFile, P_ND_DATA_CTRL_RECORD pDataCtrlRecord);
int readDataRecord(FILE *pFile, P_ND_DATA_RECORD pDataRecord);

unsigned int calculateTracks(long lFileSize, unsigned int uiBlkSize);

int receivePO(FILE *pFileIn, char sFileNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord);
int receivePS(FILE *pFileIn, char sFileNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord);

int receiveRecords(FILE *pFileIn, FILE *pFileOut, ND_RECFM recfm, unsigned int uiMaxLRECL);
void writeRecord(FILE *pFileOut, BYTE *pRecord, unsigned int uiBytesToWrite, ND_RECFM recfm);

/* rexx functions */
void R_receive(int func)
{
    char sFileNameIn[44 + 1];
    char sFileNameOut[44 + 1];

    int iErr;

    char* _style_old = _style;

    if (ARGN != 2) {
        Lerror(ERR_INCORRECT_CALL,0);
    }

    get_s(1)
    get_s(2)

    LASCIIZ(*ARG1)
    LASCIIZ(*ARG2)

#ifndef __CROSS__
    Lupper(ARG1);
    Lupper(ARG2);
#endif

    bzero(sFileNameIn,  45);
    bzero(sFileNameOut, 45);

    _style = "//DSN:";

    // #1 get the correct dsn for the input file
    iErr = getDatasetName(environment, (const char*)LSTR(*ARG1), sFileNameIn);

    // #2 get the correct dsn for the output file
    if (iErr == 0) {
        iErr = getDatasetName(environment, (const char *) LSTR(*ARG2), sFileNameOut);
    }

    // #3 receive the input dataset into the output dataset
    if (iErr == 0) {
        iErr = receive(sFileNameIn, sFileNameOut);
    }

    Licpy(ARGR, iErr);

    _style = _style_old;
} /* R_receive() */

void R_transmit(int func)
{
    char sFileNameIn[44 + 1];
    char sFileNameOut[44 + 1];

    int iErr;

    char* _style_old = _style;

    if (ARGN != 2) {
        Lerror(ERR_INCORRECT_CALL,0);
    }

    get_s(1)
    get_s(2)

    LASCIIZ(*ARG1)
    LASCIIZ(*ARG2)

#ifndef __CROSS__
    Lupper(ARG1);
    Lupper(ARG2);
#endif

    bzero(sFileNameIn, 45);
    bzero(sFileNameOut, 45);

    _style = "//DSN:";

    // get the correct dsn for the input file
    iErr = getDatasetName(environment, (const char*)LSTR(*ARG1), sFileNameIn);

    // get the correct dsn for the output file
    if (iErr == 0)
        iErr = getDatasetName(environment, (const char*)LSTR(*ARG2), sFileNameOut);

    if (iErr == 0) {
        printf("FOO> function <transmit()> not yet implemented.\n");
    }

    Licpy(ARGR, iErr);

    _style = _style_old;
} /* R_transmit() */

/* register rexx functions to brexx/370 */
void RxNetDataRegFunctions()
{
    RxRegFunction("RECEIVE",	R_receive,	0);
    RxRegFunction("TRANSMIT",	R_transmit,	0);
} /* RxNetDataRegFunctions() */

/* internal functions */
int receive(char sFileNameIn[45], char sFileNameOut[45])
{
    int iErr = 0;

    FILE                        *pFileIn;

    ND_HEADER_RECORD            headerRecord;
    P_ND_HEADER_RECORD          pHeaderRecord       = &headerRecord;

    ND_FILE_UTIL_CTRL_RECORD    fileUtilCtrlRecord;
    P_ND_FILE_UTIL_CTRL_RECORD  pFileUtilCtrlRecord = &fileUtilCtrlRecord;

    // #1 try to open the input dataset
    pFileIn = FOPEN(sFileNameIn, "r+b");
    if (pFileIn == NULL) {
        printf("FOO> Input dataset could not be opened.\n");
        iErr = -1;
    }

    // #2 read the header record
    if (iErr == 0) {
        iErr = readHeaderRecord(pFileIn, pHeaderRecord);
    }

    // #3 read the file utility control record from input dataset
    if (iErr == 0) {
        iErr = readFileUtilCtrlRecord(pFileIn, pFileUtilCtrlRecord);
    }

    if (iErr == 0) {
        switch (pFileUtilCtrlRecord->INMDSORG) {
            case PO:
                iErr = receivePO(pFileIn, sFileNameOut, pFileUtilCtrlRecord);
                break;
            case PS:
                iErr = receivePS(pFileIn, sFileNameOut, pFileUtilCtrlRecord);
                break;
            default:
                break;
        }
    }

    iErr = FCLOSE(pFileIn);

    return iErr;
}

int transmit(char sFileNameIn[45], char sFileNameOut[45])
{
    int iErr = 0;

    FILE *pFile = NULL;

    // try to open the input dataset

    pFile = FOPEN(sFileNameIn, "r+b");
    if (pFile == NULL) {
        iErr = -1;
    }

    iErr = FCLOSE(pFile);

    return iErr;
}

int checkHeaderRecord(P_ND_SEGMENT pSegment)
{
    int iErr = 0;

    if (isControlRecord(pSegment)) {
        if (getControlRecordFormat(pSegment) != INMR01) {
            iErr = -2;
        }
    } else {
        iErr = -1;
    }

    return iErr;
}

int checkFileUtilCtrlRecord(P_ND_SEGMENT pSegment)
{
    int iErr = 0;

    if (isControlRecord(pSegment)) {
        if (getControlRecordFormat(pSegment) != INMR02) {
            iErr = -2;
        }
    } else {
        iErr = -1;
    }

    return iErr;
}

int checkDataCtrlRecord(P_ND_SEGMENT pSegment)
{
    int iErr = 0;

    if (isControlRecord(pSegment)) {
        if (getControlRecordFormat(pSegment) != INMR03) {
            iErr = -2;
        }
    } else {
        iErr = -1;
    }

    return iErr;
}

int checkDataRecord(P_ND_SEGMENT pSegment)
{
    int iErr = 0;

    if (!isDataRecord(pSegment)) {
        iErr = -1;
    }

    return iErr;
}

int readHeaderRecord(FILE *pFile, P_ND_HEADER_RECORD pHeaderRecord)
{
    int                         iErr;

    ND_SEGMENT                  segment;
    P_ND_SEGMENT                pSegment = &segment;

    // #1 read next segment from input dataset
    bzero(pSegment, sizeof(ND_SEGMENT));
    iErr = readSegment(pFile, pSegment);

    // #2 check if it is an header record
    if (iErr == 0) {
        iErr = checkHeaderRecord(pSegment);
    }

    // #3 get header record (INMR01)
    if (iErr == 0) {
        bzero(pHeaderRecord, sizeof(ND_HEADER_RECORD));
        iErr = getHeaderRecord(pSegment, pHeaderRecord);
    }

    return iErr;
}

int readFileUtilCtrlRecord(FILE *pFile, P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord)
{
    int                         iErr;

    ND_SEGMENT                  segment;
    P_ND_SEGMENT                pSegment = &segment;

    // #1 read next segment from input dataset
    bzero(pSegment, sizeof(ND_SEGMENT));
    iErr = readSegment(pFile, pSegment);

    // #2 check if it is an file util control  record
    if (iErr == 0) {
        iErr = checkFileUtilCtrlRecord(pSegment);
    }

    // #3 get file util control record (INMR02)
    if (iErr == 0) {
        bzero(pFileUtilCtrlRecord, sizeof(ND_FILE_UTIL_CTRL_RECORD));
        iErr = getFileUtilCtrlRecord(pSegment, pFileUtilCtrlRecord);
    }

    return iErr;
}

int readDataCtrlRecord(FILE *pFile, P_ND_DATA_CTRL_RECORD pDataCtrlRecord)
{
    int                         iErr;

    ND_SEGMENT                  segment;
    P_ND_SEGMENT                pSegment = &segment;

    // #1 read next segment from input dataset
    bzero(pSegment, sizeof(ND_SEGMENT));
    iErr = readSegment(pFile, pSegment);

    // #2 check if it is an data control record
    if (iErr == 0) {
        iErr = checkDataCtrlRecord(pSegment);
    }

    // #3 get data control record (INMR03)
    if (iErr == 0) {
        bzero(pDataCtrlRecord, sizeof(ND_DATA_CTRL_RECORD));
        iErr = getDataCtrlRecord(pSegment, pDataCtrlRecord);
    }

    return iErr;
}

int readDataRecord(FILE *pFile, P_ND_DATA_RECORD pDataRecord)
{
    int                         iErr;

    ND_SEGMENT                  segment;
    P_ND_SEGMENT                pSegment = &segment;

    unsigned int                uiCurrentPosition;
    unsigned int                uiNumberOfBytes;
    unsigned int                uiBufferSize;

    // #1 read next segment from input dataset
    bzero(pSegment, sizeof(ND_SEGMENT));
    iErr = readSegment(pFile, pSegment);

    // #2 check if it is an data control record
    if (iErr == 0) {
        iErr = checkDataRecord(pSegment);
    }

    // #3
    if (iErr == 0) {
        getDataRecord(pSegment, pDataRecord);
    }

    return iErr;
}

unsigned int calculateTracks(long lFileSize, unsigned int uiBlkSize)
{
    unsigned int uiTracks;

    const int TRACK_SIZE = 19064;

    unsigned int uiBlkTrk;
    unsigned int uiBlocks;

    uiBlkTrk = TRACK_SIZE / uiBlkSize;

    uiBlocks = ceil(((1.0 * (unsigned int)lFileSize) / uiBlkSize));
    uiTracks = ceil(((1.5 * uiBlocks) / uiBlkTrk));

    return uiTracks;
}

int receivePS(FILE *pFileIn, char sFileNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord)
{
    int iErr = 0;

    FILE                        *pFileOut       = NULL;

    ND_DATA_CTRL_RECORD         dataCtrlRecord;
    P_ND_DATA_CTRL_RECORD       pDataCtrlRecord = &dataCtrlRecord;

    unsigned int                uiFSIZE         = 0;
    unsigned int                uiLRECL         = 0;
    unsigned int                uiBLKSIZE       = 0;
    unsigned int                uiPRI           = 0;
    unsigned int                uiSEC           = 0;
    char                        sRCFM[3];

    char                        sDCBString[256];

    // #1 read over the data control record
    iErr = readDataCtrlRecord(pFileIn, pDataCtrlRecord);
    //

    if (iErr == 0) {
        // #2 get the record format
        bzero(sRCFM, 3);
        switch (pFileUtilCtrlRecord->INMRECFM) {
            case F:
                strcpy(sRCFM, "f");
                break;
            case FB:
                strcpy(sRCFM, "fb");
                break;
            case V:
                strcpy(sRCFM, "v");
                break;
            case VB:
                strcpy(sRCFM, "vb");
                break;
            case VS:
                strcpy(sRCFM, "vs");
                break;
            case VBS:
                strcpy(sRCFM, "vbs");
                break;
            case U:
                strcpy(sRCFM, "u");
                break;
            default:
                printf("FOO> Unknown RECFM found \n");
                strcpy(sRCFM, "u");
                break;
        }
        //

        // #3 get the logical record length
        uiLRECL = pFileUtilCtrlRecord->INMLRECL;
        // TODO: check the 0x02 flag in recfm to decide wether to add 4 byte to lrecl or not
        if (pFileUtilCtrlRecord->INMRECFM == V ||
            pFileUtilCtrlRecord->INMRECFM == VB) {
            uiLRECL += 4;
        }
        //

        // #4 get the block size
        uiBLKSIZE = pFileUtilCtrlRecord->INMBLKSZ;
        //

        // #5 calculate number of tracks for the primary allocation
        uiFSIZE = pFileUtilCtrlRecord->INMSIZE;
        if (uiFSIZE <= 0) {
            // if no file size was specified, we have to use the original xmit file size
            uiFSIZE = getFileSize(pFileIn);
        }
        uiPRI = calculateTracks(uiFSIZE, uiBLKSIZE);
        uiSEC = uiPRI;
        //

        // #6 build DCB string
        sprintf(sDCBString, "wb,vmode=1,recfm=%s,lrecl=%d,blksize=%d,pri=%d,sec=%d,unit=sysda,rlse", sRCFM, uiLRECL, uiBLKSIZE,
                uiPRI, uiSEC);
        //

        // #7 open / allocate the output dataset
        //_vmode = 1;
        pFileOut = fopen(sFileNameOut, sDCBString);
        if (pFileOut == NULL) {
            iErr = -2;
        }
    }

    // #8 receive records into output dataset
    if (iErr == 0) {
        //TODO: das muss elegante gemacht werden
        if (pFileUtilCtrlRecord->INMRECFM == V ||
            pFileUtilCtrlRecord->INMRECFM == VB) {
            receiveRecords(pFileIn, pFileOut, pFileUtilCtrlRecord->INMRECFM, uiLRECL - 4);
        } else {
            receiveRecords(pFileIn, pFileOut, pFileUtilCtrlRecord->INMRECFM, uiLRECL);
        }
    }
    //

    // #9 close the output dataset
    iErr = FCLOSE(pFileOut);
    //

    return iErr;
}

int receivePO(FILE *pFileIn, char sFileNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecordPO)
{
    int iErr = 0;

    FILE                        *pFileOut;

    ND_FILE_UTIL_CTRL_RECORD    fileUtilCtrlRecordPS;
    P_ND_FILE_UTIL_CTRL_RECORD  pFileUtilCtrlRecordPS = &fileUtilCtrlRecordPS;

    // #1 read the file utility control record from input dataset
    iErr = readFileUtilCtrlRecord(pFileIn, pFileUtilCtrlRecordPS);

    // #2 handle sequential part
    if (iErr == 0) {
        receivePS(pFileIn, sFileNameOut, pFileUtilCtrlRecordPS);
    }

    return iErr;
}

int receiveRecords(FILE *pFileIn, FILE *pFileOut, ND_RECFM recfm, unsigned int uiMaxLRECL)
{
    int                 iErr                = 0;

    ND_DATA_RECORD      dataRecord;
    P_ND_DATA_RECORD    pDataRecord         = &dataRecord;

    unsigned long       uiBytesToCopy       = 0;
    unsigned long       uiBytesToWrite      = 0;
    unsigned long       uiBytesLeftInRecord = 0;
    unsigned long       uiBytesLeftInData   = 0;
    unsigned long       uiDataPos           = 0;

    BYTE *pRecord = malloc(uiMaxLRECL);
    BYTE *pRecordPos;

    while (iErr == 0) {

        bzero(pRecord, uiMaxLRECL);
        pRecordPos = pRecord;
        uiBytesLeftInRecord = uiMaxLRECL;
        uiBytesToWrite      = 0;

        while (uiBytesLeftInRecord > 0 && iErr == 0) {

            if (uiBytesLeftInData == 0)
            {
                iErr = readDataRecord(pFileIn, pDataRecord);
                uiBytesLeftInData   = pDataRecord->length;
                uiDataPos           = 0;

                if (pDataRecord->first && pRecordPos > pRecord) {
                    writeRecord(pFileOut, pRecord, uiBytesToWrite, recfm);
                    bzero(pRecord, uiMaxLRECL);
                    pRecordPos = pRecord;
                    uiBytesLeftInRecord = uiMaxLRECL;
                    uiBytesToWrite      = 0;
                }
            }

            if (iErr == 0) {
                uiBytesToCopy = MIN(uiBytesLeftInRecord, uiBytesLeftInData);

                memcpy(pRecordPos, &pDataRecord->data[uiDataPos], uiBytesToCopy);

                uiBytesLeftInRecord -= uiBytesToCopy;
                pRecordPos          += uiBytesToCopy;

                uiBytesLeftInData   -= uiBytesToCopy;
                uiDataPos           += uiBytesToCopy;

                uiBytesToWrite      += uiBytesToCopy;
            }

            if (uiBytesLeftInData == 0 && pDataRecord->first && pDataRecord->last)
                break;
        }

        if (pRecordPos > pRecord) {
            writeRecord(pFileOut, pRecord, uiBytesToWrite, recfm);
        }
    }

    fflush(pFileOut);
}

void writeRecord(FILE *pFileOut, BYTE *pRecord, unsigned int uiBytesToWrite, ND_RECFM recfm)
{
    if (pFileOut != NULL) {

        // #1 write rdw if RECFM is V/VB/VBS
        if (recfm == V || recfm == VB || recfm == VS || recfm == VBS) {
            short length = (short) uiBytesToWrite;
            fwrite(&length, 2, 1, pFileOut);
        }
        //

        // #2 write record to disk
        fwrite(pRecord, uiBytesToWrite, 1, pFileOut);
        //
    }
}
