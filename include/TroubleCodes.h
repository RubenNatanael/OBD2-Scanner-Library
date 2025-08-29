#ifndef TROUBLE_CODE
#define TROUBLE_CODE

#include <string>

#ifdef ARDUINO

// Don't include DTC description(Memory limitation)
std::string Parse(std::string &DtcCode) {
    return DtcCode;
}

#else

extern const std::string troubleCodes[];
extern const size_t troubleCodesCount;

int letterOrder(char c);

int FindDtcInList(int start, int end, const std::string &DtcCode);

std::string Parse(std::string &DtcCode);

#endif

#endif
