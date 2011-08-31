
namespace TessaInstructions {
	class ConstructSuperInstruction : public ConstructInstruction {
	private:
		MethodInfo* _methodInfo;

	public:
		ConstructSuperInstruction(TessaInstruction *receiverObject, Traits* receiverTraits, ArrayOfInstructions* arguments, MethodInfo* methodInfo, TessaVM::BasicBlock* insertAtEnd);
		void print();
		bool isConstructSuper();
		bool isSuper();
		void visit(TessaVisitorInterface* tessaVisitor);
		MethodInfo* getMethodInfo();
		ConstructSuperInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
	};
}