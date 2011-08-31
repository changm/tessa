namespace TessaInstructions {
	class InlineMethodInstruction : public CallInstruction {
	public:
		InlineMethodInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd);
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isInlineMethod();
	};
}