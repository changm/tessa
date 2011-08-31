
namespace TessaInstructions {
	class NewArrayInstruction : public TessaInstruction {
	private:
		List<TessaInstruction*, avmplus::LIST_GCObjects>* arrayElements;

	public:
		NewArrayInstruction(List<TessaInstruction*, avmplus::LIST_GCObjects>* arrayElements, TessaVM::BasicBlock* insertAtEnd); 
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		void addElement(TessaInstruction* element);
		TessaInstruction* getElement(uint32_t index);
		List<TessaInstruction*, avmplus::LIST_GCObjects> * getArrayElements();
		int numberOfElements();
		bool producesValue();
		bool isNewArray();
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}