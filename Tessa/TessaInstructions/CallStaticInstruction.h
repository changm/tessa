namespace TessaInstructions {

	/***
	 * This is used when we can call a static method. Static methods are methods without
	 * a receiver object (I think. Still an immature class)
	 */
	class CallStaticInstruction : public CallInstruction {
	private:

	public:
		CallStaticInstruction(TessaInstruction* receiverObject, Traits* resultTraits, ArrayOfInstructions* arguments, uintptr_t methodId, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		CallStaticInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		bool isCallStatic(); 
	};

}