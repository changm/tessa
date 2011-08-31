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

#include "avmplus.h"

#ifdef VMCFG_EVAL

#include "eval.h"

namespace avmplus
{
    namespace RTC
    {
        // Syntax error strings.  When these include formats they are to be interpreted as printf style
        // formats for now, notably %s means utf8 or latin1 string.  Note that a string may be used
        // more than one place, so if you add or remove or change a format be sure to look for all
        // occurences of the string.  Note also that order matters crucially, see the enum in eval.h.
        
        static const char* const syntax_errors[] = {
        /* SYNTAXERR_EOT_IN_REGEXP */        "Unexpected end of program in regexp literal",
        /* SYNTAXERR_NEWLINE_IN_REGEXP */    "Illegal newline in regexp literal",
        /* SYNTAXERR_UNEXPECTED_TOKEN_XML */ "Unexpected token in XML parsing",
        /* SYNTAXERR_NATIVE_NOT_SUPPORTED */ "'native' functions are not supported",
        /* SYNTAXERR_DEFAULT_NOT_EXPECTED */ "'default' not expected here",
        /* SYNTAXERR_ILLEGAL_QNAME */        "Illegal qualified name",
        /* SYNTAXERR_IMPOSSIBLE_DEFAULT */   "Unsupported default value (ABC restriction)",
        /* SYNTAXERR_ILLEGAL_TYPENAME */     "Illegal type name",
        /* SYNTAXERR_ILLEGAL_FIELDNAME */    "Illegal field name: String, number, or identifier is required",
        /* SYNTAXERR_ILLEGAL_PROPNAME */     "Illegal property identifier",
        /* SYNTAXERR_QUALIFIER_NOT_ALLOWED */"Qualifier not allowed here",
        /* SYNTAXERR_ILLEGAL_INCLUDE */      "Illegal 'include' directive",
        /* SYNTAXERR_ILLEGAL_NAMESPACE */    "Illegal 'namespace' directive",
        /* SYNTAXERR_ILLEGAL_IN_INTERFACE */ "Variable or constant definition not allowed in interface",
        /* SYNTAXERR_NOT_NATIVE_OR_PROTO */  "Variable or constant may not be 'native' or 'prototype'",
        /* SYNTAXERR_NO_FUNCTIONS_IN_BLOCKS*/"Function definitions cannot be block-local",
        /* SYNTAXERR_SEMICOLON_OR_NEWLINE */ "Expecting semicolon or newline",
        /* SYNTAXERR_CONST_INIT_REQD */      "'const' bindings must be initialized",
        /* SYNTAXERR_ILLEGAL_USE */          "Illegal 'use' directive",
        /* SYNTAXERR_RETURN_OUTSIDE_FN */    "'return' statement only allowed inside a function",
        /* SYNTAXERR_VOIDFN_RETURNS_VALUE */ "'void' function cannot return a value",
        /* SYNTAXERR_EXPECT_DXNS */          "Expected 'default xml namespace'",
        /* SYNTAXERR_FOR_IN_ONEBINDING */    "Only one variable binding allowed in for-in",
        /* SYNTAXERR_FOR_EACH_REQS_IN */     "'for each' requires use of the 'in' form",
        /* SYNTAXERR_DUPLICATE_DEFAULT */    "Duplicate 'default' clause",
        /* SYNTAXERR_EXPECT_CASE_OR_DEFAULT*/"Expecting 'case' or 'default'",
        /* SYNTAXERR_CLASS_NOT_ALLOWED */    "Class not allowed here",
        /* SYNTAXERR_CLASS_NATIVE */         "Class may not be 'native'",
        /* SYNTAXERR_INTERFACE_NOT_ALLOWED */"Interface not allowed here",
        /* SYNTAXERR_INTERFACE_NATIVE */     "Interface may not be 'native'",
        /* SYNTAXERR_STMT_IN_INTERFACE */    "Statements not allowed in interface",
        /* SYNTAXERR_ILLEGAL_STMT */         "Illegal statement",
        /* SYNTAXERR_KWD_NOT_ALLOWED */      "'%s' not allowed here",
        /* SYNTAXERR_INCLUDE_ORIGIN */       "The 'include' directive is only allowed in programs whose origin is a file",
        /* SYNTAXERR_INCLUDE_INACCESSIBLE */ "An include file could not be opened or read",
        /* SYNTAXERR_REDEFINITION */         "Conflicting binding of name",
        /* SYNTAXERR_REDEFINITION_TYPE */    "Conflicting binding of names: mismatching types",
        /* SYNTAXERR_REDUNDANT_CONST */      "Redundant constant binding",
        /* SYNTAXERR_REDUNDANT_METHOD */     "Redundant method binding",
        /* SYNTAXERR_REDUNDANT_NAMESPACE */  "Redundant namespace binding",
        /* SYNTAXERR_DEFAULT_VALUE_REQD */   "Default value required",
#ifdef DEBUG
        /* SYNTAXERR_WRONG_TOKEN */          "Wrong token - expected %d got %d",
#else
        /* SYNTAXERR_WRONG_TOKEN */          "Wrong token",
#endif
        /* SYNTAXERR_EXPECTED_IDENT */       "Expected identifier",
        /* SYNTAXERR_ILLEGALCHAR_NUL */      "Illegal character in input: NUL",
        /* SYNTAXERR_ILLEGAL_NUMBER */       "Illegal numeric literal: no digits",
        /* SYNTAXERR_ILLEGALCHAR_POSTNUMBER*/"Illegal character following numeric literal: %c",
        /* SYNTAXERR_EOI_IN_COMMENT */       "End of input in block comment",
        /* SYNTAXERR_ILLEGALCHAR */          "Illegal character in input: %c",
        /* SYNTAXERR_UNTERMINATED_STRING */  "Unterminated string literal (newline or end of input)",
        /* SYNTAXERR_EOI_IN_ESC */           "End of input in escape sequence",
        /* SYNTAXERR_UNTERMINATED_XML */     "Unterminated XML token",
        /* SYNTAXERR_INVALID_SLASH */        "Invalid sequence starting with '/'",
        /* SYNTAXERR_INVALID_LEFTBANG */     "Invalid sequence starting with '<!'",
        /* SYNTAXERR_IDENT_IS_KWD */         "Illegal identifier: escape sequence makes it look like a keyword",
        /* SYNTAXERR_EOL_IN_ESC */           "Illegal line terminator in escape sequence",
        /* SYNTAXERR_INVALID_VAR_ESC */      "Invalid variable-length unicode escape",
        /* SYNTAXERR_ILLEGAL_BREAK */        "No 'break' allowed here",
        /* SYNTAXERR_BREAK_LABEL_UNDEF */    "'break' to undefined label",
        /* SYNTAXERR_ILLEGAL_CONTINUE */     "No 'continue' allowed here", 
        /* SYNTAXERR_CONTINUE_LABEL_UNDEF */ "'continue' to undefined label",
        };
        
