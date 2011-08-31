
/****
 * A state vector keeps track of a variable. This can be any number of things whether they be declared local variables
 * or stack slots. State vectors can grow/shrink the number of things they track at any given time.
 */
namespace TessaVM {
	class StateVector : public MMgc::GCObject {

	private:
		List<TessaInstruction*, avmplus::LIST_GCObjects>* currentReferences;
		MMgc::GC *gc;

	public:
		StateVector(int numberOfLocals, MMgc::GC* gc);
		int getNumberOfLocals();
		void setLocal(int slot, TessaInstruction* reference);
		TessaInstruction* getLocal(int slot);
		void deleteLocal(int slot);
		StateVector*	clone();
	};
}