/***
 * The base class for all instructions.
 */

#ifndef __TESSAINSTRUCTIONS_TESSAINSTRICTION__
#define __TESSAINSTRUCTIONS_TESSAINSTRICTION__

namespace TessaVisitors {
	class TessaVisitorInterface;
}

namespace TessaVM {
	class BasicBlock;
}

namespace TessaInstructions {
	using namespace std;
	using namespace TessaVisitors;
	using namespace TessaVM;
	using namespace TessaTypes;

	class TessaInstruction : public TessaValue {
private: 
    static const char ReferenceChar = '%';
	
	TessaInstruction*		forwardInstruction;
	TessaVM::BasicBlock*	inBasicBlock;

protected:
	TessaInstruction* getClonedValue(TessaValue* originalValue, MMgc::GCHashtable* originalToCloneMap);

public:
	static const int UNITIALIZED_ID = -1;

	TessaInstruction(); 
	TessaInstruction(TessaVM::BasicBlock* basicBlockToInsert);

	bool		isInstruction();
	virtual bool modifiesMemory(); 
	virtual bool isCondition(); 
	virtual bool isLabel();
	virtual bool isPhi(); 
	virtual bool isArrayAccess();
	virtual bool isReturn();
	virtual bool isBranch();
	virtual bool isConditionalBranch();
	virtual bool isUnconditionalBranch();
	virtual bool isParameter();
	virtual bool isCall();
	virtual bool isCallStatic();
	virtual bool isCallVirtual();
	virtual bool isInlineMethod();
	virtual bool isConstruct();
	virtual bool isConstructProperty();
	virtual bool isCoerce();
	virtual bool isConvert();
	virtual bool modifiesScopeStack();
	virtual bool isBinary();
	virtual bool isUnary();
	virtual bool isLoadVirtualMethod();

	virtual bool isGetProperty();
	virtual bool isSetProperty();
	virtual bool isInitProperty();
	virtual bool isPropertyAccess();
	virtual bool isLocalVariableAccess();
	virtual bool isSlotAccess();
	virtual bool isNewObject();
	virtual bool isNewArray();
	virtual bool isConstantValue();
	virtual bool isSuper();
	virtual bool isConstructSuper();
	virtual bool isCallSuper();
	virtual bool isThis();
	virtual bool isTypeOf();
	virtual bool isSwitch();
	virtual bool isCase();
	virtual bool isNewActivation();
	virtual bool isArrayOfInstructions();
	virtual void print();
	virtual bool hasSideEffect();
	bool isBlockTerminator();

	TessaInstruction* clone();
	void				setInBasicBlock(TessaVM::BasicBlock* basicBlock);
	TessaVM::BasicBlock*			getInBasicBlock();

	string getPrintPrefix();
    string getOperandString();

	virtual void visit(TessaVisitorInterface* tessaVisitor) = 0;

	std::string			toString();
	TessaInstruction*	resolve();
	void				setForwardedInstruction(TessaInstruction* instruction);
	TessaInstruction*	getForwardedInstruction();
	virtual				TessaInstruction*	clone(MMgc::GC* gc, MMgc::GCHashtable* originalToCloneMap, TessaVM::BasicBlock* insertCloneAtEnd);
	virtual List<TessaValue*, LIST_GCObjects>* getOperands(MMgc::GC* gc);
	List<TessaInstruction*, LIST_GCObjects>* getOperandsAsInstructions(MMgc::GC* gc);
};

}

#endif