
/***
 * This represents a Switch statement.
 * A switch contains the value to switch on and a list of case instructions
 * The default case is always the LAST item on the caseInstructions list
 */
namespace TessaInstructions {
	class SwitchInstruction : public TessaInstruction {
	private:
		List<CaseInstruction*, avmplus::LIST_GCObjects>* _caseInstructions;
		TessaInstruction* _switchValue;

	public:
		SwitchInstruction::SwitchInstruction(MMgc::GC* gc, TessaInstruction* switchValue, TessaVM::BasicBlock* insertAtEnd); 
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isSwitch();
		List<CaseInstruction*, LIST_GCObjects>* getCaseInstructions();
		CaseInstruction* getDefaultCase();
		TessaInstruction* getSwitchValue();
		void addCaseInstruction(CaseInstruction* caseInstruction);
		void setDefaultCase(CaseInstruction* caseInstruction); 
		int numberOfCases();
		CaseInstruction*	getCase(int index);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
		bool hasSideEffect() { return true; }
	};

}