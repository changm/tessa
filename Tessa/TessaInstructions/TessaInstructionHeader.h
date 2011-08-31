#include <iostream>
#include <string.h>
#include <windows.h>

#ifndef __TESSAINSTRUCTIONHEADER__
#define __TESSAINSTRUCTIONHEADER

#pragma warning(disable:4291) // Disable matching delete operator. Occurs due to enabling C++ exceptions

#include "TessaAssert.h"
#include "avmplus.h"
#include "TessaTypeHeader.h"

namespace TessaVisitors {
	class TessaVisitorInterface;
}

namespace TessaVM {
	class BasicBlock;
}

/***
 * Format of this header file is copied from AvmCore.h
 */
using namespace std;
using namespace TessaVisitors;
using namespace TessaTypes;
using namespace TessaVM;

namespace TessaInstructions {
	class TessaInstruction;
	class ArrayAccessInstruction;
	class ArrayGetElementInstruction;
	class ArraySetElementInstruction;
	class ArrayIncrementElementInstruction;
	class ArrayOfInstructions;
	class NewArrayInstruction;
	class BinaryInstruction;
	class BranchInstruction;
	class CallInstruction;
	class CallStaticInstruction;
	class CallVirtualInstruction;
	class CallInterfaceInstruction;
	class LoadVirtualMethodInstruction;
	class NewObjectInstruction;
	class CallPropertyInstruction;
	class CallSuperInstruction;
	class ConstructInstruction;
	class ConstructPropertyInstruction;
	class ConstructSuperInstruction;
	class ClassFieldAccessInstruction;
	class ConditionInstruction;
	class ConditionalBranchInstruction;
	class CoerceInstruction;
	class ConvertInstruction;
	class ConstantValueInstruction;
	class FindPropertyInstruction;
	class PropertyAccessInstruction;
	class GetPropertyInstruction;
	class SetPropertyInstruction;
	class InlineBeginInstruction;
	class InitPropertyInstruction;
	class SlotAccessInstruction;
	class GetSlotInstruction;
	class SetSlotInstruction;
	class ScopeInstruction;
	class PushScopeInstruction;
	class GetScopeObjectInstruction;
	class GetGlobalScopeInstruction;
	class PopScopeInstruction;
	class WithInstruction;
	class HasMorePropertiesInstruction;
	class HasMorePropertiesObjectInstruction;
	class HasMorePropertiesRegisterInstruction;
	class NewActivationInstruction;
	class NextNameInstruction;
	class ParameterInstruction;
	class PhiInstruction;
	class ReturnInstruction;
	class SelectInstruction;
	class CaseInstruction;
	class SwitchInstruction;
	class ThisInstruction;
	class TypeOfInstruction;
	class UnaryInstruction;
	class UnconditionalBranchInstruction;

	class SsaConverter;
	class ConsistencyChecker;
	class TessaCreationApi;
}

/****
 * Please please please keep this NICE. Comment which files rely on which files because
 * C++ include order matters
 */
#include "TessaInstruction.h"
#include "ArrayAccessInstruction.h"
#include "ArrayGetElementInstruction.h"	// Relies on array access instruction
#include "ArraySetElementInstruction.h" // relies on array access instruction
#include "ArrayIncrementElementInstruction.h"	// Relies on array access
#include "ArrayOfInstructions.h"
#include "NewArrayInstruction.h"

#include "BinaryOps.h"
#include "BinaryInstruction.h"	// Binary instruction relies on binary ops. Must be included after
#include "ConditionInstruction.h"	// Relies on binary instruction

#include "BranchInstruction.h"	
#include "ConditionalBranchInstruction.h"	// Relies on branch instruction
#include "UnconditionalBranchInstruction.h"	// Relies on branch instruction

#include "ConstantValueInstruction.h"

#include "CallInstruction.h"
#include "CallStaticInstruction.h"		// Relies on call Instruction
#include "CallVirtualInstruction.h"		// Relies on call instruction
#include "CallPropertyInstruction.h"	// Relies on call instruction
#include "CallSuperInstruction.h"		// Relies on call instruction
#include "CallInterfaceInstruction.h"	// Relies on call instruction
#include "ConstructInstruction.h"			// Relies on call instruction
#include "ConstructPropertyInstruction.h"	// Relies on construct instruction
#include "ConstructSuperInstruction.h"		// Relies on constructinstruction
#include "LoadVirtualMethodInstruction.h"	// Relies only on TessaInstruction, but used by the call instructions
#include "InlineBeginInstruction.h"			// Relies on TessaInstruction - Yes this is correct, it isn't call instruction


#include "NewObjectInstruction.h"		// Looks a lot like construct, perhaps merge later?

#include "CoerceInstruction.h"
#include "ConvertInstruction.h"		// Relies on coerce instruction

#include "FindPropertyInstruction.h"
#include "ParameterInstruction.h"
#include "PhiInstruction.h"

#include "SlotAccessInstruction.h"		
#include "GetSlotInstruction.h"			// Relies on slot access instruction
#include "SetSlotInstruction.h"			// Relies on slot access instruction

#include "NewActivationInstruction.h"

// Property iterator instructions
#include "NextNameInstruction.h"		
#include "HasMorePropertiesObjectInstruction.h"	
#include "HasMorePropertiesRegisterInstruction.h"
#include "HasMorePropertiesInstruction.h"	// Relies on HasMorePropertiesObject and Index Instructions

#include "PropertyAccessInstruction.h"
#include "GetPropertyInstruction.h"	// Relies on propertyaccessinstruction
#include "SetPropertyInstruction.h" //Relies on property access instruction
#include "InitPropertyInstruction.h"	// Relies on property access
#include "ClassFieldAccessInstruction.h"	// Class field access relies on property access instruction. Must be included after

#include "ReturnInstruction.h"
#include "SelectInstruction.h"

#include "SwitchInstruction.h"
#include "CaseInstruction.h"		// Relies on switch instruction

#include "ScopeInstruction.h"
#include "PopScopeInstruction.h"	// Relies on ScopeInstruction
#include "PushScopeInstruction.h"	// Relies on ScopeInstruction
#include "WithInstruction.h"		// Relies on ScopeInstruction
#include "GetScopeObjectInstruction.h"	// Relies on ScopeInstruction
#include "GetGlobalScopeInstruction.h"	// Relies on GetScopeObjectInstruction 

#include "ThisInstruction.h"
#include "TypeOfInstruction.h"
#include "UnaryOps.h"	
#include "UnaryInstruction.h"	//UnaryInstruction requires UnaryOps.

#include "SsaConverter.h"		// Relies on all instruction types
#include "ConsistencyChecker.h"	// Relies on all instruction types
#include "TessaCreationApi.h"	// Relies on all instruction types and Ssa Converter

#include "TessaVisitorInterface.h"	// Relies on all instruction types

#endif	// End __TESSAINSTRUCTIONHEADER__
