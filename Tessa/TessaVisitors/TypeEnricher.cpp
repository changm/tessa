#include "TessaVisitors.h"

namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;
	using namespace avmplus;

	TypeEnricher::TypeEnricher(AvmCore* core, MMgc::GC* gc) {
		this->core = core;
		this->gc = gc;
		_typeFactory = TypeFactory::getInstance();
	}

	Type* TypeEnricher::getOriginalType(TessaValue* tessaValue) {
		if (tessaValue->isInstruction()) {
			TessaInstruction* instruction = static_cast<TessaInstruction*>(tessaValue);
			if (instruction->isConvert()) {
				ConvertInstruction* convertInstruction = dynamic_cast<ConvertInstruction*>(tessaValue);
				return convertInstruction->getInstructionToCoerce()->getType();
			}
		} 

		return tessaValue->getType();
	}

	void TypeEnricher::visit(TessaInstruction* tessaInstruction) {
		TessaAssert(false);
	}

	void TypeEnricher::visit(FindPropertyInstruction* findPropertyInstruction) {
	}

	void TypeEnricher::visit(ConstantValueInstruction* constantValueInstruction) {
		// Already have the type information when the instruction was created
	}

	void TypeEnricher::visit(ArrayOfInstructions* arrayOfInstructions) {

	}

	void TypeEnricher::visit(NewObjectInstruction* newObjectInstruction) {
	}

	void TypeEnricher::visit(ConstructInstruction* constructInstruction) {
	}

	void TypeEnricher::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
	}

	void TypeEnricher::visit(ConstructSuperInstruction* constructSuperInstruction) {
	}

	void TypeEnricher::visit(CallInstruction* callInstruction) {
	}

	void TypeEnricher::visit(CallVirtualInstruction* callVirtualInstruction) {
	}

	void TypeEnricher::visit(CallStaticInstruction* callStaticInstruction) {
	}

	void TypeEnricher::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {
	}

	void TypeEnricher::visit(CallInterfaceInstruction* callInterfaceInstruction) {
	}

	void TypeEnricher::visit(CallSuperInstruction* callSuperInstruction) {
	}

	void TypeEnricher::visit(CallPropertyInstruction* callPropertyInstruction) {
	}

	void TypeEnricher::visit(ReturnInstruction* returnInstruction) {
	}

	void TypeEnricher::visit(BinaryInstruction* binaryInstruction) {
	}

	void TypeEnricher::visit(ConditionInstruction* conditionInstruction) {
		/*
		Type* leftType = getOriginalType(conditionInstruction->getLeftOperand());
		Type* rightType = getOriginalType(conditionInstruction->getRightOperand()); 

		if (leftType != rightType) {
			if (leftType->isInteger() || rightType->isInteger()) {
				AvmAssert(false);
			} else if (leftType->isNumber() || rightType->isNumber()) {
				AvmAssert(false);
			} else if (leftType->isProbabilisticType() || rightType->isProbabilisticType()) {
				AvmAssert(false);
			}
		} else {
			AvmAssert(leftType->isAnyType());
		}
		*/
	}

	void TypeEnricher::visit(UnaryInstruction* unaryInstruction) {
	}

	void TypeEnricher::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {

	}

	void TypeEnricher::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {

	}

	void TypeEnricher::visit(PhiInstruction* phiInstruction) {
	}

	void TypeEnricher::visit(ParameterInstruction* parameterInstruction) {
	}

	void TypeEnricher::visit(ConvertInstruction* convertInstruction) {
	}

	void TypeEnricher::visit(CoerceInstruction* coerceInstruction) {
	}

	void TypeEnricher::visit(GetPropertyInstruction* getPropertyInstruction) {
	}

	void TypeEnricher::visit(SetPropertyInstruction* setPropertyInstruction) {
	}

	void TypeEnricher::visit(InitPropertyInstruction* initPropertyInstruction) {
	}

	void TypeEnricher::visit(GetSlotInstruction* getSlotInstruction) {
	}

	void TypeEnricher::visit(SetSlotInstruction* setSlotInstruction) {
	}

	void TypeEnricher::visit(NewArrayInstruction* newArrayInstruction) {
		// Already have the type information when the instruction was created
	}

	void TypeEnricher::visit(NextNameInstruction* nextNameInstruction) {
	}

	void TypeEnricher::visit(PushScopeInstruction* pushScopeInstruction) {
	}

	void TypeEnricher::visit(PopScopeInstruction* popScopeInstruction) {
	}

	void TypeEnricher::visit(WithInstruction* withInstruction) {
	}

	void TypeEnricher::visit(TypeOfInstruction* typeOfInstruction) {
	}

	void TypeEnricher::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
	}

	void TypeEnricher::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
	}

	void TypeEnricher::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
	}

	void TypeEnricher::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
	}

	void TypeEnricher::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesRegisterInstruction) {
	}

	void TypeEnricher::visit(SwitchInstruction* switchInstruction) {
	}

	void TypeEnricher::visit(CaseInstruction* caseInstruction) {
	}

	void TypeEnricher::visit(NewActivationInstruction* newActivationInstruction) {
	}

	void TypeEnricher::visit(InlineBeginInstruction* inlineBeginInstruction) {
	}

	void TypeEnricher::enrichTypes(ASFunction* function) {
#ifdef DEBUG
		if (core->config.tessaVerbose) {
			printf("\n\n==== Type Enrichment ====\n");
		}
#endif

		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocks();
		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();

			for (uint32_t instructionIndex = 0; instructionIndex < instructions->size(); instructionIndex++) {
				TessaInstruction* currentInstruction = instructions->get(instructionIndex);
				currentInstruction->visit(this);
			}
		}

#ifdef DEBUG
		if (core->config.tessaVerbose) {
			function->printResults();
		}
#endif
	}
}