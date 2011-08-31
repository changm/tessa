

namespace TessaInstructions {
	using namespace avmplus;

	class CallVirtualInstruction : public CallInstruction {
	private:
		LoadVirtualMethodInstruction* _methodToCall;

	public:
		CallVirtualInstruction(TessaInstruction* receiverObject, LoadVirtualMethodInstruction* methodToCall, 
			Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		CallVirtualInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		bool isCallVirtual(); 
		LoadVirtualMethodInstruction*	getLoadedMethodToCall();
	};

}