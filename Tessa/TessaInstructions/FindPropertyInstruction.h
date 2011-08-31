
namespace avmplus {
	class Multiname;
}

namespace TessaInstructions {
	using namespace avmplus;
	using namespace TessaVisitors;

	class FindPropertyInstruction : public TessaInstruction {
	private:
		const Multiname* _name;
		bool _strictLookup;
		bool _isFindDef;
		uint32_t _nameIndex;
		int _scopeDepth;	// Used for consistency checker from verifier -> runtime
		int _cacheSlot;

	public:
		bool useFindDefCache; 

		FindPropertyInstruction(int scopeDepth, const Multiname* name, bool strict, TessaVM::BasicBlock* insertAtEnd);
		FindPropertyInstruction(const Multiname *name, uint32_t nameIndex, TessaVM::BasicBlock* insertAtEnd); 
		const Multiname* getMultiname();
		uint32_t getNameIndex();
		bool isFindDefinition();
		void print();
		void visit(TessaVisitorInterface* tessaVisitor);
		bool isStrict();
		int getScopeDepth();
		void setCacheSlot(int cacheSlot);
		int getCacheSlot();
		FindPropertyInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);

		List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc); 
	};
}