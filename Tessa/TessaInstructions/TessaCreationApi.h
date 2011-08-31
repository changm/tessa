
namespace TessaVM {
	class ASFunction;
	class BasicBlock;
}

namespace TessaInstructions {
	using namespace TessaVM;

	class TessaCreationApi : public MMgc::GCObject {
	private:
		// Data
		MMgc::GC *gc;
		avmplus::AvmCore* core;
		ASFunction* currentFunction;
		TessaVM::BasicBlock* _currentBasicBlock;

		// Methods
	private:
		void					consistencyCheck();
		void					convertIntoSsa(); 

	public:
		TessaCreationApi(avmplus::AvmCore* core, MMgc::GC * gc); 
		void						printResults();
		TessaVM::BasicBlock*		createNewFunction(MethodInfo* methodInfo, string methodName, int numberOfLocals, int scopeStackSize);
		ASFunction*					getFunction();

		TessaVM::BasicBlock*		createNewBasicBlock();
		void						switchToBasicBlock(TessaVM::BasicBlock* labelInstruction);

		NextNameInstruction*		createNextNameInstruction(TessaInstruction* receiverObject, TessaInstruction* registerInstruction);
		HasMorePropertiesInstruction*			createHasMorePropertiesInstruction(HasMorePropertiesObjectInstruction* objectInstruction, HasMorePropertiesRegisterInstruction* registerInstruction);
		HasMorePropertiesObjectInstruction*		createHasMorePropertiesObjectInstruction(TessaInstruction* objectInstruction);
		HasMorePropertiesRegisterInstruction*	createHasMorePropertiesRegisterInstruction(TessaInstruction* registerInstruction);
		

		void						setLocalVariable(int localVariableNumber, TessaInstruction* newReference);
		TessaInstruction*			getLocalVariable(int localVariableNumber);
		void						trackExtraVariableInEndState(int index, TessaInstruction* reference, int startingNumberOfReferences);
		void						trackExtraVariableInEntryState(int index, TessaInstruction* reference, int startingNumberOfReferences);
		void						stopTrackingExtraVariable(int index); 
		void						finishMethod();
	};
}