
#include "TessaVisitors.h"

/***
 * Iterates over the instructions in a given function and tries to resolve the types of each instruction.
 * This has the side effect of setting the type of result field in each instruction.
 * At the moment, this also assumes that the function accepts parameters as Atoms only and therefore can only do
 * analysis on local variables. Every instruction shall have a value for typeOfResult. 
 * The types we analyze are currently only the ActionScript primitive types. If we don't know the type of a value, 
 * the type is set to ANY, which corresponds to an Atom.
 *
 * This means any TYPE we set in here, is a CONCRETE type that we KNOW is correct. It is not
 * a guess, the type becomes fact
 */
namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;
	using namespace avmplus;

	TypePropagator::TypePropagator(AvmCore* core, MMgc::GC* gc) {
		this->_core = core;
		this->_gc = gc;
		_typeFactory = TypeFactory::getInstance();
		_specializedIntegerBinaryInstructions = new (_gc) List<BinaryInstruction*, LIST_GCObjects>(_gc);
		_verifierTypes = new (_gc) avmplus::GCSortedMap<TessaInstruction*, Type*, avmplus::LIST_GCObjects>(_gc);
	}

	Type* TypePropagator::getVerifierType(TessaInstruction* value) {
		Type* verifierType = _verifierTypes->get(value);
		AvmAssert(verifierType != NULL);
		return verifierType;
	}

	void TypePropagator::visit(TessaInstruction* tessaInstruction) {
		TessaAssert(false);
	}

	void TypePropagator::visit(FindPropertyInstruction* findPropertyInstruction) {
		// This will always be this type
		if (findPropertyInstruction->isFindDefinition()) {
			findPropertyInstruction->setType(_typeFactory->scriptObjectType()); 
		} else {
			findPropertyInstruction->setType(_typeFactory->anyType());
		}
	}

	void TypePropagator::visit(ConstantValueInstruction* constantValueInstruction) {
		// Already have the type information when the instruction was created
	}

	void TypePropagator::visit(ArrayOfInstructions* arrayOfInstructions) {

	}

	void TypePropagator::visit(NewObjectInstruction* newObjectInstruction) {
		// Already have the type information when the instruction was created
		newObjectInstruction->setType(_typeFactory->scriptObjectType());
	}

	void TypePropagator::visit(ConstructInstruction* constructInstruction) {
	}

	void TypePropagator::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
	}

	void TypePropagator::visit(ConstructSuperInstruction* constructSuperInstruction) {
	}

	void TypePropagator::visit(CallInstruction* callInstruction) {
	}

	void TypePropagator::visit(CallVirtualInstruction* callVirtualInstruction) {
	}

	void TypePropagator::visit(CallStaticInstruction* callStaticInstruction) {
	}

	void TypePropagator::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {
	}

	void TypePropagator::visit(CallInterfaceInstruction* callInterfaceInstruction) {
	}

	void TypePropagator::visit(CallSuperInstruction* callSuperInstruction) {
	}

	void TypePropagator::visit(CallPropertyInstruction* callPropertyInstruction) {
	}

	void TypePropagator::visit(ReturnInstruction* returnInstruction) {
	}

	bool TypePropagator::canPrecomputeBinaryOperation(BinaryInstruction* binaryInstruction) {
		TessaValue* leftOperand = binaryInstruction->getLeftOperand();
		TessaValue* rightOperand = binaryInstruction->getRightOperand();

		return (leftOperand->isInteger() && rightOperand->isInteger() 
			&& leftOperand->isConstantValue() && rightOperand->isConstantValue());
	}

	/***
	 * A direct instruction is one that is resolved to it's original source.
	 * For example, parameter instructions will be resolved to their forwareded instruction.
	 * Coerce instructions get pushed through to the actual instruction they need to coerce.
	 * This goes up through any chain to find the original value, not a coerced/indirect value
	 */
	TessaInstruction* TypePropagator::getDirectInstruction(TessaInstruction* instruction) {
		instruction = instruction->resolve();
		while (instruction->isCoerce()) {
			CoerceInstruction* coerceInstruction = reinterpret_cast<CoerceInstruction*>(instruction);
			Type* coercedType = coerceInstruction->getType();
			Type* originalType = coerceInstruction->getInstructionToCoerce()->getType();

			if (originalType->isDownCast(coercedType) || coercedType->isHorizontalType(originalType)) {
				return coerceInstruction;
			}

			instruction = coerceInstruction->getInstructionToCoerce()->resolve();
		}

		return instruction;
	}

	/***
	 * A properly typed direct instruction may return an actual instruction or a COERCE to the typeToFind
	 * For example, we may have a paramter that points to a coerce instruction C. The coerce may point to 
	 * another BinaryInstruction that returns a number. The Coerce C coerces the Binary to Int.
	 * If the typeToFind is the number, we return the number instruction. If the typeToFind is an int, we return
	 * the Coerce instruction.
	 */
	TessaInstruction* TypePropagator::getProperlyTypedDirectInstruction(TessaInstruction* instruction, Type* typeToFind) {
		while ((instruction->getType() != typeToFind) || (instruction->isCoerce() || (instruction->isParameter()))) {
			if (instruction->isParameter()) {
				// Probably a method parameter
				if (instruction == instruction->resolve()) {
					return instruction;
				}

				instruction = instruction->resolve();
			} else if (instruction->isCoerce()) {
				CoerceInstruction* coerceInstruction = reinterpret_cast<CoerceInstruction*>(instruction);
				instruction = coerceInstruction->getInstructionToCoerce();

				Type* coerceType = coerceInstruction->getType();
				Type* originalType = instruction->getType();

				/***
				 * Sometimes we may specialize so much that the original type is the same as the coerce tyep
				 * then the original type goes up the type lattice, so that the coerce becomes a downcast.
				 * Keep the coerce around
				 */
				if (originalType->isDownCast(coerceType) || (originalType == coerceType)) { 
					AvmAssert(typeToFind->isCompatibleType(coerceType));
					return coerceInstruction;
				}

				// We have some crazy type stuff going on here
				//if (originalType->isHorizontalType(coerceType) && !originalType->isCompatibleType(typeToFind)) {
				//if (originalType->isHorizontalType(coerceType) && (originalType != typeToFind)) {
				if (originalType->isHorizontalType(coerceType)) { 
					return coerceInstruction;
				}

				instruction = instruction->resolve();
			} else {
				Type* instructionType = instruction->getType();
				AvmAssert(typeToFind->isCompatibleType(instructionType));
				return instruction;
			}
		}

		AvmAssert(instruction->getType() == typeToFind);
		return instruction;
	}

	bool TypePropagator::isConstantIntOne(TessaInstruction* constantValue) {
		if (constantValue->isConstantValue()) {
			ConstantValueInstruction* constantInstruction = reinterpret_cast<ConstantValueInstruction*>(constantValue);
			if (constantInstruction->isInteger()) {
				ConstantValue* constant = constantInstruction->getConstantValue();
				if (constant->isInteger()) {
					ConstantInt* constInt = reinterpret_cast<ConstantInt*>(constant);
					int integerValue = constInt->getValue();
					return integerValue == 1;
				}
			}
		}

		return false;
	}

	/***
	 * Look for the operand and see find the coerce instruction that coerces said operand into a number 
	 */
	TessaInstruction* TypePropagator::findNumericOperand(BinaryInstruction* binaryInstruction, TessaInstruction* operand) {
		AvmAssert(!operand->isNumeric());
		BasicBlock* parentBlock = operand->getInBasicBlock();
		List<TessaInstruction*, LIST_GCObjects>* instructions = parentBlock->getInstructions();
		int currentOperand = parentBlock->getInstructionIndex(operand);

		for (uint32_t i = currentOperand; i < instructions->size(); i++) {
			TessaInstruction* currentInstruction = instructions->get(i);
			if (currentInstruction->isCoerce()) {
				CoerceInstruction* coerceInstruction = reinterpret_cast<CoerceInstruction*>(currentInstruction);
				TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();
				if (instructionToCoerce == operand) {
					AvmAssert(instructionToCoerce->isNumeric());
					return operand;
				}
			}
		}

		// Oh we're really lost, manually insert a coerce instruction.
		BasicBlock* binaryBlock = binaryInstruction->getInBasicBlock();
		CoerceInstruction* coerceToNumber = new (_gc) CoerceInstruction(NULL, operand, _typeFactory->getInstance()->numberType(), binaryBlock);
		binaryBlock->removeInstruction(coerceToNumber);
		binaryBlock->addInstructionBefore(coerceToNumber, binaryInstruction);
		return coerceToNumber;
	}

	void TypePropagator::setBinaryType(BinaryInstruction* binaryInstruction, Type* strictestType) {
		TessaBinaryOp op = binaryInstruction->getOpcode();
		TessaInstruction* leftOperand = binaryInstruction->getLeftOperand()->toInstruction();
		TessaInstruction* rightOperand = binaryInstruction->getRightOperand()->toInstruction();
		Type* leftType = leftOperand->getType();
		Type* rightType = rightOperand->getType();
		Type* newBinaryType = strictestType;

		switch (op) {
			case ADD:
			{
				if (leftType->isString() || rightType->isString()) {
					newBinaryType = _typeFactory->getInstance()->stringType();
				} else if (leftType->isInteger() && (rightType->isInteger()) && (!newBinaryType->isInteger())) {
					newBinaryType = _typeFactory->getInstance()->numberType();
					/*
					if (isConstantIntOne(leftOperand) || isConstantIntOne(rightOperand)) {
						// Means we have something++
						newBinaryType = _typeFactory->getInstance()->integerType();
					} else {
						newBinaryType = _typeFactory->getInstance()->numberType();
					} 
					*/
				} 
				break;
			}
			
			case SUBTRACT:
			{
				if (leftType->isInteger() && (rightType->isInteger()) && (!newBinaryType->isInteger())) {
					newBinaryType = _typeFactory->getInstance()->numberType();
					/*
					if (isConstantIntOne(leftOperand) || isConstantIntOne(rightOperand)) {
						// Means we have something--
						newBinaryType = _typeFactory->getInstance()->integerType();
					} else {
						newBinaryType = _typeFactory->getInstance()->numberType();
					} 
					*/
					break;
				} 
				// Fall through to cases with mod, mul, div
			}
			case MOD:
			case MULTIPLY:
			case DIVIDE:
			{
				// Have to be a number
				if (!leftOperand->isNumeric()) {
					leftOperand = findNumericOperand(binaryInstruction, leftOperand);
				}

				if (!rightOperand->isNumeric()) {
					rightOperand = findNumericOperand(binaryInstruction, rightOperand);
				}

				AvmAssert(leftOperand->isNumeric());
				AvmAssert(rightOperand->isNumeric());
				newBinaryType = _typeFactory->numberType();
				break;
			}
			case BITWISE_AND: 
			case BITWISE_OR: 
			case BITWISE_XOR: 
			case BITWISE_LSH:
			case BITWISE_RSH: 
			case BITWISE_URSH:	
				newBinaryType = _typeFactory->integerType();
				break;
			default:
				// Use strictest type
				break;
		}

		TessaInstruction* leftValue = getProperlyTypedDirectInstruction(leftOperand, newBinaryType);
		TessaInstruction* rightValue = getProperlyTypedDirectInstruction(rightOperand, newBinaryType);
		AvmAssert(binaryOperandsAreCompatible(binaryInstruction->getOpcode(), newBinaryType, leftValue->getType(), rightValue->getType()));

		binaryInstruction->setType(newBinaryType);
		binaryInstruction->setLeftOperand(leftValue);
		binaryInstruction->setRightOperand(rightValue);
	}

	bool TypePropagator::binaryOperandsAreCompatible(TessaBinaryOp op, Type* binaryType, Type* leftType, Type* rightType) {
		switch (op) {
			case MOD:
			case MULTIPLY:
			case DIVIDE:
			case BITWISE_AND: 
			case BITWISE_OR: 
			case BITWISE_XOR: 
			case BITWISE_LSH:
			case BITWISE_RSH: 
			case BITWISE_URSH:	
				return leftType->isNumeric() && rightType->isNumeric();
			default:
				return binaryType->isCompatibleType(leftType) && binaryType->isCompatibleType(rightType);
		}

		AvmAssert(false);
		return false;
	}

	void TypePropagator::visit(BinaryInstruction* binaryInstruction) {
		// Don't touch
		if (isSpecializedBinaryInteger(binaryInstruction)) {
			return;
		}
		TessaValue* leftOperand = binaryInstruction->getLeftOperand();
		TessaValue* rightOperand = binaryInstruction->getRightOperand();

		if (leftOperand->isInstruction() && rightOperand->isInstruction()) {
			List<TessaInstruction*, LIST_GCObjects>* binaryOperands = binaryInstruction->getOperandsAsInstructions(_gc);
			Type* strictestType = getStrictestType(binaryInstruction, binaryOperands);
			setBinaryType(binaryInstruction, strictestType);
		} 
		// Otherwise we really can't do anything
	}

	void TypePropagator::visit(ConditionInstruction* conditionInstruction) {
		this->visit(reinterpret_cast<BinaryInstruction*>(conditionInstruction));
		conditionInstruction->setType(_typeFactory->boolType());
	}

	void TypePropagator::visit(UnaryInstruction* unaryInstruction) {
		// By specification, these are the results of the operation
		switch (unaryInstruction->getOpcode()) {
			case BITWISE_NOT:
				unaryInstruction->setType(_typeFactory->integerType());
				break;
			case NEGATE:
				unaryInstruction->setType(_typeFactory->numberType());
				break;
			case NOT:
				unaryInstruction->setType(_typeFactory->boolType());
				break;
		}
	}

	void TypePropagator::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {

	}

	void TypePropagator::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {

	}

	void TypePropagator::visit(PhiInstruction* phiInstruction) {
		int numberOfOperands = phiInstruction->numberOfOperands();
		Type* phiType = getStrictestType(phiInstruction, phiInstruction->getOperandsAsInstructions(_gc));
		phiInstruction->setType(phiType);

		for (int i = 0; i < numberOfOperands; i++) {
			BasicBlock* incomingBlock = phiInstruction->getIncomingEdge(i);
			TessaInstruction* incomingOperand = phiInstruction->getOperand(incomingBlock);
			TessaInstruction* properlyTypedValue = getProperlyTypedDirectInstruction(incomingOperand, phiType);
			Type* operandType = properlyTypedValue->getType();
			AvmAssert(phiType->isCompatibleType(operandType)); 

			if ((properlyTypedValue != phiInstruction) && (properlyTypedValue->getInBasicBlock() == incomingBlock)) {
				phiInstruction->setOperand(incomingBlock, properlyTypedValue);
			} else {
				AvmAssert(incomingOperand->getInBasicBlock() == incomingBlock);
				//AvmAssert(incomingOperand->isParameter());
					//AvmAssert(incomingOperand->resolve() == phiInstruction);
				//TessaInstruction* resolvedInstruction = incomingOperand->resolve();
				//AvmAssert(resolvedInstruction == properlyTypedValue);
			}
			
		}
	}

	void TypePropagator::visit(ParameterInstruction* parameterInstruction) {
		TessaInstruction* resolvedInstruction = parameterInstruction->resolve();
		parameterInstruction->setType(resolvedInstruction->getType());
	}

	/***
	 * The ABC verifier automatically assumes that the result of an addition, subtract, and multiply
	 * are the number type. A conversion is then required to convert it back to an integer when assigning
	 * those operations to an integer variable. 
	 *
	 * It's much faster to just do the binary operation as an integer, which we try to convert here
	 */
	bool TypePropagator::isCoercingToIntegerType(TessaValue* value) {
		if (value->isInstruction()) { 
			TessaInstruction* instruction= static_cast<TessaInstruction*>(value);
			if (instruction->isCoerce()) {
				CoerceInstruction* coerceInstruction = static_cast<CoerceInstruction*>(instruction);
				TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();

				Type* originalType = instructionToCoerce->getType();
				Type* convertedType = coerceInstruction ->getType();
				if ((originalType->isInteger()) && (convertedType->isNumber())) { 
					return true;
				}
			} else if (instruction->getType()->isInteger()) { 
				return true;
			}
		}

		return false;
	}

	bool TypePropagator::canSpecializeIntegerBinaryOperation(CoerceInstruction* coerceInstruction) {
		TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();
		Type* typeOfOriginal = instructionToCoerce->getType();
		Type* typeToConvert = coerceInstruction->getType();

		if (instructionToCoerce->isBinary() && (typeOfOriginal->isNumber()) && (typeToConvert->isInteger())) {
			BinaryInstruction* binaryInstruction = static_cast<BinaryInstruction*>(instructionToCoerce);
			TessaValue* leftOperand = binaryInstruction->getLeftOperand();
			TessaValue* rightOperand = binaryInstruction->getRightOperand();

			if (isCoercingToIntegerType(leftOperand) && isCoercingToIntegerType(rightOperand)) {
				switch (binaryInstruction->getOpcode()) {
					case ADD:
					case MULTIPLY:
					case SUBTRACT:
						return true;
					default:
						break;
				}
			}
		}

		return false;
	}

	TessaValue* TypePropagator::getIntegerOperandToBinary(TessaValue* binaryOperand) {
		if (binaryOperand->getType()->isInteger()) {
			return binaryOperand;
		} else if (binaryOperand->isInstruction()) {
			TessaInstruction* binaryInstruction = static_cast<TessaInstruction*>(binaryOperand);
			if (binaryInstruction->isCoerce()) {
				CoerceInstruction* coerceInstruction = static_cast<CoerceInstruction*>(binaryOperand);
				TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();

				Type* originalType = instructionToCoerce->getType();
				Type* convertedType = coerceInstruction->getType();
				
				TessaAssert(originalType->isInteger());
				TessaAssert(convertedType->isNumber()); 
				return instructionToCoerce;
			} else {
				return binaryOperand;
			}
		} else {
			TessaAssertMessage(false, "Specializing binary operation with integers, but no integer operand");
			return binaryOperand;
		}
	}

	bool TypePropagator::isSpecializedBinaryInteger(BinaryInstruction* binaryInstruction) {
		return _specializedIntegerBinaryInstructions->contains(binaryInstruction);
	}

	void TypePropagator::specializeIntegerBinaryOperation(CoerceInstruction* coerceInstruction) {
		TessaAssert(coerceInstruction->getInstructionToCoerce()->isBinary());
		BinaryInstruction* binaryInstruction = static_cast<BinaryInstruction*>(coerceInstruction->getInstructionToCoerce());

		TessaValue* leftIntegerOperand = getIntegerOperandToBinary(binaryInstruction->getLeftOperand());
		TessaValue* rightIntegerOperand = getIntegerOperandToBinary(binaryInstruction->getRightOperand());

		TessaAssert(leftIntegerOperand->getType()->isInteger()); 
		TessaAssert(rightIntegerOperand->getType()->isInteger()); 
		binaryInstruction->setLeftOperand(leftIntegerOperand);
		binaryInstruction->setRightOperand(rightIntegerOperand);
		binaryInstruction->setType(_typeFactory->integerType());
		_specializedIntegerBinaryInstructions->add(binaryInstruction);
	}

	void TypePropagator::visit(ConvertInstruction* convertInstruction) {
		if (canSpecializeIntegerBinaryOperation(convertInstruction)) {
			specializeIntegerBinaryOperation(convertInstruction);
		}
	}

	void TypePropagator::visit(CoerceInstruction* coerceInstruction) {
		if (canSpecializeIntegerBinaryOperation(coerceInstruction)) {
			specializeIntegerBinaryOperation(coerceInstruction);
		}
	}

	bool TypePropagator::optimizeArrayGetAndSetProperty(PropertyAccessInstruction* propertyAccesss) {
		TessaInstruction* receiverInstruction = propertyAccesss->getReceiverInstruction();
		if (receiverInstruction->isCoerce()) {
			CoerceInstruction* coerceInstruction = reinterpret_cast<CoerceInstruction*>(receiverInstruction);
			TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();
			Type* instructionType = instructionToCoerce->getType();

			if (instructionToCoerce->isConstruct() && instructionType->isArray()) {
				propertyAccesss->setReceiverInstruction(instructionToCoerce);
				return true;
			}
		}

		return false;
	}

	bool TypePropagator::optimizeIntVectorGetAndSetProperty(PropertyAccessInstruction* propertyAccess) {
		Type* receiverType = propertyAccess->getReceiverInstruction()->getType();
		if (receiverType->isVector()) {
			if (propertyAccess->getPropertyKey() != NULL) {
				TessaTypes::VectorType* vectorType = reinterpret_cast<TessaTypes::VectorType*>(receiverType);
				TessaTypes::Type* elementType = vectorType->getElementType();
				bool isIntVector = elementType->isInteger() || elementType->isUnsignedInt();
				propertyAccess->setType(elementType);
				return true;
			}
		}

		return false;
	}

	void TypePropagator::visit(GetPropertyInstruction* getPropertyInstruction) {
		if (!optimizeArrayGetAndSetProperty(getPropertyInstruction)) {
			optimizeIntVectorGetAndSetProperty(getPropertyInstruction);
		}
	}

	void TypePropagator::visit(SetPropertyInstruction* setPropertyInstruction) {
		if (!optimizeArrayGetAndSetProperty(setPropertyInstruction)) {
			optimizeIntVectorGetAndSetProperty(setPropertyInstruction);
		}
	}

	void TypePropagator::visit(InitPropertyInstruction* initPropertyInstruction) {
	}

	void TypePropagator::visit(GetSlotInstruction* getSlotInstruction) {
	}

	void TypePropagator::visit(SetSlotInstruction* setSlotInstruction) {
	}

	void TypePropagator::visit(NewArrayInstruction* newArrayInstruction) {
		newArrayInstruction->setType(_typeFactory->getInstance()->anyArrayType());
	}

	void TypePropagator::visit(NextNameInstruction* nextNameInstruction) {
	}

	void TypePropagator::visit(PushScopeInstruction* pushScopeInstruction) {
	}

	void TypePropagator::visit(PopScopeInstruction* popScopeInstruction) {
	}

	void TypePropagator::visit(WithInstruction* withInstruction) {
	}

	void TypePropagator::visit(TypeOfInstruction* typeOfInstruction) {
		typeOfInstruction->setType(_typeFactory->anyType());
	}

	void TypePropagator::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
		getScopeObjectInstruction->setType(_typeFactory->scriptObjectType());
	}

	void TypePropagator::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
		getGlobalScopeInstruction->setType(_typeFactory->scriptObjectType());
	}

	void TypePropagator::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
	}

	void TypePropagator::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
	}

	void TypePropagator::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesRegisterInstruction) {
	}

	void TypePropagator::visit(SwitchInstruction* switchInstruction) {
	}

	void TypePropagator::visit(CaseInstruction* caseInstruction) {
	}

	void TypePropagator::visit(NewActivationInstruction* newActivationInstruction) {
	}

	void TypePropagator::visit(InlineBeginInstruction* inlineBeginInstruction) {
	}


	/***
	 * Go through all the operands and find the "strictest" type.
	 * Strictness is defined as the type at which all operands can be coerced to.
	 * For example, int and number, the strictest type is number since integer is a subtype of number.
	 * Worst case scenario is the any type.
	 * This in the formal term, is going UP the type lattice
	 */
	Type* TypePropagator::getStrictestType(TessaInstruction* originalInstruction, List<TessaInstruction*, LIST_GCObjects>* operands) {
		AvmAssert(operands->size() >= 1);
		Type* strictestType = getDirectInstruction(operands->get(0))->getType();

		for (uint32_t i = 0; i < operands->size(); i++) {
			TessaInstruction* currentOperand = operands->get(i);
			Type* currentType = currentOperand->getType();

			TessaInstruction* directInstruction = getDirectInstruction(currentOperand);
			Type* directType = directInstruction->getType();

			if (directInstruction == originalInstruction) {
				AvmAssert(originalInstruction->isPhi());
			} else {
				strictestType = strictestType->getStrictestType(directType);
			}
		}

		return strictestType;
	}

	/***
	 * Finds the lowest type on the type lattice, which is the inverse of getStrictestType
	 */
	Type* TypePropagator::getMostConcreteType(List<TessaValue*, LIST_GCObjects>* operands) {
		AvmAssert(operands != NULL);
		Type* mostConcreteType = _typeFactory->getInstance()->anyType();
		//operands->get(0)->getType();

		for (uint32_t i = 0; i < operands->size(); i++) {
			TessaValue* currentOperand = operands->get(i);
			Type* directType = currentOperand->getType();

			if (currentOperand->isInstruction()) {
				TessaInstruction* instructionOperand = reinterpret_cast<TessaInstruction*>(currentOperand);
				TessaInstruction* directInstruction = getDirectInstruction(instructionOperand);
				directType = directInstruction->getType();
			}

			mostConcreteType = mostConcreteType->getMostConcreteType(directType);
		}

		return mostConcreteType;
	}

	void TypePropagator::propagateTypes(ASFunction* function) {
		_isDone = true;
		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocksInReversePostOrder();
		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();

			for (uint32_t i = 0; i < instructions->size(); i++) {
				TessaInstruction* currentInstruction = instructions->get(i);
				Type* originalType = currentInstruction->getType();
				//currentInstruction->print();

				currentInstruction->visit(this);
				Type* newType = currentInstruction->getType();
				//currentInstruction->print();
				
				if (newType != originalType) {
					_isDone = false;
				}
			}
		}
	}

	/***
	 * Only instructions that operate on local variables can change their types. Or instructions that exist on the stack.
	 * Values that exist in the heap, field variables and get/set property, cannot change
	 */
	bool TypePropagator::canChangeTypes(TessaInstruction* instruction) {
		bool immuneInstruction = instruction->isCoerce() || instruction->isPropertyAccess() || instruction->isSlotAccess() || instruction->isReturn() ||
			instruction->isNewActivation() || instruction->isNewArray() || instruction->isCall() || instruction->isArrayOfInstructions() || instruction->isBranch();
		return !immuneInstruction;
	}

	void TypePropagator::initializeTypes(ASFunction* function) {
		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocksInReversePostOrder();
		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();

			for (uint32_t i = 0; i < instructions->size(); i++) {
				TessaInstruction* currentInstruction = instructions->get(i);
				Type* verifierType = currentInstruction->getType();
				_verifierTypes->put(currentInstruction, verifierType);
				//currentInstruction->print();
				
				Type* currentType = getMostConcreteType(currentInstruction->getOperands(_gc));

				// Can't do anything about field variables
				if (canChangeTypes(currentInstruction)) {
					currentInstruction->setType(currentType);
				} 

				//currentInstruction->print();
				//printf("Initialized\n\n");
			}
		}
	}

	void TypePropagator::propagateTypeInformation(ASFunction* function) {
#ifdef DEBUG
		if (_core->config.tessaVerbose) {
			printf("\n\n==== Type Propagation ====\n");
		}
#endif

		_isDone = false;
		initializeTypes(function);
		while (!_isDone) {
			propagateTypes(function);
		}

#ifdef DEBUG
		if (_core->config.tessaVerbose) {
			function->printResults();
		}
#endif
	}

	/*** 
	* Can only have an accurate phi type if ALL the phi operands are the same type.
	* Otherwise return any type
	*/
	Type* TypePropagator::conservativePhiType(PhiInstruction* phiInstruction) {
		int numberOfOperands = phiInstruction->numberOfOperands();
		BasicBlock* firstBlock = phiInstruction->getIncomingEdge(0);
		TessaInstruction* firstOperand = phiInstruction->getOperand(firstBlock);
		Type* initialType = firstOperand->getType();

		for (int i = 0; i < numberOfOperands; i++) {
			BasicBlock* comingFrom = phiInstruction->getIncomingEdge(i);
			TessaInstruction* operand = phiInstruction->getOperand(comingFrom);
			Type* operandType = operand->getType();
			if (operandType != initialType) {
				return _typeFactory->anyType();
			}
		}

		return initialType;
	}

	/***
	 * Leave the verifier types as is. Only set the type for phis
	 */
	void TypePropagator::resolvePhiTypesOnly(ASFunction* function) {
		List<BasicBlock*, avmplus::LIST_GCObjects>* basicBlocks = function->getBasicBlocksInReversePostOrder();
		for (uint32_t basicBlockIndex = 0; basicBlockIndex < basicBlocks->size(); basicBlockIndex++) {
			BasicBlock* basicBlock = basicBlocks->get(basicBlockIndex);
			List<PhiInstruction*, avmplus::LIST_GCObjects>* phiInstructions = basicBlock->getPhiInstructions();

			for (uint32_t i = 0; i < phiInstructions->size(); i++) {
				PhiInstruction* currentInstruction = phiInstructions->get(i);
				Type* phiType = conservativePhiType(currentInstruction);
				currentInstruction->setType(phiType);
			}
		}

#ifdef DEBUG
		if (_core->config.tessaVerbose) {
			function->printResults();
		}
#endif
	}
}