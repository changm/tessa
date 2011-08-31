
namespace TessaInstructions {
	class BranchInstruction : public TessaInstruction {
	private:

	public:
		BranchInstruction(TessaVM::BasicBlock* insertAtEnd);
		bool				isBranch();
		virtual void		forceAbstract() = 0;
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc) { AvmAssert(false); return NULL; }
		bool hasSideEffect() { return true; }
	};
}
