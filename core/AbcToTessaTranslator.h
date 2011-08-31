#ifndef __avmplus_AbcToTessaTranslator_
#define __avmplus_AbcToTessaTranslator_

namespace TessaInstructions {
	class TessaCreationApi;
	class TessaInstruction;
	class BranchInstruction;
	class ArrayOfInstructions;
	class ConstantValueInstruction;
	class CallPropertyInstruction;
	class BinaryInstruction;
	class GetSlotInstruction;
	enum TessaBinaryOp;
}

namespace TessaVM {
	class ASFunction;
	class BasicBlock;
}

namespace TessaVisitors {
	class TessaInterpreter;
}

namespace TessaConstants {
	class TessaConstantUndefined;
	class TessaConstantValue;
	class TessaConstantString;
	class TessaConstantDouble;
	class TessaConstantInteger;
	class TessaConstantNull;
	class TessaConstantBoolean;
}

using namespace MMgc;

#include "CodegenLIR.h"
#include <stdio.h>
#include <string>
#include "TessaTypeHeader.h"

namespace avmplus {
	using namespace TessaInstructions;
	using namespace TessaVisitors;
	using namespace TessaVM;
	using namespace TessaTypes;
	using namespace TessaConstants;

	class AbcToTessaTranslator : public CodeWriter, public MMgc::GCObject {
	private:
		// Data
		MethodInfo*			_methodInfo;
		TessaCreationApi*	_tessaInstructionCreator;
		TypeFactory*		_typeFactory;
		AvmCore*			core;	// Since we inherit from CodeWriter, CodeWriter expects core to have the "core" name, can't change it
		PoolObject*			_poolObject;
		MMgc::GC *gc;
		List<TessaInstruction*, avmplus::LIST_GCObjects>* _operandStack;
		List<TessaInstruction*, avmplus::LIST_GCObjects>* _returnValues;
		TessaVM::BasicBlock*	_returnBlock;
		bool					_hasMultipleReturnValues;
		Verifier* verifier;

		// Tessa Data
		BasicBlock*			_currentBasicBlock;

		// Any hashtable that can map any pointer to any pointer will work. This is the only one I found that does 
		avmplus::GCSortedMap<int, avmplus::List<BranchInstruction*, avmplus::LIST_GCObjects>*, LIST_GCObjects>* _jumpTargetToLabels;
		avmplus::GCSortedMap<int, BasicBlock*, avmplus::LIST_GCObjects>* _pcLabelMap;
		MMgc::GCHashtable	_abcPcToLabel;	
		MMgc::GCHashtable	_stateToOperandStack;	

		const byte*			_abcStartPosition;
		int					_abcCodeLength;
		int					_numberOfLocals;

		// Cache builders
		LookupCacheBuilder	_finddef_cache_builder;
		CacheBuilder<CallCache> _call_cache_builder;
		CacheBuilder<GetCache> _get_cache_builder;
		CacheBuilder<SetCache> _set_cache_builder;

		// NanoJIT allocators
		Allocator			_cacheAllocator;
		CodeMgr*			initCodeMgr(PoolObject *pool); 

	private:
		// Helper Methods
		void				modifyStackValue(uint32_t index, TessaInstruction* value); 
		TessaInstruction*	peekInstruction(uint32_t index);
		void				pushInstruction(TessaInstruction* instruction);
		TessaInstruction*	popInstruction();

		void				saveState(const FrameState* state);
		void				loadState(const FrameState* state);
		void				loadFallThroughState(const FrameState* state);
		bool				hasState(const FrameState* state);
		void				printOpcode(const avmplus::byte* pc, AbcOpcode opcode, int offset);

		void				parseBodyHeader(const uint8_t* methodInfoStartPosition);
		int					getStackSize();
		bool				isInCurrentBasicBlock(TessaInstruction* instruction);

		// Control Flow helper methods
		void				switchBasicBlocksIfPcIsNewBasicBlock(int abcPc);
		List<BranchInstruction*, avmplus::LIST_GCObjects>* getBranchesToPc(int abcPc);
		void				addBranchToPc(int abcPc, BranchInstruction* branchToTarget);
		BasicBlock*			getBasicBlockAtPc(int abcPc);

