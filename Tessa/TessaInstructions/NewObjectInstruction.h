
namespace TessaInstructions {
	class NewObjectInstruction : public TessaInstruction {
	private:
		/***
		 * By specification new object properties is a list of alternating name/value pairs
		 * eg newObjectProperties[0] = name of the property
		 * newObjectProperties[1] = the value of newObjectProperties[0]
		 */
		ArrayOfInstructions* newObjectProperties;

	public:
		NewObjectInstruction(ArrayOfInstructions* newObjectProperties, TessaVM::BasicBlock* insertAtEnd);
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isNewObject();
		bool producesValue();
		ArrayOfInstructions*	getObjectProperties();
		int getNumberOfProperties();
		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}