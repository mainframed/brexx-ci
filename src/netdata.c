#include <string.h>
#include "netdata.h"
#include "util.h"

unsigned char  HEX_INMR01[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF1 };
unsigned char  HEX_INMR02[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF2 };
unsigned char  HEX_INMR03[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF3 };
unsigned char  HEX_INMR04[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF4 };
unsigned char  HEX_INMR06[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF6 };
unsigned char  HEX_INMR07[6] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF7 };

void
ebcdicToAscii (unsigned char *s)
{
    while (*s)
    {
        *s = e2a[(int) (*s)];
        s++;
    }
}

int
getBinaryValue( BYTE *ptr, int len)
{

    int				binaryValue, i;
    BYTE	        aByte;
    // TODO: only length of 2 and 4 are permitted (short/int)
#ifndef __CROSS__
    if (len == 1) {
         binaryValue = (int) *ptr;
    } else if (len == 2) {
         binaryValue = *(short *) ptr;
    } else if (len == 4) {
         binaryValue = *(int *)   ptr;
    }
#else

    binaryValue = 0;
    for (i = 0; i < len; i++) {
        aByte = *ptr;

        binaryValue = (binaryValue * 256) + (int) aByte;

        ptr++;
    }
#endif
    return binaryValue;
}





int
readSegment(FILE *pFile, P_SEGMENT pSegment)
{
    int          iErr                   = 0;

    unsigned int ulBytesRead            = 0;
    unsigned int ulCurrentPosition      = 0;
    unsigned int ulLength               = 0;

    // read length field
    ((BYTE *)pSegment)[ulCurrentPosition] = fgetc(pFile);
    ulBytesRead += 1;

    // check length value
    ulLength = ((BYTE *)pSegment)[0] & 0xFFu;
    if (ulLength <2 || ulLength > 255) {
        iErr = 1;
    }

    // read the entire segment
    if (iErr == 0) {
        while (ulBytesRead < ulLength) {
            ulCurrentPosition++;
            ((BYTE *)pSegment)[ulCurrentPosition] = fgetc(pFile);
            ulBytesRead++;
        }

        if (ulBytesRead  != ulLength) {
            iErr = 2;
        }
    }

    return iErr;
}

int
isControlRecord(P_SEGMENT pSegment)
{
    return
        ((pSegment->flags & SDF_FIRST_SEGMENT) == SDF_FIRST_SEGMENT) &&
        ((pSegment->flags & SDF_LAST_SEGMENT) == SDF_LAST_SEGMENT)   &&
        ((pSegment->flags & SDF_CONTROL_RECORD) == SDF_CONTROL_RECORD);
}

CONTROL_RECORD_FORMAT
getControlRecordFormat(P_SEGMENT pSegment)
{
    P_CONTROL_RECORD_DATA pControlRecordData = (P_CONTROL_RECORD_DATA) &(pSegment->data);

    if (memcmp(pControlRecordData->identifier, HEX_INMR01, 6) == 0) return INMR01;
    if (memcmp(pControlRecordData->identifier, HEX_INMR02, 6) == 0) return INMR02;
    if (memcmp(pControlRecordData->identifier, HEX_INMR03, 6) == 0) return INMR03;
    if (memcmp(pControlRecordData->identifier, HEX_INMR04, 6) == 0) return INMR04;
    if (memcmp(pControlRecordData->identifier, HEX_INMR06, 6) == 0) return INMR06;
    if (memcmp(pControlRecordData->identifier, HEX_INMR07, 6) == 0) return INMR07;

    return UNKNOWN;
}

int
getTextUnit(P_SEGMENT pSegment, unsigned int uiSearchKey, P_TEXT_UNIT *hTextUnit)
{
    int             iErr                    = 0;

    unsigned int    uiCurrentPosition       = 0;
    unsigned int    uiMaxPosition           = 0;
    unsigned int    uiCurrentKey            = 0;
    unsigned int    uiCurrentNumber         = 0; // number of text unit data elements e.g. length/data pairs
    unsigned int    uiCurrentLength         = 0; // length of real data in text unit data element

    BYTE            *pCurrentTextUnit       = 0;

    iErr = 1; // default for text unit not found
    uiMaxPosition = ((unsigned int)(pSegment->length & 0xFFu))  - 2 /* segment header               */
                                                                - 6 /* control record identifier    */
                                                                - 2 /* text unit header             */
                                                                - 2 /* text unit data length        */;
    while (uiCurrentPosition <= uiMaxPosition) {

        P_CONTROL_RECORD_DATA pControlRecordData = (P_CONTROL_RECORD_DATA) &(pSegment->data);

        pCurrentTextUnit  = (BYTE *)pControlRecordData->data;
        pCurrentTextUnit += uiCurrentPosition;

        *hTextUnit = ((P_TEXT_UNIT) pCurrentTextUnit);

        uiCurrentKey    = getBinaryValue((BYTE *)&(*(hTextUnit))->header.key, 2);
        uiCurrentNumber = getBinaryValue((BYTE *)&(*(hTextUnit))->header.number, 2);
        // TODO: handle uiCurrentNumber
        uiCurrentLength = getBinaryValue((BYTE *)&(*(hTextUnit))->data->length, 2);

        if (uiCurrentKey != uiSearchKey) {
            uiCurrentPosition += sizeof(TEXT_UNIT_HEADER) + 2 + (uiCurrentLength * uiCurrentNumber) ;
        } else {
            iErr = 0;
            break;
        }
    }

    return iErr;
}