		bool				isForwardBranch(int currentPc, int targetPc);
		bool				isBackwardsBranch(int currentPc, int targetPc);
		bool				isInteger(int atom);
		//TessaTypes::Type*	getTessaType(Traits* type); 
		Type*				getType(Traits* traitsType);

		// Helper methods
		ArrayOfInstructions*	createMethodArguments(TessaInstruction* receiverInstruction, uint32_t argCount);
		Traits*					getSlotTraits(Traits* objectTraits, int slotTraits); 
		bool					isScriptObject(Traits* traits);
		void					emitNullCheck();
		void					emitKill(int localId);
		bool					canOptimizeNullCheck(const FrameState* state);

		// Constant value helpers
		ConstantFloat*	createConstantDouble(double value);
		ConstantInt*	createConstantInteger(int value);
		ConstantString*	createConstantString(Stringp value);
		ConstantUndefined*	createConstantUndefined();
		ConstantNull*		createConstantNull();
		ConstantBool*	createConstantBoolean(bool value);
		ConstantValueInstruction*	createConstantValueInstruction(ConstantValue* constantValue, TessaVM::BasicBlock* insertAtEnd);

		// Methods to early bind get and set property
		bool					isIntegerIndex(const Multiname* propertyName, Traits* indexTraits);
		bool					isUnsignedIntegerIndex(const Multiname* propertyName, Traits* indexTraits);
		bool					canUsePropertyCache(const Multiname* propertyName);
		bool					isArrayObject(Traits* objectTraits);
		bool					isObjectVector(Traits* objectTraits);
		bool					isIntVector(Traits* objectTraits);
		bool					isUIntVector(Traits* objectTraits);
		bool					isDoubleVector(Traits* objectTraits);
		bool					isIntegerType(Traits* traits);
		bool					isUnsignedIntegerType(Traits* traits);
		bool					isNumberType(Traits* traits); 
		Type*					getIndexType(const Multiname* propertyName, Traits* indexTraits); 

		bool				canPrecomputeBinaryInstruction(TessaInstruction* leftOperand, TessaInstruction* rightOperand);
		ConstantValueInstruction*	precomputeBinaryValue(TessaBinaryOp opcode, TessaInstruction* leftOperand, TessaInstruction* rightOperand);

		// Methods for each opcode
		void				emitConvertToString(const FrameState* state, int sp);
		TessaInstruction*	emitBinaryInstruction(AbcOpcode abcOpcode, Traits* resultTraits); 
		void				emitUnaryInstruction(AbcOpcode abcOpcode); 
		void				emitReturnInstruction(AbcOpcode abcOpcode);
		void				emitBooleanBranch(AbcOpcode abcOpcode, int trueTargetPc, int falseTargetPc, const FrameState* state); 
		void				emitConditionalBranch(AbcOpcode abcOpcode, int trueTargetPc, int falseTargetPc, const FrameState* state); 
		void				emitConditionalBranch(AbcOpcode abcOpcode, TessaInstruction* leftOperand, TessaInstruction* rightOperand, int trueTargetPc, int falseTargetPc, const FrameState* state); 
		void				emitUnconditionalBranch(int currentPc, int targetPc, const FrameState* state);
		void				emitConvertOrCoerceInstruction(AbcOpcode abcOpcode, int multinameIndex, Traits* resultTraits, Traits* instructionTraits);

		GetSlotInstruction*	emitGetSlot(int slot, Traits* objectTraits, Traits* slotTraits);
		void				emitSetSlot(int slot, Traits* objectTraits, Traits* slotTraits);
		void				emitGetPropertySlow(const Multiname* multiname, const FrameState* state, Binding* binding, Traits* indexTraits, Traits* objectTraits, Traits* resultTraits);
		void				emitSetPropertySlow(const Multiname* multiname, const FrameState* state, Binding* binding, Traits* objectTraits);
		void				emitInitProperty(const Multiname* multiname, const FrameState* state, TessaInstruction* namespaceInstruction, Traits* objectTraits); 
		void				emitNewObjectInstruction(uint32_t argCount);
		bool				needCoerceObject(Traits* result, Traits* in); 

