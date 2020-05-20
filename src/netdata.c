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
ebcdicToAscii(unsigned char *s, unsigned int length)
{
    unsigned int uiCurrentPosition = 0;
    while (uiCurrentPosition < length)
    {
        s[uiCurrentPosition] = e2a[(int) (s[uiCurrentPosition])];
        uiCurrentPosition++;
    }
}

int
getBinaryValue(BYTE *ptr, int len)
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

    return UNKNOWN_CTRL_REC_FORMAT;
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

    if (getControlRecordFormat(pSegment) == INMR02) {
        uiCurrentPosition = 4;
    }

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
        uiCurrentLength = getBinaryValue((BYTE *)&(*(hTextUnit))->value.length, 2);

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
getTextUnitLength(P_ND_TEXT_UNIT pTextUnit)
{
    return getBinaryValue((BYTE *) &pTextUnit->value.length, 2);
}

int
getTextUnitNumber(P_ND_TEXT_UNIT pTextUnit)
{
    return getBinaryValue((BYTE *) &pTextUnit->number, 2);
}

int
getTextUnitValue(P_ND_TEXT_UNIT pTextUnit, unsigned int uiSearchNumber, P_ND_TEXT_UNIT_VALUE *hTextUnitValue)
{
    int             iErr                    = 0;

    unsigned int    uiMaxNumber             = 0;
    unsigned int    uiCurrentNumber         = 0;
    unsigned int    uiCurrentDataLength     = 0;

    BYTE            *pTextUnitValue         = 0;

    pTextUnitValue = (BYTE *) &pTextUnit->value;
    uiMaxNumber = getTextUnitNumber(pTextUnit);

    if (uiSearchNumber > uiMaxNumber) {
        iErr = 1;
    }

    if (iErr == 0) {
        for (uiCurrentNumber = 1; uiCurrentNumber <= uiMaxNumber; uiCurrentNumber++) {

            if (uiCurrentNumber == uiSearchNumber) {
                *hTextUnitValue = (P_ND_TEXT_UNIT_VALUE)pTextUnitValue;
            } else {
                uiCurrentDataLength = getBinaryValue((BYTE *) &((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length,
                                                         sizeof((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length);
                pTextUnitValue += uiCurrentDataLength +  sizeof((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length;
            }
        }
    }

    return iErr;
}

int
getHeaderRecord(P_ND_SEGMENT pSegment, P_ND_HEADER_RECORD pHeaderRecord)
{
    int                     iErr            = 0;

    P_ND_TEXT_UNIT      pTextUnit           = NULL;
    P_ND_TEXT_UNIT      *hTextUnit          = &pTextUnit;

    unsigned int        uiCurrentPosition   = 0;
    unsigned int        uiTextUnitLength    = 0;

    int                 iFound              = 0;

    /* mandatory fields */
    bzero(pHeaderRecord->INMFNODE, sizeof(pHeaderRecord->INMFNODE));
    iErr = getTextUnit(pSegment, ND_INMFNODE, hTextUnit);
    if (iErr == 0) {
        memcpy(pHeaderRecord->INMFNODE, pTextUnit->value.data, getTextUnitLength(pTextUnit));
    }

    if (iErr == 0) {
        bzero(pHeaderRecord->INMFUID, sizeof(pHeaderRecord->INMFUID));
        iErr = getTextUnit(pSegment, ND_INMFUID, hTextUnit);
        if (iErr == 0) {
            memcpy(pHeaderRecord->INMFUID, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pHeaderRecord->INMFTIME, sizeof(pHeaderRecord->INMFTIME));
        iErr = getTextUnit(pSegment, ND_INMFTIME, hTextUnit);
        if (iErr == 0) {
            memcpy(&pHeaderRecord->INMFTIME, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pHeaderRecord->INMLRECL, sizeof(pHeaderRecord->INMLRECL));
        iErr = getTextUnit(pSegment, ND_INMLRECL, hTextUnit);
        if (iErr == 0) {
            pHeaderRecord->INMLRECL = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pHeaderRecord->INMTNODE, sizeof(pHeaderRecord->INMTNODE));
        iErr = getTextUnit(pSegment, ND_INMTNODE, hTextUnit);
        if (iErr == 0) {
            memcpy(&pHeaderRecord->INMTNODE, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pHeaderRecord->INMTUID, sizeof(pHeaderRecord->INMTUID));
        iErr = getTextUnit(pSegment, ND_INMTUID, hTextUnit);
        if (iErr == 0) {
            memcpy(&pHeaderRecord->INMTUID, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    /* optional fields */
    if (iErr == 0) {
        bzero(&pHeaderRecord->INMFACK, sizeof(pHeaderRecord->INMFACK));
        iFound = getTextUnit(pSegment, ND_INMFACK, hTextUnit);
        if (iFound == 0) {
            memcpy(&pHeaderRecord->INMFACK, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pHeaderRecord->INMFVERS, sizeof(pHeaderRecord->INMFVERS));
        iFound = getTextUnit(pSegment, ND_INMFVERS, hTextUnit);
        if (iFound == 0) {
            pHeaderRecord->INMFVERS = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pHeaderRecord->INMNUMF, sizeof(pHeaderRecord->INMNUMF));
        iFound = getTextUnit(pSegment, ND_INMNUMF, hTextUnit);
        if (iFound == 0) {
            pHeaderRecord->INMNUMF = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pHeaderRecord->INMUSERP, sizeof(pHeaderRecord->INMUSERP));
        iFound = getTextUnit(pSegment, ND_INMUSERP, hTextUnit);
        if (iFound == 0) {
            memcpy(&pHeaderRecord->INMUSERP, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    return iErr;
}

int
getFileUtilRecord(P_ND_SEGMENT pSegment, P_ND_FILE_UTIL_RECORD pFileUtilRecord)
{
    int                     iErr            = 0;

    P_ND_TEXT_UNIT      pTextUnit           = NULL;
    P_ND_TEXT_UNIT      *hTextUnit          = &pTextUnit;

    unsigned int        uiCurrentPosition   = 0;
    unsigned int        uiTextUnitLength    = 0;
    unsigned int        uiTextUnitNumber    = 0;
    unsigned int        uiCurrentNumber     = 0;
    unsigned int        uiDataLength        = 0;
    int                 iFound              = 0;

    /* mandatory fields */
    bzero(&pFileUtilRecord->INMDSORG, sizeof(pFileUtilRecord->INMDSORG));
    iErr = getTextUnit(pSegment, ND_INMDSORG, hTextUnit);
    if (iErr == 0) {
        pFileUtilRecord->INMDSORG = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
    }

    if (iErr == 0) {
        bzero(&pFileUtilRecord->INMLRECL, sizeof(pFileUtilRecord->INMLRECL));
        iErr = getTextUnit(pSegment, ND_INMLRECL, hTextUnit);
        if (iErr == 0) {
            pFileUtilRecord->INMLRECL = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pFileUtilRecord->INMRECFM, sizeof(pFileUtilRecord->INMRECFM));
        iErr = getTextUnit(pSegment, ND_INMRECFM, hTextUnit);
        if (iErr == 0) {
            memcpy(&pFileUtilRecord->INMRECFM, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pFileUtilRecord->INMSIZE, sizeof(pFileUtilRecord->INMSIZE));
        iErr = getTextUnit(pSegment, ND_INMSIZE, hTextUnit);
        if (iErr == 0) {
            pFileUtilRecord->INMSIZE = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    if (iErr == 0) {
        bzero(&pFileUtilRecord->INMUTILN, sizeof(pFileUtilRecord->INMUTILN));
        iErr = getTextUnit(pSegment, ND_INMUTILN, hTextUnit);
        if (iErr == 0) {
            memcpy(&pFileUtilRecord->INMUTILN, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }
    }

    /* optional fields */
    if (iErr == 0) {
        bzero(&pFileUtilRecord->INMBLKSZ, sizeof(pFileUtilRecord->INMBLKSZ));
        iFound = getTextUnit(pSegment, ND_INMBLKSZ, hTextUnit);
        if (iFound == 0) {
            pFileUtilRecord->INMBLKSZ = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMCREAT, sizeof(pFileUtilRecord->INMCREAT));
        iFound = getTextUnit(pSegment, ND_INMCREAT, hTextUnit);
        if (iFound == 0) {
            memcpy(&pFileUtilRecord->INMCREAT, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMDIR, sizeof(pFileUtilRecord->INMDIR));
        iFound = getTextUnit(pSegment, ND_INMDIR, hTextUnit);
        if (iFound == 0) {
            pFileUtilRecord->INMDIR = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMDSNAM, sizeof(pFileUtilRecord->INMDSNAM));
        iFound = getTextUnit(pSegment, ND_INMDSNAM, hTextUnit);
        if (iFound == 0) {
            BYTE *pTextUnitValue = (BYTE *)&pTextUnit->value;
            uiTextUnitNumber = getTextUnitNumber(pTextUnit);

            for (uiCurrentNumber = 1; uiCurrentNumber <= uiTextUnitNumber; uiCurrentNumber++) {
                uiDataLength = getBinaryValue((BYTE *) &((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length,
                        sizeof((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length);

                memcpy(&pFileUtilRecord->INMDSNAM[uiCurrentPosition],
                       ((P_ND_TEXT_UNIT_VALUE)pTextUnitValue)->data, uiDataLength);

                uiCurrentPosition += uiDataLength;
                pTextUnitValue    += uiDataLength + sizeof((P_ND_TEXT_UNIT_VALUE) pTextUnitValue)->length;
            }
        }

        bzero(&pFileUtilRecord->INMEXPDT, sizeof(pFileUtilRecord->INMEXPDT));
        iFound = getTextUnit(pSegment, ND_INMEXPDT, hTextUnit);
        if (iFound == 0) {
            memcpy(&pFileUtilRecord->INMEXPDT, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMFFM, sizeof(pFileUtilRecord->INMFFM));
        iFound = getTextUnit(pSegment, ND_INMFFM, hTextUnit);
        if (iFound == 0) {
            pFileUtilRecord->INMFFM = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMLCHG, sizeof(pFileUtilRecord->INMLCHG));
        iFound = getTextUnit(pSegment, ND_INMLCHG, hTextUnit);
        if (iFound == 0) {
            memcpy(&pFileUtilRecord->INMLCHG, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMLREF, sizeof(pFileUtilRecord->INMLREF));
        iFound = getTextUnit(pSegment, ND_INMLREF, hTextUnit);
        if (iFound == 0) {
            memcpy(&pFileUtilRecord->INMEXPDT, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMTERM, sizeof(pFileUtilRecord->INMTERM));
        iFound = getTextUnit(pSegment, ND_INMTERM, hTextUnit);
        if (iFound == 0) {
            pFileUtilRecord->INMTERM = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMUSERP, sizeof(pFileUtilRecord->INMUSERP));
        iFound = getTextUnit(pSegment, ND_INMUSERP, hTextUnit);
        if (iFound == 0) {
            memcpy(&pFileUtilRecord->INMUSERP, pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

        bzero(&pFileUtilRecord->INMMEMBRN, sizeof(pFileUtilRecord->INMMEMBRN));
        iFound = getTextUnit(pSegment, ND_INMMEMBR, hTextUnit);
        if (iFound == 0) {
            pFileUtilRecord->INMMEMBRN = getBinaryValue(pTextUnit->value.data, getTextUnitLength(pTextUnit));
        }

    }

    return iErr;
}