void parseXMI(FILE *pFile) {

    int                     iErr            = 0;

    SEGMENT                 segment;
    P_SEGMENT               pSegment        = &segment;

    P_TEXT_UNIT             pTextUnit       = NULL;
    P_TEXT_UNIT            *hTextUnit       = &pTextUnit;

    CONTROL_RECORD_FORMAT   ctrlRecFormat   = UNKNOWN;

    // clear segment
    bzero(pSegment, sizeof(SEGMENT));

    // read first segment from file
    iErr = readSegment(pFile, pSegment);

    // first segment must be a INMR01 control record
    if (iErr == 0) {
        if (isControlRecord(pSegment)) {
            ctrlRecFormat = getControlRecordFormat(pSegment);
            if (ctrlRecFormat != INMR01) {
                iErr = 1;
            }
        } else {
            iErr = 2;
        }
    }

    printf("-----------------------------\n");
    printf("INMR01 - mandatory text units\n");
    printf("-----------------------------\n");

    /* INMFNODE */
    if (iErr == 0) {
        char node[9];
        bzero(node,9);

        iErr = getTextUnit(pSegment, INMFNODE, hTextUnit);

        if (iErr == 0) {
            memcpy(node, pTextUnit->data->data, getBinaryValue((BYTE *)&pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(node);
#endif
            printf("INMFNODE > %s\n", node);
        }
    }

    /* INMFUID */
    if (iErr == 0) {
        char uid[9];
        bzero(uid, 9);

        iErr = getTextUnit(pSegment, INMFUID, hTextUnit);

        if (iErr == 0) {
            memcpy(uid, pTextUnit->data->data, getBinaryValue((BYTE *) & pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(uid);
#endif
            printf("INMFUID  > %s\n", uid);
        }
    }

    /* INMFTIME */
    if (iErr == 0) {
        NETDATA_TIME netdataTime;
        bzero(&netdataTime, sizeof(NETDATA_TIME));

        iErr = getTextUnit(pSegment, INMFTIME, hTextUnit);

        if (iErr == 0) {

            memcpy(&netdataTime, pTextUnit->data->data, getBinaryValue((BYTE *) & pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(&netdataTime);
#endif
            printf("INMFTIME > %.2s.%.2s.%.4s %.2s:%.2s:%.2s\n",
                    netdataTime.day,
                    netdataTime.month,
                    netdataTime.year,
                    netdataTime.hour,
                    netdataTime.minute,
                    netdataTime.second);
        }
    }

    /* INMTNODE */
    if (iErr == 0) {
        char node[9];
        bzero(node,9);

        iErr = getTextUnit(pSegment, INMTNODE, hTextUnit);

        if (iErr == 0) {
            memcpy(node, pTextUnit->data->data, getBinaryValue((BYTE *)&pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(node);
#endif
            printf("INMTNODE > %s\n", node);
        }
    }

    /* INMTUID */
    if (iErr == 0) {
        char uid[9];
        bzero(uid, 9);

        iErr = getTextUnit(pSegment, INMTUID, hTextUnit);

        if (iErr == 0) {
            memcpy(uid, pTextUnit->data->data, getBinaryValue((BYTE *) & pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(uid);
#endif
            printf("INMTUID  > %s\n", uid);
        }
    }

    /* INMLRECL */
    if (iErr == 0) {
        unsigned int uiLrecl = 0;

        iErr = getTextUnit(pSegment, INMLRECL, hTextUnit);

        if (iErr == 0) {
            uiLrecl = getBinaryValue((BYTE *) &pTextUnit->data->data,
                    getBinaryValue((BYTE *) &pTextUnit->data->length, 2));
            printf("INMLRECL > %d\n", uiLrecl);
        }
    }

    printf("-----------------------------\n");
    printf("INMR01 - optional text units\n");
    printf("-----------------------------\n");

    /* INMFACK */
    if (iErr == 0) {
        unsigned int uiLrecl = 0;

        iErr = getTextUnit(pSegment, INMFACK, hTextUnit);

        if (iErr == 0) {
            printf("INMFACK  > present\n");
        } else {
            printf("INMFACK  > not present\n");
        }
    }

    iErr = 0;

    /* INMFVERS */
    if (iErr == 0) {
        unsigned int uiLrecl = 0;

        iErr = getTextUnit(pSegment, INMFVERS, hTextUnit);

        if (iErr == 0) {
            printf("INMFVERS > present\n");
        } else {
            printf("INMFVERS > not present\n");
        }
    }

    iErr = 0;

    /* INMNUMF */
    if (iErr == 0) {
        unsigned int uiLrecl = 0;

        iErr = getTextUnit(pSegment, INMNUMF, hTextUnit);

        if (iErr == 0) {
            printf("INMNUMF  > present\n");
        } else {
            printf("INMNUMF  > not present\n");
        }
    }

    iErr = 0;

    /* INMUSERP */
    if (iErr == 0) {
        unsigned int uiLrecl = 0;

        iErr = getTextUnit(pSegment, INMUSERP, hTextUnit);

        if (iErr == 0) {
            printf("INMUSERP > present\n");
        } else {
            printf("INMUSERP > not present\n");
        }
    }
}


