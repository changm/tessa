
#ifndef __TESSAVM__
#define __TESSAVM__

#include "TessaInstructionHeader.h"

namespace TessaVM {
	class ASClass;
	class ASFunction;
	class ASPackage;
	class BasicBlock;
	class StateVector;
};

#include "StateVector.h"
#include "BasicBlock.h"	// Relies on state vector
#include "ASFunction.h"	// Relies on basic block
#include "ASClass.h"	// Relies on ASclass
#include "ASPackage.h"	// Relies on ASClass


#endif