        // Assume that the number of unique identifiers in a program is roughly the square root
        // of the raw length of the program.  Needs experimental verification.  May only be
        // true for programs above a certain size; may want a lower limit on the table size.
        //
        // Using this "arbitrary" value for k we have to use mod when computing the hash bucket
        // when interning.  Faster would be to round k up to a power of 2, then use k-1 as a
        // mask in intern().

        static Str** makeStrTable(Allocator* allocator, uint32_t tableSize)
        {
            Str** strTable = (Str**)allocator->alloc(sizeof(Str*) * tableSize);
            for ( uint32_t i=0 ; i < tableSize ; i++ )
                strTable[i] = NULL;
            return strTable;
        }

        static const wchar default_filename[] = { '(', 'e', 'v', 'a', 'l', ' ', 's', 't', 'r', 'i', 'n', 'g', ')', 0 };
        
        Compiler::Compiler(HostContext* context, const wchar* filename, const wchar* src, uint32_t srclen)
            : context(context)
            , allocator(new Allocator(this))
            , filename(filename != NULL ? filename : default_filename)
            , tableSize(uint32_t(sqrt((double)srclen)))
            , es3_keywords(false)       // ActionScript: false
            , liberal_idents(true)      // ActionScript: true
            , local_functions(true)     // ActionScript: true
            , octal_literals(false)     // ActionScript: false
            , origin_is_file(filename != NULL)
            , debugging(true)
            , namespace_counter(1)
            , strTable(makeStrTable(allocator, tableSize))
            , lexer(this, src, srclen)
            , parser(this, &lexer)
            , abc(this)
            , SYM_(intern(""))
            , SYM_Array(intern("Array"))
            , SYM_Namespace(intern("Namespace"))
            , SYM_Number(intern("Number"))
            , SYM_Object(intern("Object"))
            , SYM_RegExp(intern("RegExp"))
            , SYM_XML(intern("XML"))
            , SYM_XMLList(intern("XMLList"))
            , SYM_anonymous(intern("anonymous"))
            , SYM_arguments(intern("arguments"))
            , SYM_children(intern("children"))
            , SYM_each(intern("each"))
            , SYM_get(intern("get"))
            , SYM_include(intern("include"))
            , SYM_int(intern("int"))
            , SYM_length(intern("length"))
            , SYM_namespace(intern("namespace"))
#ifdef DEBUG
            , SYM_print(intern("print"))
#endif
            , SYM_prototype(intern("prototype"))
            , SYM_set(intern("set"))
            , SYM_static(intern("static"))
            , SYM_to(intern("to"))
            , SYM_use(intern("use"))
            , SYM_xml(intern("xml"))
            , str_filename(intern(this->filename, NULL))
            , NS_public(abc.addNamespace(CONSTANT_Namespace, abc.addString(SYM_)))
            , ID_Array(abc.addQName(NS_public, abc.addString(SYM_Array)))
            , ID_Namespace(abc.addQName(NS_public, abc.addString(SYM_Namespace)))
            , ID_Number(abc.addQName(NS_public, abc.addString(SYM_Number)))
            , ID_Object(abc.addQName(NS_public, abc.addString(SYM_Object)))
            , ID_RegExp(abc.addQName(NS_public, abc.addString(SYM_RegExp)))
            , ID_XML(abc.addQName(NS_public, abc.addString(SYM_XML)))
            , ID_XMLList(abc.addQName(NS_public, abc.addString(SYM_XMLList)))
            , ID_children(abc.addQName(NS_public, abc.addString(SYM_children)))
            , ID_int(abc.addQName(NS_public, abc.addString(SYM_int)))
            , ID_length(abc.addQName(NS_public, abc.addString(SYM_length)))
#ifdef DEBUG
            , ID_print(abc.addQName(NS_public, abc.addString(SYM_print)))
#endif
            , NSS_public(abc.addNsset(ALLOC(Seq<uint32_t>, (NS_public))))
            , MNL_public(abc.addMultinameL(NSS_public, false))
            , MNL_public_attr(abc.addMultinameL(NSS_public, true))
        {
        }

