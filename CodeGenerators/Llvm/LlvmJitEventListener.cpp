
#include "LlvmCodeGenerator.h"

namespace LlvmCodeGenerator {
	void LlvmJitEventListener::NotifyFunctionEmitted(const llvm::Function &functionCompiled,
                                     void *Code, size_t Size,
									 const EmittedFunctionDetails &Details) {
		 this->sizeOfCompiledCode = Size;
	}
}	// End namespace