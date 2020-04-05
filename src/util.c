#include <stdio.h>
#include <ctype.h>

#include "lstring.h"
#include "util.h"

int IsReturnCode(char * input) {
    int iRet = 0;

    if (isdigit(input[0])) {
        iRet = 1;
    } else if (input[0] == '-' && isdigit(input[1])) {
        iRet = 1;
    }

    return iRet;
}

QuotationType CheckQuotation(char *sDSName)
{
    bool bQuotationMarkAtBeginning  = FALSE;
    bool bQuotationMarkAtEnd        = FALSE;
    QuotationType quotationType     = UNQUOTED;

    /* define possible quotation mark positions */
    size_t iFirstCharPos = 0;
    size_t iLastCharPos  = (strlen(sDSName) > 0) ? (strlen(sDSName) - 1) : 0;

    /* get chars at defined positions */
    unsigned char cFirstChar = (unsigned char)sDSName[iFirstCharPos];
    unsigned char cLastChar  = (unsigned char)sDSName[iLastCharPos];

    if (cFirstChar == '\'' || cFirstChar == '\"') {
        bQuotationMarkAtBeginning = TRUE;
    }

    if (cLastChar == '\'' || cLastChar == '\"') {
        bQuotationMarkAtEnd = TRUE;
    }

    if ((bQuotationMarkAtBeginning) && (bQuotationMarkAtEnd)) {
        quotationType = FULL_QUOTED;
    } else if ((bQuotationMarkAtBeginning) || (bQuotationMarkAtEnd)) {
        quotationType = PARTIALLY_QUOTED;
    }

    return quotationType;
}

void DumpHex(const unsigned char* data, size_t size)
{
    char ascii[17];
    size_t i, j;
    bool padded = FALSE;

    ascii[16] = '\0';

    printf("%08X (+%08X) | ", &data[0], 0);
    for (i = 0; i < size; ++i) {
        printf("%02X", data[i]);

        if ( isprint(data[i])) {
            ascii[i % 16] = data[i];
        } else {
            ascii[i % 16] = '.';
        }

        if ((i+1) % 4 == 0 || i+1 == size) {
            if ((i+1) % 4 == 0) {
                printf(" ");
            }

            if ((i+1) % 16 == 0) {
                printf("| %s \n", ascii);
                if (i+1 != size) {
                    printf("%08X (+%08X) | ", &data[i+1], i+1);
                }
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';

                for (j = (i+1) % 16; j < 16; ++j) {
                    if ((j) % 4 == 0) {
                        if (padded) {
                            printf(" ");
                        }
                    }
                    printf("  ");
                    padded = TRUE;
                }
                printf(" | %s \n", ascii);
            }
        }
    }
}
