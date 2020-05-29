#ifndef BREXX_UTIL_H
#define BREXX_UTIL_H
#include "rxmvsext.h"

typedef enum quotation { UNQUOTED, PARTIALLY_QUOTED, FULL_QUOTED } QuotationType;

QuotationType CheckQuotation(const char *sDSName);
int getDatasetName(RX_ENVIRONMENT_CTX_PTR pEnvironmentCtx,  const char *datasetNameIn, char datasetNameOut[44 + 1]);
int IsReturnCode(char * input);
long getFileSize(FILE *pFile);
void DumpHex(const unsigned char* data, size_t size);

void ebcdicToAscii(unsigned char *s, unsigned int length);

#endif //BREXX_UTIL_H
