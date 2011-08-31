
namespace LlvmCodeGenerator {
	class LlvmJitEventListener : public llvm::JITEventListener {
	public:
		size_t sizeOfCompiledCode;

		void NotifyFunctionEmitted(const llvm::Function &F,
                                     void *Code, size_t Size,
									 const EmittedFunctionDetails &Details);
	};
}