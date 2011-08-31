namespace TessaInstructions {

	class LoadVirtualMethodInstruction : public TessaInstruction {
	private:
		int _methodId;
		TessaInstruction* _receiverObject;

		TessaValue*	_loadedMethodEnv;
		TessaValue*	_loadedMethodInfo;

	public:
		LoadVirtualMethodInstruction(MMgc::GC* gc, TessaInstruction* receiverObject, int methodId, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		int getMethodId();
		TessaInstruction* getReceiverObject(); 
		LoadVirtualMethodInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);

		TessaValue*	getLoadedMethodEnv();
		TessaValue*	getLoadedMethodInfo();
		bool isLoadVirtualMethod();
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
		bool hasSideEffect() { return true; }
	};

}