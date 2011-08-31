#include "TessaInstructionHeader.h"

namespace TessaInstructions {

#ifdef _DEBUG
	void _TessaAssertMessage(bool condition, char *message, long line, char* file) {
		if (!condition) {
			printf("%s : (%s, line %d)\n", message, file, line);
			::DebugBreak();
		}
	}
#endif

#ifdef _DEBUG
	void _TessaAssert(bool condition, long line, char *file) {
		_TessaAssertMessage(condition, "tessa assert fail", line, file);
	}
#endif
}