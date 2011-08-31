#include "TessaInstructionHeader.h"

namespace TessaInstructions {

	ConstantStringInstruction::ConstantStringInstruction(Stringp value) {
		this->value = value;
	}
}