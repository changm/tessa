#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ArraySetElementInstruction::ArraySetElementInstruction(TessaInstruction *receiverObject, TessaInstruction* index, TessaInstruction *valueToSet) 
		: ArrayAccessInstruction(receiverObject, index) 
	{
		this->valueToSet = valueToSet;
	}
}