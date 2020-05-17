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
    if (len == 2) {
         binaryValue = *(short *) ptr;
    } else if (4) {
         binaryValue = *(int *) ptr;
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
    int             iErr        = 0;

    unsigned long   ulLength;
    unsigned long   ulBytesRead;

    // read segment header
    ulBytesRead = fread((void *) &pSegment->header, 1, sizeof(SEGMENT_HEADER), pFile);
    if (ulBytesRead != sizeof(SEGMENT_HEADER)) {
        iErr = EOF;
    }

    // check segment header length value
    if (iErr == 0) {
        ulLength = (pSegment->header.length & 0xFFu) - 2;

        if (ulLength > 253) {
            iErr = -2;
        }
    }

    // read the entire segment
    if (iErr == 0) {
        ulBytesRead = fread(&pSegment->data, 1, ulLength, pFile);
        if (ulBytesRead != ulLength) {
            iErr = EOF;
        }
    }

    return iErr;
}

int
isControlRecord(P_SEGMENT pSegment)
{
    return
        ((pSegment->header.flags & SDF_FIRST_SEGMENT) == SDF_FIRST_SEGMENT) &&
        ((pSegment->header.flags & SDF_LAST_SEGMENT) == SDF_LAST_SEGMENT)   &&
        ((pSegment->header.flags & SDF_CONTROL_RECORD) == SDF_CONTROL_RECORD);
}

CONTROL_RECORD_FORMAT
getControlRecordFormat(P_SEGMENT pSegment)
{
    char        identifier[6];
    BYTE        data[247];

    printf("FOO> sizeof(SEGMENT) %lu\n", sizeof(SEGMENT));
    printf("FOO> sizeof(SEGMENT_HEADER) %lu\n", sizeof(SEGMENT_HEADER));
    printf("FOO> header length = %x, flags = %x\n", pSegment->header.length, pSegment->header.flags);
    printf("FOO> sizeof(SEGMENT_DATA) %lu\n", sizeof(SEGMENT_DATA));
    printf("FOO> sizeof(CONTTROL_RECORD_DATA) %lu\n", sizeof(CONTTROL_RECORD_DATA));
    printf("FOO> sizeof(CONTTROL_RECORD_DATA.ID) %lu\n", sizeof(identifier));
    printf("FOO> sizeof(CONTTROL_RECORD_DATA.DATA) %lu\n", sizeof(data));
    printf("FOO> sizeof(TEXT_UNIT) %lu\n", sizeof(TEXT_UNIT));
    printf("FOO> sizeof(TEXT_UNIT_HEADER) %lu\n", sizeof(TEXT_UNIT_HEADER));
    printf("FOO> sizeof(TEXT_UNIT_DATA) %lu\n", sizeof(TEXT_UNIT_DATA));

    printf("FOO> the segment\n");
    DumpHex((char *)pSegment, 96);

    printf("FOO> the data\n");
    DumpHex(pSegment->data.conttrolRecordData.identifier, 6);

    printf("FOO> a search value\n");
    DumpHex(HEX_INMR01,6);

    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR01, 6) == 0) return INMR01;
    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR02, 6) == 0) return INMR02;
    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR03, 6) == 0) return INMR03;
    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR04, 6) == 0) return INMR04;
    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR06, 6) == 0) return INMR06;
    if (memcmp(pSegment->data.conttrolRecordData.identifier, HEX_INMR07, 6) == 0) return INMR07;

    return UNKNOWN;
}

int
getTextUnit(P_SEGMENT pSegment, unsigned int uiSearchKey, P_TEXT_UNIT *hTextUnit)
{
    int             iErr                        = 0;

    unsigned int    uiCurrentPosition           = 0;

    unsigned int    uiCurrentKey                = 0;
    unsigned int    uiCurrentNumber             = 0; // number of text unit data elements e.g. length/data pairs
    unsigned int    uiCurrentLength             = 0; // length of real data in text unit data element

    BYTE            *pCurrentTextUnit           = 0;

    // todo: ende richtig definiren
    while (uiCurrentPosition <= 241) {

        pCurrentTextUnit  = (BYTE *)pSegment->data.conttrolRecordData.data;
        pCurrentTextUnit += uiCurrentPosition;

        *hTextUnit = ((P_TEXT_UNIT) pCurrentTextUnit);

        uiCurrentKey    = getBinaryValue((BYTE *)&(*(hTextUnit))->header.key, 2);
        uiCurrentNumber = getBinaryValue((BYTE *)&(*(hTextUnit))->header.number, 2);
        uiCurrentLength = getBinaryValue((BYTE *)&(*(hTextUnit))->data->length, 2);

        if (uiCurrentKey == uiSearchKey) {
            return 0;
        } else {
            uiCurrentPosition += sizeof(TEXT_UNIT_HEADER) + 2 + (uiCurrentLength * uiCurrentNumber) ;
        }
    }

    return -1;
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

    // get first key
    if (iErr == 0) {
        char node[9];
        bzero(node,9);

        iErr = getTextUnit(pSegment, INMFNODE, hTextUnit);

        if (iErr == 0) {
            memcpy(node, pTextUnit->data->data, getBinaryValue((BYTE *)&pTextUnit->data->length, 2));
#ifdef __CROSS__
            ebcdicToAscii(node);
#endif
            printf("FOO> %s\n", node);
        }
    }

    printf("FOO> iErr = %d\n", iErr);
}


