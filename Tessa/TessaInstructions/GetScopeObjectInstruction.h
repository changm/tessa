
namespace TessaInstructions {
	class GetScopeObjectInstruction : public ScopeInstruction {
	private:
		int32_t scopeIndex;
		bool lookInOuterScope;

	public:
		GetScopeObjectInstruction(int32_t scopeIndex, bool lookInOuterScope, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		int32_t getScopeIndex();
		bool isOuterScope();
		GetScopeObjectInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd); 
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}