        void Compiler::destroy()
        {
            delete allocator;
        }

        void Compiler::compile()
        {
#if 0 && defined DEBUG
            lexer.trace();
#endif
            Program* program = parser.parse();

            ABCTraitsTable* global_traits = ALLOC(ABCTraitsTable, (this));
            ABCMethodInfo* global_info = ALLOC(ABCMethodInfo, (this, abc.addString("script$init"), 0, NULL, 0, NULL, 0));
            ABCMethodBodyInfo* global_body = ALLOC(ABCMethodBodyInfo, (this, global_info, global_traits, 1));
            program->cogen(&global_body->cogen);
            global_info->setFlags(global_body->getFlags() | MethodInfo::SETS_DXNS);

            ALLOC(ABCScriptInfo, (this, global_info, global_traits));

            global_body->clearTraits();     // the traits belong to the script, so don't emit them with the body too
            
            abc.serialize(context->obtainStorageForResult(abc.size()));
        }

        void Compiler::internalError(uint32_t lineno, const char* fmt, ...)
        {
            char buf[500];
            char lbuf[12];
            if (lineno != 0)
                VMPI_sprintf(lbuf, "%d", lineno);
            else
                VMPI_strcpy(lbuf, "Unknown");
            {
                char fbuf[500];
                formatUtf8(fbuf, sizeof(fbuf), filename);
                VMPI_snprintf(buf, sizeof(buf), "%s:%s: Internal error: ", fbuf, lbuf);
                buf[sizeof(buf)-1] = 0;
            }
            int k = int(VMPI_strlen(buf));
            va_list args;
            va_start(args,fmt);
            VMPI_vsnprintf(buf+k, sizeof(buf)-k, fmt, args);
            va_end(args);
            
            context->throwInternalError(buf);
        }
        
