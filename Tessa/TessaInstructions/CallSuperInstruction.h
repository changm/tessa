
namespace TessaInstructions {
	class CallSuperInstruction : public CallInstruction {
	private:
		const Multiname* multiname;
		bool earlyBound;

	public:
		CallSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd);
		CallSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd);

		bool isCallSuper();
		bool isSuper();
		bool isEarlyBound();
		void visit(TessaVisitorInterface* tessaVisitor);
		void print();
		const Multiname* getMultiname();
		CallSuperInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
	};

}