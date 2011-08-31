
#include "TessaVisitors.h"

/***
 * Much of the interpretation implementation is actually just calling into the VM.
 * For implemtnation details, look at Interpreter.cpp which is the ABC interpreter.
 * The Tessa Interpreter only works on atoms. Each instruction must purely operate on
 * and convert the result back to an atom, EVEN if an operand is a primitive unboxed value.
 */
namespace TessaVisitors {
	using namespace TessaVM;
	using namespace TessaInstructions;
	using namespace avmplus;

	TessaInterpreter::TessaInterpreter(AvmCore* core, PoolObject* poolObject) 
		: values(core->gc, 128)
	{
		this->core = core;
		this->gc = core->gc;
		this->poolObject = poolObject;
		scopeStack = new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);
	}

	TessaInterpreter::~TessaInterpreter() {
	}

	void TessaInterpreter::putOperand(TessaValue* tessaValue, Atom value) {
		TessaAssert(tessaValue!= NULL);
		TessaAssert(value != 0);
		values.add((Atom)tessaValue, value);
	}

	Atom TessaInterpreter::getOperand(TessaValue* tessaValue) {
		Atom result = values.get((Atom)tessaValue);
		TessaAssert(result != 0);
		return result;
	}

	void TessaInterpreter::jumpToTarget(BasicBlock* targetBlock) {
		flowingFromBasicBlock = currentBasicBlockId;
		int targetLabelId = targetBlock->getBasicBlockId();
		branchTable[currentBasicBlockId] = 1;

		this->targetBlock = targetLabelId;
		this->currentBasicBlockId = targetLabelId;
	}

	void TessaInterpreter::interpretBasicBlock(BasicBlock* basicBlock) {
		currentBasicBlockId = basicBlock->getBasicBlockId();
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = basicBlock->getInstructions();

		for (uint32_t instructionIndex = 0; instructionIndex < instructions->size(); instructionIndex++) {
			TessaInstruction* currentInstruction = instructions->get(instructionIndex);
			if (core->config.tessaVerbose) {
				printf("Interpreting: ");
				currentInstruction->print();
			}

			currentInstruction->visit(this);

			if (currentInstruction->isBranch() || currentInstruction->isReturn() || currentInstruction->isSwitch()) {
				return;
			}
		}
	}

	void TessaInterpreter::initOptionalArguments(BasicBlock* firstBasicBlock) {
		TessaAssert(firstBasicBlock->getBasicBlockId() == 0);
		MethodInfo* methodInfo = env->method;
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = firstBasicBlock->getInstructions();

		if (methodInfo->hasOptional()) {
			int32_t optionalCount = methodSignature->optional_count();
			int32_t requiredCount = methodSignature->param_count() - optionalCount;

			/***
			 * Assume function:
			 * function testFunction(firstParameter, otherParameter = default)
			 * Arguments are laid out like this:
			 *   0     1               2       
			 * "this"  firstParameter  otherParameter
			 *
			 * Required count will be 1, optional count will be 1
			 * To actually get the default value, need to offset optionalCount + requiredCount which will get us to the optinal arguments
			 */
			for (int optionalIndex = 0; optionalIndex < optionalCount; optionalIndex++) {
				int parameterIndex = optionalIndex + requiredCount + 1;	// + 1. Optional starts at 0, but we need to get the param AFTER all the required parameters
				TessaInstruction* parameter = instructions->get(parameterIndex);
				Atom defaultValue = methodSignature->getDefaultValue(optionalIndex);	// MethodSignature auto corrects the index
				TessaAssert(parameter->isParameter());
				putOperand(parameter, defaultValue);
			}
		}
	}

	/***
	 * Argument data structures are kind of weird. Even if there is no explicit default value in AS source,
	 * the method signature assumes that the default value for a variable is undefined.
	 * Therefore, we have to initialize all arguments to undefined, THEN overwrite them
	 * with the real passed in values.
	 */
	void TessaInterpreter::mapParametersToArguments(BasicBlock* firstBasicBlock, int argCount, Atom* arguments) {
		TessaAssert(firstBasicBlock->getBasicBlockId() == 0);
		List<TessaInstruction*, avmplus::LIST_GCObjects>* instructions = firstBasicBlock->getInstructions();
		initOptionalArguments(firstBasicBlock);
		int i = 1;

		TessaAssert((int) firstBasicBlock->getParameterInstructions()->size() >= argCount);
		for (i = 0; i <= argCount; i++) {
			TessaInstruction* parameter = instructions->get(i);
			TessaAssert(parameter->isParameter());
			putOperand(parameter, arguments[i]);
		}

		/**
		 * Fill out the "REST" arguments AFTER the normal parameter instructions
		 * Also, we have to explicitly remap some the REST parameter AFTER we put in the "real" passed in arguments
		 * Ask the semantics of the interpreter why this is
		 */
		MethodInfo* methodInfo = env->method;
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		if (methodInfo->needRest()) {
			int parameterCount = methodSignature->param_count();
			TessaInstruction* parameter = instructions->get(parameterCount + 1);
			TessaAssert(parameter->isParameter());
			putOperand(parameter, env->createRest(arguments, argCount)->atom());
        } else if (methodInfo->needArguments()) {
			TessaAssert(false);
            // create arguments using atomv[1..argc].
            // Even tho E3 says create an Object, E4 says create an Array so thats what we will do.
            //framep[param_count+1] = env->createArguments(_atomv, arguments_argc)->atom();
        }

		/***
		 * TODO: Fix so that we coerce arguments to their declared types, then rebox them
		 * for typed methods
		 */

		TessaAssert(i == argCount + 1);
	}

	
	/***
	 * Copied from interpreter.cpp. If that changes, change it here too
	 */
	Traits* TessaInterpreter::getTraits(const Multiname* name, PoolObject* pool, Toplevel* toplevel, AvmCore* core) {
        Traits* traits = pool->getTraits(*name, toplevel);
		if( name->isParameterizedType() ) {
			const Multiname* param_name = pool->precomputedMultiname(name->getTypeParameter());
            Traits* param_traits = name->getTypeParameter() ? getTraits(param_name, pool, toplevel, core) : NULL;
            traits = pool->resolveParameterizedType(toplevel, traits, param_traits);
        }

        return traits;
    }

	Atom TessaInterpreter::interpret(MethodEnv* env, int32_t argc, Atom* args, ASFunction* function) {
		this->env = env;
		this->currentFunction = function;

		List<BasicBlock*, avmplus::LIST_GCObjects> *basicBlocks = function->getBasicBlocks();
		this->branchTable = new int[basicBlocks->size()];
		memset(branchTable, 0, sizeof(int) * basicBlocks->size());

		isDone = false;
		targetBlock = 0;
		mapParametersToArguments(basicBlocks->get(0), argc, args);

		while (!isDone) {
			interpretBasicBlock(basicBlocks->get(targetBlock));
		}

		return _returnValue;
	}

	Atom* TessaInterpreter::createArguments(ArrayOfInstructions* arguments) {
		return createArguments(arguments->getInstructions());
	}

	Atom* TessaInterpreter::createArguments(List<TessaInstruction*, avmplus::LIST_GCObjects>* arguments) {
		uint32_t argCount = arguments->size();
		Atom* atomArguments = new Atom[argCount];
		for (uint32_t i = 0; i < argCount; i++) {
			atomArguments[i] = getOperand(arguments->get(i));
		}

		return atomArguments;
	}

	Atom TessaInterpreter::atomPointer(void* pointer) {
		return ((Atom)(uintptr_t(pointer) & ~7));
	}

	bool TessaInterpreter::isInteger(int atom) {
		return (((atom) & 7) == kIntptrType);
	}

	bool TessaInterpreter::atomCanBeUint32(Atom atom)
    {
        AvmAssert(atomIsIntptr(atom));
#ifdef AVMPLUS_64BIT
        intptr_t const i = atomGetIntptr(atom);
        uint32_t const u32 = uint32_t(i);
        return i == intptr_t(u32);
#else
        // int atoms always fit in uint32 on 32-bit, if they are >= 0
        return atom >= 0;
#endif
    }

	uint32_t TessaInterpreter::toUint32(Atom value) {
		return uint32_t(atomGetIntptr(value));
	}

	/***
	 * Scope instructions
	 */
	void TessaInterpreter::pushScope(TessaInstruction* scopeObject) {
		scopeStack->add(scopeObject);
	}

	TessaInstruction* TessaInterpreter::popScope() {
		return scopeStack->removeLast();
	}

	int TessaInterpreter::getScopeDepth() {
		return scopeStack->size();
	}

	/***
	 * Gets an object off the stack. Index is counting from the BOTTOM of the stack
	 * This is inverse of peek from the operand stack because CodegenLIR does this
	 * The operand stack peek comes from the TOP of the stack
	 * The scope stack peek looks from teh BOTTOM of the stack
	 */
	TessaInstruction* TessaInterpreter::peekScope(uint32_t index) {
		//return scopeStack->get(scopeStack->size() - index);
		return scopeStack->get(index);
	}

	/***
	 * Begin instruction visitor methods
	 */
	void TessaInterpreter::visit(TessaInstruction* tessaInstruction) {
		TessaAssertMessage(false, "Should not interpret generic instruction");
	}

	void TessaInterpreter::visit(ReturnInstruction* returnInstruction) {
		if (returnInstruction->getIsVoidReturn()) {
			_returnValue = avmplus::AtomConstants::undefinedAtom;
		} else {
			_returnValue = getOperand(returnInstruction->getValueToReturn());
		}

		putOperand(returnInstruction, _returnValue);
		isDone = true;
	}

	Atom TessaInterpreter::doDoubleBinaryOp(TessaBinaryOp opcode, Atom leftOperand, Atom rightOperand) {
		double leftDouble = core->number(leftOperand);
		double rightDouble = core->number(rightOperand);
		double resultDouble;

		switch (opcode) {
			case SUBTRACT:
				resultDouble = leftDouble - rightDouble;
				break;
			case MULTIPLY:
				resultDouble = leftDouble * rightDouble;
				break;
			case DIVIDE:
				resultDouble = leftDouble / rightDouble;
				break;
			case MOD:
				resultDouble = MathUtils::mod(leftDouble, rightDouble);
				break;
			default:
				TessaAssertMessage(false, "Unknown double binary instruction");
				break;
		}

		return core->doubleToAtom(resultDouble);
	}

	Atom TessaInterpreter::interpretBitwiseOp(TessaBinaryOp opcode, Atom leftOperand, Atom rightOperand) {
		int leftInteger = AvmCore::integer(leftOperand);
		int rightInteger = AvmCore::integer(rightOperand);
		int resultInteger;

		switch (opcode) {
			case BITWISE_LSH:
				resultInteger = leftInteger << rightInteger;
				break;
			case BITWISE_RSH:
				resultInteger = leftInteger >> rightInteger;
				break;
			case BITWISE_URSH: 
			{
			    uint32_t u1 = AvmCore::toUInt32(leftOperand);
                uint32_t u2 = AvmCore::toUInt32(rightOperand);
				return core->uintToAtom( (uint32_t)(u1 >> (u2 & 0x1F)) );
				break;
		   }
			case BITWISE_AND:
				resultInteger = leftInteger & rightInteger;
				break;
			case BITWISE_OR:
				resultInteger = leftInteger | rightInteger;
				break;
			case BITWISE_XOR:
				resultInteger = leftInteger ^ rightInteger;
				break;
			default:
				TessaAssertMessage(false, "Unknown bitwise operation");
				break;
		}

		return core->intToAtom(resultInteger);
	}

	void TessaInterpreter::visit(BinaryInstruction* binaryInstruction) {
		TessaValue* leftOperand = binaryInstruction->getLeftOperand();
		TessaValue* rightOperand = binaryInstruction->getRightOperand();
		TessaBinaryOp opcode = binaryInstruction->getOpcode();

		Atom leftOperandAtom = getOperand(leftOperand);
		Atom rightOperandAtom = getOperand(rightOperand);
		Atom result;

		switch (opcode) {
			case BITWISE_LSH:
			case BITWISE_RSH:
			case BITWISE_URSH:
			case BITWISE_OR:
			case BITWISE_XOR:
			case BITWISE_AND:
			{
				result = interpretBitwiseOp(opcode, leftOperandAtom, rightOperandAtom);
				break;
			}
			case ADD:
			{
				result = op_add(core, leftOperandAtom, rightOperandAtom);
				break;
			}
			case SUBTRACT:
			case MULTIPLY:
			case DIVIDE:
			case MOD:
			{
				result = doDoubleBinaryOp(opcode, leftOperandAtom, rightOperandAtom);
				break;
			}
			default:
			{
				TessaAssertMessage(false, "Unknown binary instruction");
				break;
			}
		}

		putOperand(binaryInstruction, result);
	}

	Atom TessaInterpreter::inverseBooleanAtom(Atom atom) {
		if (atom == trueAtom) {
			return falseAtom;
		} else {
			// May still get some unknown kind of atom
			TessaAssert(atom == falseAtom);
			return trueAtom;
		}
	}

	void TessaInterpreter::visit(ConditionInstruction* conditionInstruction) {
		Atom leftOperand = getOperand(conditionInstruction->getLeftOperand());
		Atom rightOperand = getOperand(conditionInstruction->getRightOperand());
		TessaBinaryOp opcode = conditionInstruction->getOpcode();

		Atom result = falseAtom;

		/****
		 * Core->compare() returns true if less than.
		 */
		switch (opcode) {
			case EQUAL: 
			{
				result = core->equals(leftOperand, rightOperand);
				break;
			}
			case NOT_EQUAL:
			{
				result = core->equals(leftOperand, rightOperand);
				result = inverseBooleanAtom(result);
				break;
			}
			
			case NOT_GREATER_EQUAL_THAN:
			case LESS_THAN:
			{
				result = core->compare(leftOperand, rightOperand);
				break;
			}
			case GREATER_EQUAL_THAN:
			case NOT_LESS_THAN:
			{
				result = core->compare(leftOperand, rightOperand);
				result = inverseBooleanAtom(result);
				break;
			}
			case NOT_GREATER_THAN:
			case LESS_EQUAL_THAN: 
			{
				Atom lessThanResult = core->compare(leftOperand, rightOperand);	// true if less than
				Atom equalResult = core->equals(leftOperand, rightOperand); // true if equal

				if ((lessThanResult == trueAtom) || (equalResult == trueAtom)) {
					result = trueAtom;
				} else {
					result = falseAtom;
				}
				break;
			}
			case NOT_LESS_EQUAL_THAN:
			case GREATER_THAN:
			{
				Atom lessThanResult = core->compare(leftOperand, rightOperand);
				Atom equalResult = core->equals(leftOperand, rightOperand); // true if equal

				if ((lessThanResult == trueAtom) || (equalResult == trueAtom)) {
					result = falseAtom;
				} else {
					result = trueAtom;
				}
				break;
			}
			case IFFALSE:
			{
				TessaAssert(rightOperand == falseAtom);
				result = core->equals(leftOperand, rightOperand);
				break;
			}
			case IFTRUE:
			{
				TessaAssert(rightOperand == trueAtom);
				result = core->equals(leftOperand, rightOperand);
				break;
			}
			case STRICT_NOT_EQUAL:
			{
				result = core->stricteq(leftOperand, rightOperand);
				result = inverseBooleanAtom(result);
				break;
			}
			case STRICT_EQUAL:
			{
				result = core->stricteq(leftOperand, rightOperand);
				break;
			}
			default:
				TessaAssert(false);
				break;
		}

		TessaAssert((result == trueAtom) || (result == falseAtom));
		putOperand(conditionInstruction, result);
	}

	void TessaInterpreter::visit(UnaryInstruction* unaryInstruction) {
		TessaInstruction* operand = unaryInstruction->getOperand();
		TessaUnaryOp opcode = unaryInstruction->getOpcode();

		Atom atomOperand = getOperand(operand);
		Atom result;

		switch (opcode) {
			case BITWISE_NOT:
			{
				
				int integer = AvmCore::integer(atomOperand);
				result = core->intToAtom(~integer);
				break;
			}
			case NEGATE:
			{
				result = core->doubleToAtom(-(core->number(atomOperand)));
				break;
			}
			case NOT:
			{
				result = AvmCore::booleanAtom(atomOperand);
                result = result ^ (trueAtom ^ falseAtom);
				break;
			}
		}

		TessaAssert(result != NULL);
		putOperand(unaryInstruction, result);
	}

	void TessaInterpreter::visit(FindPropertyInstruction* findPropertyInstruction) {
		const Multiname* name = findPropertyInstruction->getMultiname();
		Atom foundObject;

		if (findPropertyInstruction->isFindDefinition()) {
			foundObject = this->env->finddef(name)->atom();
		} else {
			/***
			 * Todo:
			 * Work in with scope
			 */
			uint32_t nameIndex = findPropertyInstruction->getNameIndex();
			bool isStrict = findPropertyInstruction->isStrict();
			ScopeChain* scopeChain = env->scope();
			Atom* withBase = NULL;
			int scopeDepth = getScopeDepth();
			Atom* scopeBase = new Atom[scopeDepth + 1];
			scopeBase[0] = NULL;
			TessaAssert(scopeDepth == 0);
			foundObject = env->findproperty(scopeChain, scopeBase, scopeDepth, name, isStrict, withBase);
		}

		putOperand(findPropertyInstruction, foundObject);
	}

	void TessaInterpreter::visit(ConstantValueInstruction* constantValueInstruction) {
		ConstantValue* constantValue = constantValueInstruction->getConstantValue();
		Atom result;

		if (constantValue->isString()) {
			ConstantString* constantString = (ConstantString*) constantValue;
			Stringp stringValue = constantString->getValue();
			result = stringValue->atom();
		} else if (constantValue->isInteger()) {
			ConstantInt* constantInteger = (ConstantInt*) constantValue;
			if (constantInteger->getType()->isInteger()) {
				int integerValue = constantInteger->getValue();
				result = core->intToAtom(integerValue); 
			} else {
				TessaAssert(constantInteger->getType()->isUnsignedInt());
				uint32_t uintValue = constantInteger->getValue();
				result = core->uintToAtom(uintValue);
			}
		} else if (constantValue->isUndefined()) {
			result = avmplus::AtomConstants::undefinedAtom;
		} else if (constantValue->isNull()) {
			result = avmplus::AtomConstants::nullObjectAtom;
		} else if (constantValue->isBoolean()) {
			ConstantBool* boolean = (ConstantBool*) constantValue;
			if (boolean->getValue()) {
				result = trueAtom;
			} else {
				result = falseAtom;
			}
		} else if (constantValue->isNumber()) {
			ConstantFloat* constantDouble = (ConstantFloat*) constantValue;
			result = core->doubleToAtom(constantDouble->getValue());
		} else {
			TessaAssertMessage(false, "Unknown constant value");
		}

		TessaAssert(result != NULL);
		putOperand(constantValueInstruction, result);
	}

	void TessaInterpreter::visit(ArrayOfInstructions* arrayOfInstructions) {
		int numberOfArguments = arrayOfInstructions->size();
		Atom* arguments = this->createArguments(arrayOfInstructions);
		putOperand(arrayOfInstructions, (Atom)arguments);
	}

	void TessaInterpreter::visit(NewObjectInstruction* newObjectInstruction) {
		Atom* objectPropertyPairs = (Atom*) getOperand(newObjectInstruction->getObjectProperties());
		int numberOfProperties = newObjectInstruction->getNumberOfProperties(); 
		Atom* endOfObjectPropertyPairs = objectPropertyPairs + (numberOfProperties * 2) - 1;

		// op_newobject expects argv to be at the end of the object property pairs
		Atom newObjectAtom = env->op_newobject(endOfObjectPropertyPairs, numberOfProperties)->atom();
		putOperand(newObjectInstruction, newObjectAtom);
	}

	void TessaInterpreter::visit(ConstructInstruction* constructInstruction) {
		Toplevel* toplevel = env->toplevel();
		Atom receiverObject = getOperand(constructInstruction->getReceiverObject());
		Atom* arguments = (Atom*) getOperand(constructInstruction->getArguments());
		int argCount = constructInstruction->getNumberOfArgs();
		Atom constructedObject = toplevel->op_construct(receiverObject, argCount, arguments);
		putOperand(constructInstruction, constructedObject);
	}

	/***
	 * The real argument count is always callArguments->size() - 1 since we ignore the "THIS" pointer
	 */
	void TessaInterpreter::visit(ConstructPropertyInstruction* constructPropertyInstruction) {
		const Multiname* propertyMultiname = constructPropertyInstruction->getPropertyMultiname();
		ArrayOfInstructions* callArguments = constructPropertyInstruction->getArguments();
		int argCount = callArguments->size();
		TessaAssert(argCount >= 1);

		if (propertyMultiname->isRuntime()) {
			TessaAssertMessage(false, "Don't know how to handle runtime multinames");
		}

		Toplevel* toplevel = env->toplevel();
		Atom receiverObject = getOperand(callArguments->getInstruction(0));
		Atom* arguments = new Atom[argCount];
		
		for (int i = 1; i < argCount; i++) {
			arguments[i] = getOperand(callArguments->getInstruction(i));
		}
		arguments[0] = receiverObject;

		// constructprop expects the arg count to NOT include the "this" pointer
		Atom result = toplevel->constructprop(propertyMultiname, argCount - 1, arguments, toplevel->toVTable(receiverObject));
		TessaAssert(result != NULL);
		putOperand(constructPropertyInstruction, result);
	}

	Atom TessaInterpreter::executeDynamicCallInstruction(CallInstruction* callInstruction, Atom* arguments, int argCount) {
		TessaAssert(callInstruction->isDynamicMethod() && !callInstruction->hasAbcOpcode()); 
		Atom functionObject = getOperand(callInstruction->getFunctionObject());
		return env->toplevel()->op_call(functionObject, argCount, arguments);
	}

	void TessaInterpreter::visit(CallVirtualInstruction* callVirtualInstruction) {
		Atom* arguments = (Atom*) getOperand(callVirtualInstruction->getArguments());
		MethodInfo* methodInfo = callVirtualInstruction->getMethodInfo();
		uintptr_t methodId = callVirtualInstruction->getMethodId();
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		Atom receiverObject = getOperand(callVirtualInstruction->getReceiverObject());

		LoadVirtualMethodInstruction* loadVirtualMethod = callVirtualInstruction->getLoadedMethodToCall();
        MethodEnv* methodEnv = (MethodEnv*) getOperand(loadVirtualMethod->getLoadedMethodEnv());
		MethodInfo* loadedMethod = (MethodInfo*) getOperand(loadVirtualMethod->getLoadedMethodInfo());
		/***
		  * TessaAssert(methodEnv->method == methodInfo);	
		  * Can't assert this. Doesn't work for virtual methods. Unfortunatley we can't tell
		  * if a method is early bound at this point. FIX FIX FIX
		  */
		int argCount = callVirtualInstruction->getNumberOfArgs();
		Atom returnValue = loadedMethod->invoke(methodEnv, argCount, arguments);
		putOperand(callVirtualInstruction, returnValue);
	}

	void TessaInterpreter::visit(CallStaticInstruction* callStaticInstruction) {
		TessaAssert(false);
	}

	void TessaInterpreter::visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) {
		int methodId = loadVirtualMethodInstruction->getMethodId();
		Atom receiverObject = getOperand(loadVirtualMethodInstruction->getReceiverObject());
		VTable* vtable = env->toplevel()->toVTable(receiverObject); // includes null check
        AvmAssert(methodId < (int32_t) vtable->traits->getTraitsBindings()->methodCount);
        MethodEnv* methodEnv = vtable->methods[methodId];
		MethodInfo* methodInfo = methodEnv->method;

		putOperand(loadVirtualMethodInstruction->getLoadedMethodEnv(), (Atom) methodEnv);
		putOperand(loadVirtualMethodInstruction->getLoadedMethodInfo(), (Atom) methodInfo);
	}

	Atom TessaInterpreter::executeCallInstruction(CallInstruction*	callInstruction, Atom* arguments, int argCount) {
		TessaAssert(false);
		TessaAssert(!callInstruction->isDynamicMethod());
		MethodInfo* methodInfo = callInstruction->getMethodInfo();
		uintptr_t methodId = callInstruction->getMethodId();
		MethodSignaturep methodSignature = methodInfo->getMethodSignature();
		Atom receiverObject = getOperand(callInstruction->getReceiverObject());
		Toplevel* toplevel = env->toplevel();

		VTable* vtable = toplevel->toVTable(receiverObject); // includes null check
        AvmAssert(methodId < vtable->traits->getTraitsBindings()->methodCount);
        MethodEnv* methodEnv = vtable->methods[methodId];
		/***
		  * TessaAssert(methodEnv->method == methodInfo);	
		  * Can't assert this. Doesn't work for virtual methods. Unfortunatley we can't tell
		  * if a method is early bound at this point. FIX FIX FIX
		  */
		return methodEnv->method->invoke(methodEnv, argCount, arguments);
	}

	Atom TessaInterpreter::executeAbcCallInstruction(CallInstruction* callInstruction, Atom* arguments, int argCount) {
		TessaAssert(callInstruction->hasAbcOpcode());

		switch (callInstruction->getAbcOpcode()) {
			case OP_applytype:
			{
				Atom functionObject = getOperand(callInstruction->getFunctionObject());
				// Have to do arguments + 1 because arguments[0] is the function object
				// and apply_type expects arguments to not have that
				TessaAssert(functionObject == arguments[0]);
				return op_applytype(env, functionObject, argCount, (arguments + 1));
			}
			case OP_newfunction:
			{
				// ArgCount doesn't count the "this" pointer
				TessaAssert(argCount == 0);
				uint32_t methodIndex = core->integer_u(arguments[0]);
				MethodInfo* methodInfo = poolObject->getMethodInfo(methodIndex);

				int scopeDepth = getScopeDepth();
				Atom* scopeBase = new Atom[scopeDepth + 1];
				for (int i = 0; i < scopeDepth; i++) {
					scopeBase[i] = getOperand(peekScope(i));
				}

				return env->newfunction(methodInfo, env->scope(), scopeBase)->atom();
			}
			default:
				TessaAssert(false);
		}

		TessaAssert(false);
		return NULL;
	}

	void TessaInterpreter::visit(CallInstruction* callInstruction) {
		Toplevel *toplevel = env->toplevel();
		Atom* arguments = (Atom*) getOperand(callInstruction->getArguments());
		int argCount = callInstruction->getNumberOfArgs();
		Atom result;

		if (callInstruction->hasAbcOpcode()) {
			result = executeAbcCallInstruction(callInstruction, arguments, argCount);
		} else if (callInstruction->isDynamicMethod()) {
			result = executeDynamicCallInstruction(callInstruction, arguments, argCount);
		} else {
			result = executeCallInstruction(callInstruction, arguments, argCount);
		}

		TessaAssert(result != NULL);
		putOperand(callInstruction, result);
	}

	void TessaInterpreter::visit(CallPropertyInstruction* callPropertyInstruction) {
		const Multiname* propertyName = callPropertyInstruction->getProperty();
		if (propertyName->isRuntime()) {
			TessaAssert(false);
		} 
		
		Toplevel* toplevel = env->toplevel();
		Atom receiverAtom = getOperand(callPropertyInstruction->getReceiverObject());
		Atom* arguments = this->createArguments(callPropertyInstruction->getArguments());
		uint32_t argCount = callPropertyInstruction->getNumberOfArgs();
		Atom result = toplevel->callproperty(receiverAtom, propertyName, argCount, arguments, toplevel->toVTable(receiverAtom));
		putOperand(callPropertyInstruction, result);
	}

	void TessaInterpreter::visit(CallSuperInstruction* callSuperInstruction) {
		Atom receiverObject = getOperand(callSuperInstruction->getReceiverObject());
		Atom* arguments = this->createArguments(callSuperInstruction->getArguments());
		Atom result;

		const Multiname* multiname = callSuperInstruction->getMultiname();
		if (multiname->isRuntime()) {
			TessaAssert(false);
		}

		env->nullcheck(receiverObject);
		result = env->callsuper(multiname, callSuperInstruction->getNumberOfArgs(), arguments);
		TessaAssert(result != NULL);
		putOperand(callSuperInstruction, result);
	}

	void TessaInterpreter::visit(ConstructSuperInstruction* constructSuperInstruction) {
		MethodInfo* methodInfo = constructSuperInstruction->getMethodInfo();
		Atom receiverAtom = getOperand(constructSuperInstruction->getReceiverObject());
		Atom* arguments = createArguments(constructSuperInstruction->getArguments());
		uint32_t argCount = constructSuperInstruction->getNumberOfArgs();

		env->nullcheck(receiverAtom);
		env->super_init()->coerceEnter((int32_t)argCount, arguments);
	}

	void TessaInterpreter::visit(ConditionalBranchInstruction* conditionalBranchInstruction) {
		Atom branchCondition = getOperand(conditionalBranchInstruction->getBranchCondition());
		BasicBlock* targetBlock;

		if (branchCondition == trueAtom) {
			targetBlock = conditionalBranchInstruction->getTrueTarget();
		} else {
			TessaAssert(branchCondition == falseAtom)
			targetBlock = conditionalBranchInstruction->getFalseTarget();
		}

		jumpToTarget(targetBlock);
	}

	void TessaInterpreter::visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) {
		BasicBlock* targetBlock = unconditionalBranchInstruction->getBranchTarget();
		jumpToTarget(targetBlock);
	}

	void TessaInterpreter::visit(PhiInstruction* phiInstruction) {
		BasicBlock* cameFromBasicBlock = currentFunction->getBasicBlocks()->get(flowingFromBasicBlock);
		TessaInstruction* phiValue = phiInstruction->getOperand(cameFromBasicBlock);
		Atom result = getOperand(phiValue);
		putOperand(phiInstruction, result);
	}

	void TessaInterpreter::visit(ParameterInstruction* parameterInstruction) {
		TessaInstruction* resolvedInstruction = parameterInstruction->resolve();

		if (this->currentBasicBlockId != 0) {
			TessaAssert(resolvedInstruction != parameterInstruction);
		}

		Atom result = getOperand(resolvedInstruction);
		putOperand(parameterInstruction, result);
	}

	Atom TessaInterpreter::coerceAtomToOtherAtomType(TessaTypes::Type* typeToCoerce, Atom instructionValue) {
		Atom coercedType;
		if (typeToCoerce->isObject()) {
			// There are probably more cases, we just haven't fleshed them out yet
			if (instructionValue == avmplus::AtomConstants::undefinedAtom) {
				coercedType = avmplus::AtomConstants::nullObjectAtom;
			} else {
				coercedType = instructionValue;
			}
		} else if (typeToCoerce->isBoolean()) {
			coercedType = AvmCore::booleanAtom(instructionValue);
		} else if (typeToCoerce->isUnsignedInt()) {
			coercedType = core->uintAtom(instructionValue);
		} else if (typeToCoerce->isInteger()) {
			coercedType = core->intAtom(instructionValue);
		} else if (typeToCoerce->isNumber()) {
			coercedType = core->numberAtom(instructionValue);
		} else if (typeToCoerce->isString()) {
			coercedType = core->string(instructionValue)->atom();
		} else {
			coercedType = instructionValue;
		}	
		
		return coercedType;
	}


	void TessaInterpreter::visit(CoerceInstruction* coerceInstruction) {
		TessaInstruction* instructionToCoerce = coerceInstruction->getInstructionToCoerce();
		Atom instructionValue = getOperand(instructionToCoerce);
		Atom coercedType;

		if (coerceInstruction->isMultinameCoerce()) {
			Toplevel* toplevel = env->toplevel();
			coercedType = toplevel->coerce(instructionValue, getTraits(coerceInstruction->getMultinameToCoerce(), poolObject, toplevel, core));
		} else {
			TessaTypes::Type* typeToCoerce = coerceInstruction->getTypeToCoerce();
			coercedType = coerceAtomToOtherAtomType(typeToCoerce, instructionValue);
		}

		putOperand(coerceInstruction, coercedType);
	}

	void TessaInterpreter::visit(ConvertInstruction* convertInstruction) {
		Type* typeToConvert = convertInstruction->getTypeToCoerce();
		Atom instructionValue = getOperand(convertInstruction->getInstructionToCoerce());
		Atom convertedType = coerceAtomToOtherAtomType(typeToConvert, instructionValue);

		TessaAssert(convertedType != NULL);
		putOperand(convertInstruction, convertedType);
	}

	void TessaInterpreter::visit(GetPropertyInstruction* getPropertyInstruction) {
		const Multiname* propertyName = getPropertyInstruction->getPropertyMultiname();
		Atom receiverObject = getOperand(getPropertyInstruction->getReceiverInstruction());
		TessaInstruction* key = getPropertyInstruction->getPropertyKey();
		Atom result;

		if (!propertyName->isRuntime()) {
			// Key is null in these scenarios
			Toplevel* toplevel = env->toplevel();
			result = toplevel->getproperty(receiverObject, propertyName, toplevel->toVTable(receiverObject));
		} else if (!propertyName->isRtns() && isInteger(getOperand(key)) && atomCanBeUint32(getOperand(key)) && AvmCore::isObject(receiverObject)) {
			Atom atomKey = getOperand(key);
			uint32_t uintValue = toUint32(atomKey);
			result = AvmCore::atomToScriptObject(receiverObject)->getUintProperty(uintValue);
		} else if (propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, receiverObject)) {
			TessaAssert(false);
		} else {
			Atom atomKey = getOperand(key);
			result = AvmCore::atomToScriptObject(receiverObject)->getAtomProperty(atomKey);
		}

		TessaAssert(result != NULL);
		putOperand(getPropertyInstruction, result);
	}

	void TessaInterpreter::visit(SetPropertyInstruction* setPropertyInstruction) {
		const Multiname* propertyName = setPropertyInstruction->getPropertyName();
		TessaInstruction* key = setPropertyInstruction->getPropertyKey();
		Atom value = getOperand(setPropertyInstruction->getValueToSet());
		Atom receiverObject = getOperand(setPropertyInstruction->getReceiverInstruction());

		if (!propertyName->isRuntime()) {
			Toplevel* toplevel = env->toplevel();
			toplevel->setproperty(receiverObject, propertyName, value, toplevel->toVTable(receiverObject));
		} else if (!propertyName->isRtns() && isInteger(getOperand(key)) && atomCanBeUint32(getOperand(key)) && AvmCore::isObject(receiverObject)) {
			Atom keyAtom = getOperand(key);
			AvmCore::atomToScriptObject(receiverObject)->setUintProperty(toUint32(keyAtom), value);
		} else if (propertyName->isRtns() && !AvmCore::isDictionaryLookup(receiverObject, getOperand(key))) {
			TessaAssert(false);
		} else {
			AvmCore::atomToScriptObject(receiverObject)->setAtomProperty(getOperand(key), value);
		}
	}

	void TessaInterpreter::visit(InitPropertyInstruction* initPropertyInstruction) {
		AvmAssert(false);
		// Atom valueToInitialize = getOperand(initPropertyInstruction->getValueToInit());
		Atom valueToInitialize = getOperand(initPropertyInstruction->getValueToSet());
		Atom receiverObject = getOperand(initPropertyInstruction->getReceiverInstruction());
		const Multiname* propertyName = initPropertyInstruction->getPropertyName();
		Toplevel* toplevel = env->toplevel();

		if (propertyName->isRuntime()) {
			TessaAssert(false);
        }

        env->initproperty(receiverObject, propertyName, valueToInitialize, toplevel->toVTable(receiverObject));
	}

	void TessaInterpreter::visit(GetSlotInstruction* getSlotInstruction) {
		Atom receiverObject = getOperand(getSlotInstruction->getReceiverObject());
		int slotIndex = getSlotInstruction->getSlotNumber();
		Atom slotResult = AvmCore::atomToScriptObject(receiverObject)->getSlotAtom(slotIndex);
		putOperand(getSlotInstruction, slotResult);
	}

	void TessaInterpreter::visit(SetSlotInstruction* setSlotInstruction) {
		int slotNumber = setSlotInstruction->getSlotNumber();
		Atom receiverObject = getOperand(setSlotInstruction->getReceiverObject());
		Atom valueToSet = getOperand(setSlotInstruction->getValueToSet());
		AvmCore::atomToScriptObject(receiverObject)->coerceAndSetSlotAtom((uint32_t)slotNumber, valueToSet);
	}

	void TessaInterpreter::visit(NewArrayInstruction* newArrayInstruction) {
		Atom* arrayElements = createArguments(newArrayInstruction->getArrayElements());
		int argCount = newArrayInstruction->numberOfElements();
		Toplevel* toplevel = env->toplevel();
		Atom newArray = avmplus::newarray(toplevel, argCount, arrayElements)->atom();
		putOperand(newArrayInstruction, newArray);
	}

	void TessaInterpreter::visit(NextNameInstruction* nextNameInstruction) {
		Atom object = getOperand(nextNameInstruction->getBaseObject());
		Atom registerObject = getOperand(nextNameInstruction->getRegisterFile());
        Atom result = env->nextname(object, AvmCore::integer_i(registerObject));
		putOperand(nextNameInstruction, result);
	}

	void TessaInterpreter::visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) {
		/***
		 * Don't do anything for this. The hasMorePropertiesInstruction needs to set the value for this.
		 * Read in hasMoreProperties header file to find out. Also look at the hasnext2 AVM opcode.
		 */
	}

	void TessaInterpreter::visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesRegisterInstruction) {
		/***
		 * Don't do anything for this. The hasMorePropertiesInstruction needs to set the value for this.
		 * Read in hasMoreProperties header file to find out. Also look at the hasnext2 AVM opcode.
		 */
	}

	void TessaInterpreter::visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) {
		HasMorePropertiesRegisterInstruction* registerIndex = hasMorePropertiesInstruction->getRegisterIndex(); 
		Atom indexAtom = getOperand(registerIndex->getRegisterInstruction());
		TessaAssert(isInteger(indexAtom));
		int indexInteger = AvmCore::integer(indexAtom);
		Atom hasNextResult;

		if (hasMorePropertiesInstruction->modifiesObject()) {
			// AVM Opcode hasnext2
			HasMorePropertiesObjectInstruction* hasMorePropertiesObjectInstruction = hasMorePropertiesInstruction->getObjectIndex();
			TessaInstruction* objectIterator = hasMorePropertiesObjectInstruction->getObjectInstruction();
			Atom objectAtom = getOperand(objectIterator);

			hasNextResult = env->hasnextproto(objectAtom, indexInteger) ? trueAtom : falseAtom;
			putOperand(hasMorePropertiesObjectInstruction, objectAtom);
		} else {
			// This is for AVMOpcode hasnext
			TessaAssert(false);
		}

		// This is changed in hasNextProto
		indexAtom = core->intToAtom(indexInteger);
		putOperand(registerIndex, indexAtom);
		putOperand(hasMorePropertiesInstruction, hasNextResult);
	}

	

	void TessaInterpreter::visit(PushScopeInstruction* pushScopeInstruction) {
		this->pushScope(pushScopeInstruction->getScopeObject());
	}

	void TessaInterpreter::visit(GetScopeObjectInstruction* getScopeObjectInstruction) {
		int32_t scopeIndex = getScopeObjectInstruction->getScopeIndex();
		Atom scopeObject;

		if (getScopeObjectInstruction->isOuterScope()) {
			ScopeChain* scopeChain = env->scope();
			scopeObject = scopeChain->getScope(scopeIndex);
		} else {
			scopeObject = getOperand(peekScope(scopeIndex));
		}

		TessaAssert(scopeObject != NULL);
		putOperand(getScopeObjectInstruction, scopeObject);
	}

	void TessaInterpreter::visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) {
		ScopeChain* scopeChain = env->scope();
		Atom globalScope;
		if (scopeChain->getSize() > 0) {
			globalScope = scopeChain->getScope(0);
		} else {
			globalScope = getOperand(peekScope(0));
		}

		TessaAssert(globalScope != NULL);
		putOperand(getGlobalScopeInstruction, globalScope);
	}

	void TessaInterpreter::visit(PopScopeInstruction* popScopeInstruction) {
		TessaAssert(false);
	}

	void TessaInterpreter::visit(WithInstruction* withInstruction) {
		TessaAssert(false);
	}

	void TessaInterpreter::visit(TypeOfInstruction* typeOfInstruction) {
		TessaAssert(typeOfInstruction->isLateCheck());
		Atom valueToCheck = getOperand(typeOfInstruction->getObjectToTest());
		Atom typeToCompare = getOperand(typeOfInstruction->getTypeToCompare());

		Toplevel* toplevel = env->toplevel();
		Atom result = AvmCore::istypeAtom(valueToCheck, toplevel->toClassITraits(typeToCompare));
		putOperand(typeOfInstruction, result);
	}

	void TessaInterpreter::visit(SwitchInstruction* switchInstruction) {
		Atom switchIndexAtom = getOperand(switchInstruction->getSwitchValue());
		TessaAssert(isInteger(switchIndexAtom));
		int index = AvmCore::integer(switchIndexAtom);

		CaseInstruction* caseInstruction = switchInstruction->getCase(index);
		BasicBlock* caseBlock = caseInstruction->getTargetBlock();
		jumpToTarget(caseBlock);
	}
	void TessaInterpreter::visit(CaseInstruction* caseInstruction) {
	}

	void TessaInterpreter::visit(NewActivationInstruction* newActivationInstruction) {
		Atom result = env->newActivation()->atom();
		putOperand(newActivationInstruction, result);
	}

	/***
	 * Call interface isn't represented in ABC at all. Instead it's a decision
	 * caused by the verifier when we can early bind a callproperty.
	 * Thus there is no real "guide" on how to implement this since it normally only occurs
	 * at compile time. 
	 */
	void TessaInterpreter::visit(CallInterfaceInstruction* callInterfaceInstruction) {
		// Call interface methods must fetch the method id from the method info itself. Verifier gives the
		// AbcToTessaTranslator a wacky methodId during parsing.
		Atom receiverAtom = getOperand(callInterfaceInstruction->getReceiverObject());
		Atom* arguments = (Atom*) getOperand(callInterfaceInstruction->getArguments());
		int argCount = callInterfaceInstruction->getNumberOfArgs();
		int methodIid = callInterfaceInstruction->getMethodId();

		// note, could be MethodEnv* or ImtThunkEnv*
		int index = int(callInterfaceInstruction->getMethodId() % VTable::IMT_SIZE);
		int offset = (offsetof(VTable,imt) + (sizeof(ImtThunkEnv*) * index)) / sizeof(int32_t);	// Let LLVM recalculate

		VTable* vtable = env->toplevel()->toVTable(receiverAtom);
		ImtThunkEnv* loadedInterfaceEnv = vtable->imt[index];
		Atom (*fp)(ImtThunkEnv*, int, Atom*, int) = (Atom(*)(ImtThunkEnv*, int, Atom*, int))(loadedInterfaceEnv->implGPR());

		// Have to coerce arguments into typed arguments.
		TessaAssert(false);

		// Receiver must be in ScriptObject*
		TessaAssert(core->isObject(receiverAtom));
		Atom resultValue = fp(loadedInterfaceEnv, argCount, arguments, methodIid);

		// Have to cast the return value back to an atom
		TessaAssert(false);
		putOperand(callInterfaceInstruction, resultValue);
	}

	void TessaInterpreter::visit(InlineBeginInstruction* inlineBeginInstruction) {
		TessaAssert(false);
	}
}