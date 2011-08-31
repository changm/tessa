/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is [Open Source Virtual Machine.].
 *
 * The Initial Developer of the Original Code is
 * Adobe System Incorporated.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Adobe AS3 Team
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// This file provides common, private APIs for the eval implementation.
//
// About the coding style used in eval:
//
//  - eval.h is included into every cpp file in the eval subdirectory.
//
//  - Inline functions are almost never in-line in the class definition, but
//    are placed in a separate file and declared 'inline' explicitly.  This
//    helps separate definition from implementation and reduces clutter, and
//    it resolves circular dependencies between classes in that two class
//    definitions will be visible to both classes' inline functions.  The main
//    exception to the rule is trivial constructors, especially in eval-parse.h.
//
//  - Class members are 'const' whenever possible.
//
//  - All eval code is placed in the namespace avmplus::RTC ("run-time compiler")
//    so that it is clearly segregated from the rest of the VM; no part of the
//    VM code should need to open that namespace at all.
//
//  - Reasonably standard C++ is assumed, the intent is not for this code to
//    be portable to the most feeble compilers for embedded systems.
//
//  - Almost all allocation is off a private heap of header-less objects that
//    has minimal allocation cost, very low fragmentation, and which can be
//    freed in bulk very quickly.  See eval-util.h.
//
//  - I've strived to make the code independent of avmplus, so that it can be
//    easily incorporated into a standalone compiler.  As a consequence, there
//    is no use of String, AvmCore, GC, or other central data structures of
//    avmplus; the necessary functionality (not much) is implemented inside
//    eval or made available through an abstract HostContext class provided
//    to the Compiler instance when the latter is created.
//
//    Still, a few dependencies remain:
//
//     - avmplus.h is included everywhere, so there's a dependency on that name,
//       if nothing else
//
//     - ActionBlockConstants data, both opcode definitions and the instruction
//       attribute table
//
//     - MethodInfo, because eval uses the attribute bits defined therein
//
//     - MathUtils, for isNaN (in one case, can be factored into HostContext)
//
//     - AvmAssert (can be mapped trivially to ISO C "assert")
//
//     - wchar (can be mapped trivially to uint16_t)
//
//    A prototype standalone compiler exists in utils/avmc and demonstrates that
//    the independence works pretty well in practice.

#ifdef VMCFG_EVAL

#ifdef _MSC_VER
    #pragma warning(disable:4355) // 'this' : used in base member initializer list
    #pragma warning(disable:4345) // behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#endif

#ifdef DEBUG
#  define DEBUG_ONLY(x) x
#else
#  define DEBUG_ONLY(x)
#endif

namespace avmplus
{
    namespace RTC
    {
        using namespace ActionBlockConstants;
        
        // Types
        //
        // egrep '^class ' *.h | awk '{ print $2 }' | sort | awk '{ print "class " $1 ";" }'
        
        class ABCChunk;
        class ABCClassInfo;
        class ABCExceptionInfo;
        class ABCExceptionTable;
        class ABCFile;
        class ABCInstanceInfo;
        class ABCMetadataInfo;
        class ABCMethodBodyInfo;
        class ABCMethodInfo;
        class ABCMethodTrait;
        class ABCMultinameInfo;
        class ABCNamespaceInfo;
        class ABCNamespaceSetInfo;
        class ABCScriptInfo;
        class ABCSlotTrait;
        class ABCTrait;
        class ABCTraitsTable;
        class Allocator;
        class AssignExpr;
        class BinaryExpr;
        class Binding;
        class BlockStmt;
        class BreakStmt;
        class ByteBuffer;
        class CVAnonNS;
        class CVNamespace;
        class CVValue;
        class CallExpr;
        class CaseClause;
        class CatchClause;
        class ClassDefn;
        class CodeBlock;
        class Cogen;
        class ComputedName;
        class ConditionalExpr;
        class ConstValue;
        class ContinueStmt;
        class DefaultXmlNamespaceStmt;
        class DescendantsExpr;
        class DoWhileStmt;
        class EmptyStmt;
        class EscapeExpr;
        class Expr;
        class ExprStmt;
        class FilterExpr;
        class ForInStmt;
        class ForStmt;
        class FunctionDefn;
        class FunctionParam;
        class IfStmt;
        class ImportStmt;
        class InterfaceDefn;
        class Label;
        class LabelSetStmt;
        class LabeledStmt;
        class Lexer;
        class LiteralArray;
        class LiteralBoolean;
        class LiteralDouble;
        class LiteralField;
        class LiteralFunction;
        class LiteralInt;
        class LiteralNull;
        class LiteralObject;
        class LiteralRegExp;
        class LiteralString;
        class LiteralUInt;
        class LiteralUndefined;
        class NameComponent;
        class NameExpr;
        class NamespaceDefn;
        class NewExpr;
        class ObjectRef;
        class Parser;
        class Program;
        class QualifiedName;
        class Qualifier;
        class RefLocalExpr;
        class ReturnStmt;
        class SimpleName;
        class Stmt;
        class Str;
        class StringBuilder;
        class SwitchStmt;
        class ThisExpr;
        class ThrowStmt;
        class TryStmt;
        class UnaryExpr;
        class UseNamespaceStmt;
        class WhileStmt;
        class WildcardName;
        class WithStmt;
        class XmlInitializer;
        
        // some special cases
        
        class Compiler;
        class Ctx;
        class XmlContext;
        
