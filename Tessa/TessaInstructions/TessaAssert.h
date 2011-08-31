#ifndef __TessaAssert__
#define __TessaAssert__

namespace TessaInstructions {

#ifdef _DEBUG
	void _TessaAssertMessage(bool condition, char *message, long line, char* file); 
	void _TessaAssert(bool condition, long line, char *file); 

#define TessaAssert(condition) \
	_TessaAssert(condition, __LINE__, __FILE__);

#define TessaAssertMessage(condition, message) \
	_TessaAssertMessage(condition, message, __LINE__, __FILE__);
#else

#define TessaAssert(condition)
#define TessaAssertMessage(condition, message)

#endif
}

#endif /* Tessa Assert */