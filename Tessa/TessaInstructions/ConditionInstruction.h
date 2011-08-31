
namespace TessaInstructions {
	class ConditionInstruction : public BinaryInstruction {
	private:
	public:
		bool forceOptimizeNullPointerCheck;

		TessaValue* getClonedValue(TessaValue* operand, MMgc::GCHashtable* originalToCloneMap);

	public:
		ConditionInstruction(TessaBinaryOp op, TessaValue* leftOperand, TessaValue* rightOperand, TessaVM::BasicBlock* insertAtEnd);
		bool isCondition();
		void print();
		void visit(TessaVisitorInterface *tessaVisitor);
		ConditionInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc) { return BinaryInstruction::getOperands(gc); }
	};
}