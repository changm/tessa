
namespace avmplus {
	class AvmCore;
	class MethodEnv;
	class MethodInfo;
}

namespace LlvmCodeGenerator {
	class LlvmJitCompiler;
}

#include <string>

namespace TessaCodeGenerators {
	using namespace LlvmCodeGenerator;
	using namespace std;

	// Really (MethodEnv*, int argCount, Atom* arguments);
	typedef int (*MethodInvoke)(avmplus::MethodEnv*, int, int*);

	class TessaCodeGenerator {
	private:
		LlvmJitCompiler*	llvmJitCompiler;
		MethodInvoke		_nativeCodeLocation;

		std::string createMethodName(avmplus::MethodInfo* env); 

	public:
		TessaCodeGenerator();
		void compileCode(ASFunction* function, avmplus::MethodInfo* methodInfo, avmplus::AvmCore* core); 
		MethodInvoke getNativeCodeLocation();
		static void deleteInstance();
	};
}