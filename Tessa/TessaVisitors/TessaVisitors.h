
#ifndef __TESSAVISITORS__
#define __TESSAVISITORS__

#include "TessaVM.h"	// Also includes TessaInstructions

namespace TessaVisitors {
	class TessaInterpreter;
	class TypePropagator;
	class TessaInliner;
	class TessaVisitorInterface;
	class DeadCodeElimination;
}

#include "TessaVisitorInterface.h"
#include "TypePropagator.h"
#include "TessaInliner.h"
#include "TessaInterpreter.h"
#include "TypeEnricher.h"
#include "DeadCodeElimination.h"

#endif __TESSAVISITORS__