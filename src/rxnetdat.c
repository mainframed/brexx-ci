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
int receive(char sDataSetNameIn[45], char sDataSetNameOut[45]);
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

int receivePO(FILE *pF_IN, char sDataSetNameTemp[45], char sDataSetNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord);
int receivePS(FILE *pF_IN, char sDataSetNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord);

int receiveRecords(FILE *pFileIn, FILE *pFileOut, ND_RECFM recfm, unsigned int uiMaxLRECL);
void writeRecord(FILE *pFileOut, BYTE *pRecord, unsigned int uiBytesToWrite, ND_RECFM recfm);

int allocateTargetDataSet(char *sDataSetName, P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord, FILE *pSourceFile);

void getDataSetName2(FILE *pF, char sDataSetName[45]);
void getDataDiscriptorName(FILE *pF, char sDataDiscriptorName[9]);

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
int receive(char sDataSetNameIn[45], char sDataSetNameOut[45])
{
    int iErr = 0;

    char                        sDataSetNameTemp[45];

    FILE                        *pF_IN;

    ND_HEADER_RECORD            headerRecord;
    P_ND_HEADER_RECORD          pHeaderRecord       = &headerRecord;

    ND_FILE_UTIL_CTRL_RECORD    fileUtilCtrlRecord;
    P_ND_FILE_UTIL_CTRL_RECORD  pFileUtilCtrlRecord = &fileUtilCtrlRecord;

    bzero(sDataSetNameTemp, 45);
    strcat(sDataSetNameTemp, "//DSN:&&TMPSEQ");

#ifdef __CROSS__
    bzero(sDataSetNameTemp, 45);
    strncpy(sDataSetNameTemp, sDataSetNameOut, 40);
    strcat(sDataSetNameTemp, ".TMP");
#endif

    // #1 try to open the input dataset
    pF_IN = FOPEN(sDataSetNameIn, "r+b");
    if (pF_IN == NULL) {
        printf("ERR> Input data set could not be opened.\n");
        iErr = -1;
    } else {
        printf("DBG> Input data set '%s' opened.\n", sDataSetNameIn);
    }
    //

    // #2 read the header record
    if (iErr == 0) {
        iErr = readHeaderRecord(pF_IN, pHeaderRecord);
    }
    //

    // #3 read the first file utility control record from the input data set
    if (iErr == 0) {
        iErr = readFileUtilCtrlRecord(pF_IN, pFileUtilCtrlRecord);
    }
    //

    // #4 handle the payload
    if (iErr == 0) {
        switch (pFileUtilCtrlRecord->INMDSORG) {
            case PO:
                iErr = receivePO(pF_IN, sDataSetNameTemp, sDataSetNameOut, pFileUtilCtrlRecord);
                break;
            case PS:
                iErr = allocateTargetDataSet(sDataSetNameOut, pFileUtilCtrlRecord, pF_IN);
                if (iErr != 0) {
                    printf("ERR> Output data set could not be allocated.\n");
                } else {
                    printf("DBG> Output data set '%s' allocated.\n", sDataSetNameOut);
                }
                //
                iErr = receivePS(pF_IN, sDataSetNameOut, pFileUtilCtrlRecord);
                break;
            default:
                break;
        }
    }
    //

    // #5 close the input data set
    iErr = FCLOSE(pF_IN);

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

int receivePS(FILE *pF_IN, char sDataSetNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord)
{
    int                         iErr            = 0;

    FILE                        *pF_OUT         = NULL;

    ND_DATA_CTRL_RECORD         dataCtrlRecord;
    P_ND_DATA_CTRL_RECORD       pDataCtrlRecord = &dataCtrlRecord;

    unsigned int                uiLRECL         = 0;

    // #1 read over the data control record
    iErr = readDataCtrlRecord(pF_IN, pDataCtrlRecord);
    //

    // #1 try to open the output data set
    pF_OUT = FOPEN(sDataSetNameOut, "wb,vmode=1");
    if (pF_OUT == NULL) {
        printf("ERR> Output data set could not be opened.\n");
        iErr = -1;
    } else {
        printf("DBG> Output data set '%s' opened.\n", sDataSetNameOut);
    }
    //

    uiLRECL = pFileUtilCtrlRecord->INMLRECL;
    // #8 receive records into output dataset
    if (iErr == 0) {
        receiveRecords(pF_IN, pF_OUT, pFileUtilCtrlRecord->INMRECFM, uiLRECL);
    }
    //

    // #9 close the output dataset
    iErr = FCLOSE(pF_OUT);
    //

    return iErr;
}

int receivePO(FILE *pF_IN, char sDataSetNameTemp[45], char sDataSetNameOut[45], P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecordPO)
{
    int                         iErr = 0;

    FILE                        *pF_TMP;
    FILE                        *pF_OUT;
    FILE                        *pF_SYSIN;

    ND_FILE_UTIL_CTRL_RECORD    fileUtilCtrlRecordPS;
    P_ND_FILE_UTIL_CTRL_RECORD  pFileUtilCtrlRecordPS = &fileUtilCtrlRecordPS;

    char                        inddn[9];
    char                        outddn[9];

    char                        sDataDefinitionNameTemp[8];
    // #1 read the next file utility control record from input data set
    iErr = readFileUtilCtrlRecord(pF_IN, pFileUtilCtrlRecordPS);
    //

    // #2 allocate the temporary data set
    if (iErr == 0) {
        iErr = allocateTargetDataSet(sDataSetNameTemp, pFileUtilCtrlRecordPS, pF_IN);
        if (iErr != 0) {
            printf("ERR> Temp data set could not be allocated.\n");
        } else {
#ifndef __CROSS__
            bzero(sDataDefinitionNameTemp, sizeof(sDataDefinitionNameTemp));
            pF_TMP = fopen(sDataSetNameTemp, "r");
            getDataSetName2(pF_TMP, sDataSetNameTemp);
            getDataDiscriptorName(pF_TMP, sDataDefinitionNameTemp);
            printf("DBG> Temporary data set '%s' allocated as %.8s.\n", sDataSetNameTemp, sDataDefinitionNameTemp);
            fclose(pF_TMP);
#else
            printf("DBG> Temporary data set '%s' allocated.\n", sDataSetNameTemp);
#endif
        }
    }
    //

    // #3 allocate the output / target  data set
    if (iErr == 0) {
        iErr = allocateTargetDataSet(sDataSetNameOut, pFileUtilCtrlRecordPO, pF_IN);
        if (iErr != 0) {
            printf("ERR> Target data set '%s' could not be allocated.\n", sDataSetNameOut);
        } else {
#ifndef __CROSS__
            char ddn[9];
            bzero(ddn, 9);
            pF_OUT = fopen(sDataSetNameOut, "r");
            getDataSetName2(pF_OUT, sDataSetNameOut);
            getDataDiscriptorName(pF_OUT, ddn);
            printf("DBG> Target data set '%s' allocated as %s.\n", sDataSetNameOut, ddn);
            fclose(pF_OUT);
#else
            printf("DBG> Target data set '%s' allocated.\n", sDataSetNameTemp);
#endif
        }
    }
    //

    // #4 extract the sequential part to a temporary data set
    if (iErr == 0) {
        iErr = receivePS(pF_IN, sDataSetNameTemp, pFileUtilCtrlRecordPS);
    }
    //

    if (iErr == 0) {

        char  dsn[44 +1];
        char c;
        RX_DYNALC_PARAMS_PTR inputParams;
        RX_DYNALC_PARAMS_PTR sysinParams;
        RX_DYNALC_PARAMS_PTR targetParams;

        printf("DBG> Sequential file extracted.\n");


        inputParams = MALLOC(sizeof(RX_DYNALC_PARAMS), "TARGET_PARMS");
        memset(inputParams, ' ', sizeof(RX_DYNALC_PARAMS));
        memcpy(inputParams->ALCFUNC, "ALLOC", 5);
        memcpy(inputParams->ALCDDN,  "BRXINDD ", 8);
        memcpy(inputParams->ALCDSN,  sDataSetNameTemp, strlen(sDataSetNameTemp));
        iErr = call_rxdynalc(inputParams);
        FREE(inputParams);

        targetParams = MALLOC(sizeof(RX_DYNALC_PARAMS), "TARGET_PARMS");
        memset(targetParams, ' ', sizeof(RX_DYNALC_PARAMS));
        memcpy(targetParams->ALCFUNC, "ALLOC", 5);
        memcpy(targetParams->ALCDDN,  "BRXOUTDD", 8);
        memcpy(targetParams->ALCDSN,  sDataSetNameOut, strlen(sDataSetNameOut));
        iErr = call_rxdynalc(targetParams);
        FREE(targetParams);


        pF_SYSIN = fopen("//DSN:&&TMPSYSIN","wt,recfm=fb,lrecl=80,blksize=80,pri=1,dirblks=0,unit=sysda");
        if (pF_SYSIN != NULL)
        {
            char temp[79];
            bzero(temp, 79);
            bzero(dsn, sizeof(dsn));
            getDataSetName2(pF_SYSIN, dsn);
            printf("DBG> SYSIN will be %s\n", dsn);

            sprintf(temp, " COPY OUTDD=BRXOUTDD,INDD=BRXINDD\n");
            printf("DBG> sending \"%s\" to SYSIN\n", temp);
            fwrite(&temp, strlen(temp),1,pF_SYSIN);
            fflush(pF_SYSIN);
            fwrite("\\*", 2, 1, pF_SYSIN);
            fflush(pF_SYSIN);
            fclose(pF_SYSIN);
            printf("FOO> SYSIN written\n");

        } else {
            printf("DBG> SYSIN could not be created.\n");
            iErr = -2;
        }

        if (iErr == 0) {
            sysinParams = MALLOC(sizeof(RX_DYNALC_PARAMS), "SYSIN_PARMS");
            memset(sysinParams, ' ', sizeof(RX_DYNALC_PARAMS));
            memcpy(sysinParams->ALCFUNC, "ALLOC", 5);
            memcpy(sysinParams->ALCDDN, "SYSIN", 5);
            memcpy(sysinParams->ALCDSN, dsn, strlen(dsn));

            iErr = call_rxdynalc(sysinParams);
            FREE(sysinParams);

        }

        pF_TMP = fopen("//DDN:BRXINDD", "rb");
        while ((c = fgetc(pF_TMP)) != EOF) {
            printf("%c", c);
        }
        printf("DBG> calling IEBCOPY\n");
        system("IEBCOPY");

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

int allocateTargetDataSet(char *sDataSetName, P_ND_FILE_UTIL_CTRL_RECORD pFileUtilCtrlRecord, FILE *pSourceFile)
{
    int iErr = 0;

    unsigned int                uiFSIZE         = 0;
    unsigned int                uiLRECL         = 0;
    unsigned int                uiBLKSIZE       = 0;
    unsigned int                uiDIR           = 0;
    unsigned int                uiPRI           = 0;
    unsigned int                uiSEC           = 0;
    char                        sRECFM[3];
    char                        sMODE[4];

    // #1 get the record format
    bzero(sRECFM, sizeof(sRECFM));
    switch (pFileUtilCtrlRecord->INMRECFM) {
        case F:
            strcpy(sRECFM, "f");
            break;
        case FB:
            strcpy(sRECFM, "fb");
            break;
        case V:
            strcpy(sRECFM, "v");
            break;
        case VB:
            strcpy(sRECFM, "vb");
            break;
        case VS:
            strcpy(sRECFM, "vs");
            break;
        case VBS:
            strcpy(sRECFM, "vbs");
            break;
        case U:
            strcpy(sRECFM, "u");
            break;
        default:
            printf("FOO> Unknown RECFM found \n");
            strcpy(sRECFM, "u");
            break;
    }
    //

    // #2 get the logical record length
    uiLRECL = pFileUtilCtrlRecord->INMLRECL;
    // TODO: check the 0x02 flag in recfm to decide wether to add 4 byte to lrecl or not
    if (pFileUtilCtrlRecord->INMRECFM == V ||
        pFileUtilCtrlRecord->INMRECFM == VB) {
        uiLRECL += 4;
    }
    //

    // #3 get the directory blocks
    uiDIR = pFileUtilCtrlRecord->INMDIR;
    //

    // #4 get the block size
    uiBLKSIZE = pFileUtilCtrlRecord->INMBLKSZ;
    //

    // #5 calculate number of tracks for the primary allocation
    uiFSIZE = pFileUtilCtrlRecord->INMSIZE;
    if (uiFSIZE <= 0) {
        // if no file size was specified, we have to use the original xmit file size
        uiFSIZE = getFileSize(pSourceFile);
    }
    uiPRI = calculateTracks(uiFSIZE, uiBLKSIZE);
    uiSEC = uiPRI;
    //

    bzero(sMODE, sizeof(sMODE));
    strcat(sMODE, "wb");

    iErr = createDataset(sDataSetName, sMODE, sRECFM, uiLRECL, uiBLKSIZE, uiDIR, uiPRI, uiSEC);

    return iErr;
}

void getDataSetName2(FILE *pF, char sDataSetName[45])
{
    __get_ddndsnmemb(fileno(pF),NULL, sDataSetName, NULL, NULL, NULL);
}

void getDataDiscriptorName(FILE *pF, char sDataDiscriptorName[9])
{
    __get_ddndsnmemb(fileno(pF),sDataDiscriptorName, NULL, NULL, NULL, NULL);
}