
#include "TessaInstructionHeader.h"
#include "TessaTypeHeader.h"

namespace TessaInstructions {
	NewObjectInstruction::NewObjectInstruction(ArrayOfInstructions* newObjectProperties, TessaVM::BasicBlock* insertAtEnd) 
		: TessaInstruction(insertAtEnd)
	{
		this->newObjectProperties = newObjectProperties;

		/***
		 * NewObject must have name/value pairs.
		 * The names must be strings
		 */
		for (uint32_t i = 0; i < newObjectProperties->size(); i++) {
			TessaInstruction* value = newObjectProperties->getInstruction(i);
			if ((i % 2) == 0) {
				TessaAssert(value->isCoerce() || value->isConstantValue() || (value->isConvert()));
				if (value->isConstantValue()) {
					ConstantValueInstruction* constantValue = (ConstantValueInstruction*) value;
					TessaAssert(constantValue->getConstantValue()->isString());
				}
			}
		}
	}

	void NewObjectInstruction::print() {
		printf("%s NewObjectInstruction (%s) (Type %s)\n", getPrintPrefix().c_str(), 
			newObjectProperties->getOperandString().c_str(),
			getType()->toString().data());
	}

	void NewObjectInstruction::visit(TessaVisitorInterface* tessaVisitor) {
		tessaVisitor->visit(this);
	}

	bool NewObjectInstruction::isNewObject() {
		return true;
	}

	bool NewObjectInstruction::producesValue() {
		return true;
	}

	ArrayOfInstructions* NewObjectInstruction::getObjectProperties() {
		return newObjectProperties;
	}

	int NewObjectInstruction::getNumberOfProperties() {
		// Divide by 2 because the newObjectProperties is a collection of name/value pairs.
		// The real number of properties is the number of pairs
		return newObjectProperties->size() / 2;
	}

	List<TessaValue*, LIST_GCObjects>* NewObjectInstruction::getOperands(MMgc::GC* gc) {
		avmplus::List<TessaValue*, LIST_GCObjects>* operandList = new (gc) avmplus::List<TessaValue*, LIST_GCObjects>(gc);
		ArrayOfInstructions* objects = getObjectProperties();
		operandList->add(objects);
		/*
		for (int i = 0; i < objects->size(); i++) {
			TessaInstruction* currentOp = objects->getInstruction(i);
			operandList->add(currentOp);
		}
		*/

		return operandList;
	}
}