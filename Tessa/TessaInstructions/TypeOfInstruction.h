
/***
 * Like instance of but checks to see if an object is of a certain type
 */
namespace TessaInstructions {
	class TypeOfInstruction : public TessaInstruction {
	private:
		bool lateCheck;
		TessaInstruction* objectToTest;
		TessaInstruction* typeToCompare;
		const Multiname* multiname;

	public:
		TypeOfInstruction(TessaInstruction* objectToTest, TessaInstruction* typeToCompare, TessaVM::BasicBlock* insertAtEnd);
		TypeOfInstruction(TessaInstruction* objectToTest, const Multiname* multiname, TessaVM::BasicBlock* insertAtEnd);
		bool isLateCheck();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		TessaInstruction* getObjectToTest();
		TessaInstruction* getTypeToCompare();
		const Multiname* getMultiname();
		bool producesValue();
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}