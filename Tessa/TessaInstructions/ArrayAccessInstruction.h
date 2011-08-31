
#ifndef __TESSAINSTRUCTIONS__ARRAYACCESSINSTRUCTION
#define __TESSAINSTRUCTIONS__ARRAYACCESSINSTRUCTION

namespace TessaInstructions {
	class ArrayAccessInstruction : public TessaInstruction {
	private:
		TessaInstruction* receiverObject;
		TessaInstruction* index;
		virtual void forceAbstract() = 0;

	public:
		ArrayAccessInstruction(TessaInstruction* receiverObject, TessaInstruction* index);
	};
}

#endif