        void Compiler::syntaxError(uint32_t lineno, SyntaxError fmt, ...)
        {
            const char* fmtstring = syntax_errors[(int)fmt];
            char buf[500];
            char lbuf[12];
            if (lineno != 0)
                VMPI_sprintf(lbuf, "%d", lineno);
            else
                VMPI_strcpy(lbuf, "Unknown");
            {
                char fbuf[500];
                formatUtf8(fbuf, sizeof(fbuf), filename);
                VMPI_snprintf(buf, sizeof(buf), "%s:%s: Syntax error: ", fbuf, lbuf);
                buf[sizeof(buf)-1] = 0;
            }
            int k = int(VMPI_strlen(buf));
            va_list args;
            va_start(args,fmt);
            VMPI_vsnprintf(buf+k, sizeof(buf)-k, fmtstring, args);
            va_end(args);

            context->throwSyntaxError(buf);
        }

        Str* Compiler::intern(const char* s)
        {
            StringBuilder b(this);
            b.append(s);
            return b.str();
        }

        Str* Compiler::intern(uint32_t u)
        {
            wchar buf[12];
            uint32_t i = 0;
            if (u == 0)
                buf[i++] = '0';
            else {
                while (u != 0) {
                    buf[i++] = '0' + (u % 10);
                    u /= 10;
                }
                for ( uint32_t j=0, k=i-1 ; j < k ; j++, k-- ) {
                    wchar c = buf[j];
                    buf[j] = buf[k];
                    buf[k] = c;
                }
            }
            return intern(buf, i);
        }

        Str* Compiler::intern(const wchar* chars, uint32_t nchars)
        {
            // '%' may not be the best here, see comment above about how to speed it up
            // if it turns out to be a bottleneck.
            
            uint32_t h = hashString(chars, nchars) % tableSize;
            for ( Str* p = strTable[h] ; p != NULL ; p = p->next ) {
                if (p->hash == h) 
                    if (p->length == nchars)
                        if (VMPI_memcmp(p->s, chars, sizeof(wchar)*nchars) == 0)
                            return p;
            }

            // The string is not known yet

            Str* str = (Str*)allocator->alloc(sizeof(Str) + sizeof(wchar)*(nchars - 1 + 1));    // -1 for the elt in Str, +1 for the NUL
            VMPI_memcpy(str->s, chars, sizeof(wchar)*nchars);
            str->s[nchars] = 0;
            str->hash = h;
            str->ident = ~0U;
            str->next = strTable[h];
            str->length = nchars;
            strTable[h] = str;
#if 0 && defined DEBUG
            char buf[200];
            printf(">>> Interning: %s\n", getn(buf, str, sizeof(buf)));
#endif
            return str;
        }
    }
}

#endif // VMCFG_EVAL
