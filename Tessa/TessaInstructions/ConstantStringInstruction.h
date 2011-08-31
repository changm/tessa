
namespace TessaInstructions {
	using namespace avmplus;

	class ConstantStringInstruction : ConstantValueInstruction {
	private:
		Stringp value;

	public:
		ConstantStringInstruction(Stringp value);
	};
}