
/***
 * This denotes the beginning of an inlined method. Ideally, we could get rid of this, but each method is linked to a
 * different MethodEnv*. Usually, the CallInstruction implicitly has to load this new MethodEnv, but since we inlined the method
 * The call instruction no longer has to. But we still have to load the new MethodEnv*, and so this instruction
 * makes that load explicit. Terrible.
 */
namespace TessaInstructions {
	class InlineBeginInstruction : public TessaInstruction {
	private:
		LoadVirtualMethodInstruction* _loadVirtualMethod;
		Traits* _resultTraits;
		int _methodId;
		int _scopeStackIncreaseSize;

	public:
		InlineBeginInstruction(LoadVirtualMethodInstruction* loadVirtualMethod, Traits* resultTraits, int methodId, int callerMaxScopeSize, int increaseInScopeStackSize, MethodInfo* inlinedMethod, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* visit);
		InlineBeginInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 

		Traits*				getResultTraits();
		int					getmethodId();
		LoadVirtualMethodInstruction* getLoadedMethod();
		int getIncreaseInScopeStack();

		int _callerScopeSize;
		MethodInfo* _inlinedMethod;
		bool hasSideEffect() { return true; }
	};
}