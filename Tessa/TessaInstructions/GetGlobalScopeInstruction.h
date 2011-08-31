
/***
 * Gets the global scope / object. 
 * We make this an explicit instruction because some programs heavily use the global object. However, we could
 * also roll this into the GetScopeObjectInstruction as SquirrelFish does. I'm unsure if we really need it, but the current
 * implementation of the way the VM keeps track of scopes seems like we have to.
 *
 * IT also has weird semantics.
 * If the scope depth is > 0, then we can assume that the global object is passed into ScopeChain on the MEthodEnv.
 * if the scope depth is == 0, we get the 0th item on the runtime scope stack.
 */
namespace TessaInstructions {
	class GetGlobalScopeInstruction : public GetScopeObjectInstruction {
	public:
		GetGlobalScopeInstruction(TessaVM::BasicBlock* insertAtEnd);
		void visit(TessaVisitorInterface* tessaVisitor);
		void print();
		GetGlobalScopeInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
	};
}