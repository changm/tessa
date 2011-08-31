
namespace TessaInstructions {
	class ParameterInstruction : public TessaInstruction {
	private:
		TessaInstruction* defaultValue;

	public:
		ParameterInstruction(TessaInstruction* defaultValue, TessaVM::BasicBlock* insertAtEnd);
		TessaInstruction* getDefaultValue(); 
		void setDefaultValue(TessaInstruction *newDefaultValue);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isParameter();
		bool producesValue();
		ParameterInstruction* clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}