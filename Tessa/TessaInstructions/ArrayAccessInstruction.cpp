#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ArrayAccessInstruction::ArrayAccessInstruction(TessaInstruction* receiverObject, TessaInstruction* index) {
		this->receiverObject = receiverObject;
		this->index = index;
	}
}