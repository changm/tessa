#ifndef __TESSAINSTRUCTIONS__TESSABINARYOPS__
#define __TESSAINSTRUCTIONS__TESSABINARYOPS__

namespace TessaInstructions {
	enum TessaBinaryOp { 
		ADD, SUBTRACT, MULTIPLY, DIVIDE, MOD,	// Arithmetic
		EQUAL, NOT_EQUAL, LESS_THAN, NOT_LESS_THAN, LESS_EQUAL_THAN, NOT_LESS_EQUAL_THAN, GREATER_THAN, NOT_GREATER_THAN, GREATER_EQUAL_THAN, NOT_GREATER_EQUAL_THAN, // conditionals
		BITWISE_AND, BITWISE_OR, BITWISE_XOR, BITWISE_LSH, BITWISE_RSH, BITWISE_URSH,	// Bitwise Operators. U stands for unsigned otherwise default to signed
		IFTRUE, IFFALSE, STRICT_EQUAL, STRICT_NOT_EQUAL
	};

	
}

#endif