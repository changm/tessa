
#include "TessaInstructionHeader.h"

namespace TessaInstructions {
	ArrayGetElementInstruction::ArrayGetElementInstruction(TessaInstruction* receiverObject, TessaInstruction* index) 
		: ArrayAccessInstruction(receiverObject, index) {

	}

}