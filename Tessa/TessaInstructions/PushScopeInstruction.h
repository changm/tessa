
namespace TessaInstructions {
	class PushScopeInstruction : public ScopeInstruction {
	public:
		TessaInstruction* _scopeObject;
		PushScopeInstruction(TessaInstruction* scopeObject, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		TessaInstruction* getScopeObject();
		PushScopeInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}