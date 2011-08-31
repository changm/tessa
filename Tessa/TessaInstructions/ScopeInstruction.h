
namespace TessaInstructions {
	class ScopeInstruction : public TessaInstruction {
	public:
		ScopeInstruction(TessaVM::BasicBlock* insertAtEnd);
		bool modifiesScopeStack();
		bool hasSideEffect() { return true; }	
	};
}