		/***
		 * Inlining ActionScript Method helpers
		 */
		TessaInstruction*	earlyBindConstructCall(TessaInstruction* receiverObject, int argCount, Traits* classTraits); 
		void				emitConstruct(uint32_t argCount, Traits* classTraits);
		void				emitConstructProperty(uint32_t argCount, const Multiname* multiname, TessaInstruction* receiverInstruction, const FrameState* state, Traits* receiverType);
		void				emitConstructSuper(uint32_t argCount, MethodInfo* methodInfo, Traits* receiverType);
		CallPropertyInstruction*	emitSlotBoundCallProperty(GetSlotInstruction* functionValue, uint32_t argCount, const Multiname* propertyName, Traits* receiverType);
		void				emitCallProperty(uint32_t argCount, const Multiname* propertyName, Traits* receiverType, CallCache* cacheSlot);
		void				emitCallSuper(uint32_t multinameIndex, uint32_t argCount, Traits* baseType, const FrameState* state, Traits* receiverType);
		void				emitCallInstruction(uint32_t argCount, uintptr_t disp_id, MethodInfo* methodInfo, Traits* receiverType);
		void				emitCallInterface(const FrameState* state, uint32_t argCount, MethodInfo* methodInfo, int methodId, Traits* resultTraits);
		void				emitAbcOpcodeCall(AbcOpcode opcode, uint32_t argCount);
		void				emitNewFunction(uint32_t methodId);
		void				emitCallDynamicMethod(uint32_t argCount, Traits* receiverType);
		void				emitNewArray(uint32_t numberOfArguments);
		void				emitNextNameInstruction();
		void				emitCondition(AbcOpcode abcOpcode);
		void				emitTypeOfInstruction(AbcOpcode abcOpcode);
		void				emitGetScopeObject(int32_t index, bool isOuterScope);
		void				emitSwitch(int32_t numberOfCases, int defaultPc, int* caseTargets);
		void				emitHasNextTwo(int objectIndex, int registerIndex);

		void				emitUnconditionalBranchForFallThrough(int abcPc, BasicBlock* targetBlock, const FrameState* state);
		void				startParsingBasicBlock(BasicBlock* basicBlock, const FrameState* state);
		void				patchBranchesToTarget(avmplus::List<BranchInstruction*, avmplus::LIST_GCObjects> *unpatchedBranches, BasicBlock* target);
		void				setTypesOfLocals(BasicBlock* basicBlock, const FrameState* state); 
		void				fillReturnBlock();

	public:
		AbcToTessaTranslator(AvmCore *core, MethodInfo *methodInfo, Verifier* verifier);
		~AbcToTessaTranslator();

		void	printResults();
		ASFunction*	getMethod();

		void		recreateControlFlowGraph(const uint8_t* methodInfoStartPosition);

		// Inherited CodeWriter methods
		void write(const FrameState* state, const avmplus::byte* pc, AbcOpcode opcode, Traits *type);
        void writeOp1(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, uint32_t opd1, Traits* type);
        void writeOp2(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, uint32_t opd1, uint32_t opd2, Traits* type);
        void writeMethodCall(const FrameState* state, const avmplus::byte *pc, AbcOpcode opcode, MethodInfo*, uintptr_t disp_id, uint32_t argc, Traits* type);
        void writeNip(const FrameState* state, const avmplus::byte *pc);
        void writeCheckNull(const FrameState* state, uint32_t index);
        void writeCoerce(const FrameState* state, uint32_t index, Traits *type);
        void writePrologue(const FrameState* state, const avmplus::byte *pc);
        void writeEpilogue(const FrameState* state);
        void writeBlockStart(const FrameState* state);
		void writeOpcodeVerified(const FrameState* state, const avmplus::byte* pc, AbcOpcode opcode);
		void writeFixExceptionsAndLabels(const FrameState* state, const avmplus::byte* pc);
        void cleanup();
	};
}
#endif 
