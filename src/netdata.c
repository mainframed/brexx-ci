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
ebcdicToAscii (unsigned char *s, unsigned int length)
{
    unsigned int uiCurrentPosition = 0;
    while (uiCurrentPosition < length)
    {
        s[uiCurrentPosition] = e2a[(int) (s[uiCurrentPosition])];
        uiCurrentPosition++;
    }
}

int
getBinaryValue( BYTE *ptr, int len)
{

    int				binaryValue, i;
    BYTE	        aByte;
    // TODO: only length of 1, 2 and 4 are permitted (short/int)
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
readSegment(FILE *pFile, P_ND_SEGMENT pSegment)
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
isControlRecord(P_ND_SEGMENT pSegment)
{
    return
            ((pSegment->flags & ND_FIRST_SEGMENT) == ND_FIRST_SEGMENT) &&
            ((pSegment->flags & ND_LAST_SEGMENT) == ND_LAST_SEGMENT) &&
            ((pSegment->flags & ND_CONTROL_RECORD) == ND_CONTROL_RECORD);
}

ND_CTRL_RECORD_FORMAT
getControlRecordFormat(P_ND_SEGMENT pSegment)
{
    P_ND_CTRL_RECORD pControlRecordData = (P_ND_CTRL_RECORD) &(pSegment->data);

    if (memcmp(pControlRecordData->identifier, HEX_INMR01, 6) == 0) return INMR01;
    if (memcmp(pControlRecordData->identifier, HEX_INMR02, 6) == 0) return INMR02;
    if (memcmp(pControlRecordData->identifier, HEX_INMR03, 6) == 0) return INMR03;
    if (memcmp(pControlRecordData->identifier, HEX_INMR04, 6) == 0) return INMR04;
    if (memcmp(pControlRecordData->identifier, HEX_INMR06, 6) == 0) return INMR06;
    if (memcmp(pControlRecordData->identifier, HEX_INMR07, 6) == 0) return INMR07;

    return UNKNOWN;
}

int
getTextUnit(P_ND_SEGMENT pSegment, unsigned int uiSearchKey, P_ND_TEXT_UNIT *hTextUnit)
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

        P_ND_CTRL_RECORD pControlRecordData = (P_ND_CTRL_RECORD) &(pSegment->data);

        pCurrentTextUnit  = (BYTE *)pControlRecordData->data;
        pCurrentTextUnit += uiCurrentPosition;

        *hTextUnit = ((P_ND_TEXT_UNIT) pCurrentTextUnit);

        uiCurrentKey    = getBinaryValue((BYTE *)&(*(hTextUnit))->key, 2);
        uiCurrentNumber = getBinaryValue((BYTE *)&(*(hTextUnit))->number, 2);
        // TODO: handle uiCurrentNumber
        uiCurrentLength = getBinaryValue((BYTE *)&(*(hTextUnit))->length, 2);

        if (uiCurrentKey != uiSearchKey) {
            uiCurrentPosition += 4 + 2 + (uiCurrentLength * uiCurrentNumber) ;
        } else {
            iErr = 0;
            break;
        }
    }

    return iErr;
}


int
getTextUnitLength(const struct s_nd_text_unit *pTextUnit)
{
    return getBinaryValue((BYTE *) &pTextUnit->length, 2);
}

int
getHeaderRecord(P_ND_SEGMENT pSegment, P_ND_HEADER_RECORD pHeaderRecord)
{
    int                     iErr            = 0;

    P_ND_TEXT_UNIT      pTextUnit           = NULL;
    P_ND_TEXT_UNIT      *hTextUnit          = &pTextUnit;

    unsigned int        uiCurrentPosition   = 0;
    unsigned int        uiTextUnitLength    = 0;

    /* INMFNODE */
    bzero(pHeaderRecord->INMFNODE, sizeof(pHeaderRecord->INMFNODE));
    iErr = getTextUnit(pSegment, ND_INMFNODE, hTextUnit);
    if (iErr == 0) {
        memcpy(pHeaderRecord->INMFNODE, pTextUnit->data, getTextUnitLength(pTextUnit));
    }

    /* INMFUID */
    bzero(pHeaderRecord->INMFUID, sizeof(pHeaderRecord->INMFUID));
    iErr = getTextUnit(pSegment, ND_INMFUID, hTextUnit);
    if (iErr == 0) {
        memcpy(pHeaderRecord->INMFUID, pTextUnit->data, getTextUnitLength(pTextUnit));
    }

    /* INMFTIME */
    bzero(&pHeaderRecord->INMFTIME, sizeof(pHeaderRecord->INMFTIME));
    iErr = getTextUnit(pSegment, ND_INMFTIME, hTextUnit);
    if (iErr == 0) {
        memcpy(&pHeaderRecord->INMFTIME, pTextUnit->data, getTextUnitLength(pTextUnit));
    }

    /* INMLRECL */
    bzero(&pHeaderRecord->INMLRECL, sizeof(pHeaderRecord->INMLRECL));
    iErr = getTextUnit(pSegment, ND_INMLRECL, hTextUnit);
    if (iErr == 0) {
        memcpy(&pHeaderRecord->INMLRECL, pTextUnit->data, getTextUnitLength(pTextUnit));
    }

    return iErr;
}

void parseXMI(FILE *pFile) {

    int                     iErr            = 0;

    ND_SEGMENT              segment;
    P_ND_SEGMENT            pSegment        = &segment;

    ND_HEADER_RECORD        ndHeaderRecord;
    P_ND_HEADER_RECORD      pNdHeaderRecord = &ndHeaderRecord;

    ND_CTRL_RECORD_FORMAT   ctrlRecFormat   = UNKNOWN;

    // clear segment
    bzero(pSegment, sizeof(ND_SEGMENT));

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

    if (iErr == 0) {
        iErr = getHeaderRecord(pSegment, pNdHeaderRecord);
    }

#ifdef __CROSS__
    ebcdicToAscii((BYTE *)&pNdHeaderRecord->INMFNODE, sizeof(pNdHeaderRecord->INMFNODE));
    ebcdicToAscii((BYTE *)&pNdHeaderRecord->INMFUID,  sizeof(pNdHeaderRecord->INMFUID));
    ebcdicToAscii((BYTE *)&pNdHeaderRecord->INMFTIME, sizeof(pNdHeaderRecord->INMFTIME));
#endif

    printf("XMIT file was created by %.8s.%.8s on %.2s.%.2s.%.4s at %.2s:%.2s:%.2s \n",
           pNdHeaderRecord->INMFNODE,
           pNdHeaderRecord->INMFUID,
           pNdHeaderRecord->INMFTIME.day,
           pNdHeaderRecord->INMFTIME.month,
           pNdHeaderRecord->INMFTIME.year,
           pNdHeaderRecord->INMFTIME.hour,
           pNdHeaderRecord->INMFTIME.minute,
           pNdHeaderRecord->INMFTIME.second);
}