        /**
         * Symbolic names for syntax error messages / format strings (localizable).  See
         * eval-compile.cpp for the actual contents, the English language strings define
         * the meaning of the symbols.
         */
        enum SyntaxError {
            SYNTAXERR_EOT_IN_REGEXP = 0,
            SYNTAXERR_NEWLINE_IN_REGEXP = 1,
            SYNTAXERR_UNEXPECTED_TOKEN_XML = 2,
            SYNTAXERR_NATIVE_NOT_SUPPORTED = 3,
            SYNTAXERR_DEFAULT_NOT_EXPECTED = 4,
            SYNTAXERR_ILLEGAL_QNAME = 5,
            SYNTAXERR_IMPOSSIBLE_DEFAULT = 6,
            SYNTAXERR_ILLEGAL_TYPENAME = 7,
            SYNTAXERR_ILLEGAL_FIELDNAME = 8,
            SYNTAXERR_ILLEGAL_PROPNAME = 9,
            SYNTAXERR_QUALIFIER_NOT_ALLOWED = 10,
            SYNTAXERR_ILLEGAL_INCLUDE = 11,
            SYNTAXERR_ILLEGAL_NAMESPACE = 12,
            SYNTAXERR_ILLEGAL_IN_INTERFACE = 13,
            SYNTAXERR_NOT_NATIVE_OR_PROTO = 14,
            SYNTAXERR_NO_FUNCTIONS_IN_BLOCKS = 15,
            SYNTAXERR_SEMICOLON_OR_NEWLINE = 16,
            SYNTAXERR_CONST_INIT_REQD = 17,
            SYNTAXERR_ILLEGAL_USE = 18,
            SYNTAXERR_RETURN_OUTSIDE_FN = 19,
            SYNTAXERR_VOIDFN_RETURNS_VALUE = 20,
            SYNTAXERR_EXPECT_DXNS = 21,
            SYNTAXERR_FOR_IN_ONEBINDING = 22,
            SYNTAXERR_FOR_EACH_REQS_IN = 23,
            SYNTAXERR_DUPLICATE_DEFAULT = 24,
            SYNTAXERR_EXPECT_CASE_OR_DEFAULT = 25,
            SYNTAXERR_CLASS_NOT_ALLOWED = 26,
            SYNTAXERR_CLASS_NATIVE = 27,
            SYNTAXERR_INTERFACE_NOT_ALLOWED = 28,
            SYNTAXERR_INTERFACE_NATIVE = 29,
            SYNTAXERR_STMT_IN_INTERFACE = 30,
            SYNTAXERR_ILLEGAL_STMT = 31,
            SYNTAXERR_KWD_NOT_ALLOWED = 32,
            SYNTAXERR_INCLUDE_ORIGIN = 33,
            SYNTAXERR_INCLUDE_INACCESSIBLE = 34,
            SYNTAXERR_REDEFINITION = 35,
            SYNTAXERR_REDEFINITION_TYPE = 36,
            SYNTAXERR_REDUNDANT_CONST = 37,
            SYNTAXERR_REDUNDANT_METHOD = 38,
            SYNTAXERR_REDUNDANT_NAMESPACE = 39,
            SYNTAXERR_DEFAULT_VALUE_REQD = 40,
            SYNTAXERR_WRONG_TOKEN = 41,
            SYNTAXERR_EXPECTED_IDENT = 42,
            SYNTAXERR_ILLEGALCHAR_NUL = 43,
            SYNTAXERR_ILLEGAL_NUMBER = 44,
            SYNTAXERR_ILLEGALCHAR_POSTNUMBER = 45,
            SYNTAXERR_EOI_IN_COMMENT = 46,
            SYNTAXERR_ILLEGALCHAR = 47,
            SYNTAXERR_UNTERMINATED_STRING = 48,
            SYNTAXERR_EOI_IN_ESC = 49,
            SYNTAXERR_UNTERMINATED_XML = 50,
            SYNTAXERR_INVALID_SLASH = 51,
            SYNTAXERR_INVALID_LEFTBANG = 52,
            SYNTAXERR_IDENT_IS_KWD = 53,
            SYNTAXERR_EOL_IN_ESC = 54,
            SYNTAXERR_INVALID_VAR_ESC = 55,
            SYNTAXERR_ILLEGAL_BREAK = 56,
            SYNTAXERR_BREAK_LABEL_UNDEF = 57,
            SYNTAXERR_ILLEGAL_CONTINUE = 58,
            SYNTAXERR_CONTINUE_LABEL_UNDEF = 59,
        };
        
        // The HostContext must be implemented by the embedder of eval.  'wchar' is a 16-bit unsigned value always.

        class HostContext {
        public:
#ifndef AVMC_STANDALONE
            // AvmCore is used for VMPI_alloca.
            HostContext(AvmCore* core) : core(core) {}
            AvmCore * const core;
#endif
            
            virtual ~HostContext() {};
            virtual uint8_t* obtainStorageForResult(uint32_t nbytes) = 0;
            virtual const wchar* readFileForEval(const wchar* basename, const wchar* filename, uint32_t* inputlen) = 0;
            virtual void freeInput(const wchar* input) = 0;
            virtual void doubleToString(double d, char* buf, size_t bufsiz) = 0;
            virtual bool stringToDouble(const char* s, double* d) = 0;
            virtual void throwInternalError(const char* msgz) = 0;
            virtual void throwSyntaxError(const char* msgz) = 0;
        };
        
        // type definitions
        
#include "eval-util.h"
#include "eval-lex.h"
#include "eval-parse.h"
#include "eval-cogen.h"
#include "eval-abc.h"
#include "eval-compile.h"
#include "eval-unicode.h"

        // all inline functions for those types

#include "eval-util-inlines.h"
#include "eval-abc-inlines.h"
#include "eval-lex-inlines.h"
#include "eval-parse-inlines.h"
#include "eval-cogen-inlines.h"

    }
}

#endif // VMCFG_EVAL
