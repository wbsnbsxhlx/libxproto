#include "libxproto.h"
#include <stdarg.h>
#include <string.h>

void strAppendFmt(std::string& str, const char* formatTxt, ...) {
	va_list argList;
	va_start(argList, formatTxt);

	char buf[256];
	vsprintf_s(buf, 255, formatTxt, argList);
	str += buf;
}
