
namespace TessaInstructions {
	class PopScopeInstruction : public ScopeInstruction {
	public:
		PopScopeInstruction(TessaVM::BasicBlock* insertAtEnd);
		void visit(TessaVisitorInterface* tessaVisitor);
		void print();
		PopScopeInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
	};
}