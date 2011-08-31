
namespace TessaConstants {
	class TessaConstantValue;
}

namespace TessaInstructions {
	using namespace TessaConstants;

	class ConstantValueInstruction : public TessaInstruction {
	private:
		ConstantValue* _constantValue;

	public:
		ConstantValueInstruction(ConstantValue* constantValue, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		ConstantValue* getConstantValue();
		bool producesValue();
		bool isConstantValue();
		ConstantValueInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}