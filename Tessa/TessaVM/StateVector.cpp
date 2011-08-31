
#include "TessaVM.h"

namespace TessaVM {
	StateVector::StateVector(int numberOfLocals, MMgc::GC* gc) {
		this->gc = gc;
		this->currentReferences= new (gc) List<TessaInstruction*, avmplus::LIST_GCObjects>(gc);
	}

	int StateVector::getNumberOfLocals() {
		return this->currentReferences->size();
	}

	void StateVector::setLocal(int slot, TessaInstruction* reference) {
		TessaAssert(slot <= getNumberOfLocals() + 1);	// + 1 since we can grow, but growing has to be in ORDER. so +1
		currentReferences->set(slot, reference); 
	};

	TessaInstruction* StateVector::getLocal(int slot) {
		TessaAssert((uint32_t)slot < currentReferences->size());
		TessaInstruction* currentReference = currentReferences->get(slot);
		TessaAssert(currentReference != NULL);
		return currentReference;
	}

	// Again have to delete in order from the end only
	void StateVector::deleteLocal(int slot) {
		TessaAssert(slot == getNumberOfLocals());
		currentReferences->removeLast();
	}

	/***
	 * Returns a shallow copy 
	 */
	StateVector* StateVector::clone() {
		StateVector *clone = new (gc) StateVector(currentReferences->size(), gc);

		for (int32_t i = 0; i < getNumberOfLocals(); i++) {
			TessaInstruction* currentReference = getLocal(i);
			clone->setLocal(i, currentReference);
		}

		return clone;
	}
}