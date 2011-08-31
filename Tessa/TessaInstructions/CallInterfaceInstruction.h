namespace TessaInstructions {
	class CallInterfaceInstruction : public CallInstruction {
	private:

	public:
		CallInterfaceInstruction(TessaInstruction* receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, 
			uint32_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd); 
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isCallInterface();
		CallInterfaceInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
	};

}