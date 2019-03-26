#ifndef BREXX_UTIL_H
#define BREXX_UTIL_H

typedef enum quotation { UNQUOTED, PARTIALLY_QUOTED, FULL_QUOTED } QuotationType;

QuotationType CheckQuotation(char *sDSName);
void DumpHex(const unsigned char* data, size_t size);

#endif //BREXX_UTIL_H
