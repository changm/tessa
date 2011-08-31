
namespace TessaInstructions {
	using namespace avmplus;

	class ArrayOfInstructions : public TessaInstruction {
	private:
		avmplus::List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions;

	public:
		ArrayOfInstructions(MMgc::GC* gc, TessaVM::BasicBlock* insertAtEnd); 
		void addInstruction(TessaInstruction* instruction);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		TessaInstruction* getInstruction(uint32_t index);
		void setInstruction(int index, TessaInstruction* instruction); 
		List<TessaInstruction*, avmplus::LIST_GCObjects>* getInstructions();
		uint32_t size();
		bool isArrayOfInstructions();
		ArrayOfInstructions*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}