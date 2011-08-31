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
 * The Original Code is [Open Source Virtual Machine].
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

/* machine generated file -- do not edit */

#define AVMTHUNK_VERSION 5

namespace avmplus {
    class ArgumentErrorClass; //ArgumentError$
    class ArgumentErrorObject; //ArgumentError
    class ArrayClass; //Array$
    class ArrayObject; //Array
    class BooleanClass; //Boolean$
    class ClassClass; //Class$
    class ClassClosure; //Class
    class DateClass; //Date$
    class DateObject; //Date
    class DefinitionErrorClass; //DefinitionError$
    class DefinitionErrorObject; //DefinitionError
    class DoubleVectorClass; //__AS3__.vec::Vector$double$
    class DoubleVectorObject; //__AS3__.vec::Vector$double
    class ErrorClass; //Error$
    class ErrorObject; //Error
    class EvalErrorClass; //EvalError$
    class EvalErrorObject; //EvalError
    class FunctionClass; //Function$
    class FunctionObject; //Function
    class IntClass; //int$
    class IntVectorClass; //__AS3__.vec::Vector$int$
    class IntVectorObject; //__AS3__.vec::Vector$int
    class MathClass; //Math$
    class MethodClosure; //private::MethodClosure
    class MethodClosureClass; //private::MethodClosure$
    class Namespace; //Namespace
    class NamespaceClass; //Namespace$
    class NumberClass; //Number$
    class ObjectClass; //Object$
    class ObjectVectorClass; //__AS3__.vec::Vector$object$
    class ObjectVectorObject; //__AS3__.vec::Vector
    class QNameClass; //QName$
    class QNameObject; //QName
    class RangeErrorClass; //RangeError$
    class RangeErrorObject; //RangeError
    class ReferenceErrorClass; //ReferenceError$
    class ReferenceErrorObject; //ReferenceError
    class RegExpClass; //RegExp$
    class RegExpObject; //RegExp
    class SecurityErrorClass; //SecurityError$
    class SecurityErrorObject; //SecurityError
    class String; //String
    class StringClass; //String$
    class SyntaxErrorClass; //SyntaxError$
    class SyntaxErrorObject; //SyntaxError
    class TypeErrorClass; //TypeError$
    class TypeErrorObject; //TypeError
    class UIntClass; //uint$
    class UIntVectorClass; //__AS3__.vec::Vector$uint$
    class UIntVectorObject; //__AS3__.vec::Vector$uint
    class URIErrorClass; //URIError$
    class URIErrorObject; //URIError
    class UninitializedErrorClass; //UninitializedError$
    class UninitializedErrorObject; //UninitializedError
    class VectorClass; //__AS3__.vec::Vector$
    class VerifyErrorClass; //VerifyError$
    class VerifyErrorObject; //VerifyError
    class XMLClass; //XML$
    class XMLListClass; //XMLList$
    class XMLListObject; //XMLList
    class XMLObject; //XML
}

namespace avmplus { namespace NativeID {

extern const uint32_t builtin_abc_class_count;
extern const uint32_t builtin_abc_script_count;
extern const uint32_t builtin_abc_method_count;
extern const uint32_t builtin_abc_length;
extern const uint8_t builtin_abc_data[];
AVMTHUNK_DECLARE_NATIVE_INITIALIZER(builtin)

/* classes */
const uint32_t abcclass_Object = 0;
const uint32_t abcclass_Class = 1;
const uint32_t abcclass_Function = 2;
const uint32_t abcclass_Namespace = 3;
const uint32_t abcclass_Boolean = 4;
const uint32_t abcclass_Number = 5;
const uint32_t abcclass_int = 6;
const uint32_t abcclass_uint = 7;
const uint32_t abcclass_String = 8;
const uint32_t abcclass_Array = 9;
const uint32_t abcclass___AS3___vec_Vector = 10;
const uint32_t abcclass___AS3___vec_Vector_object = 11;
const uint32_t abcclass___AS3___vec_Vector_int = 12;
const uint32_t abcclass___AS3___vec_Vector_uint = 13;
const uint32_t abcclass___AS3___vec_Vector_double = 14;
const uint32_t abcclass_private_MethodClosure = 15;
const uint32_t abcclass_Math = 16;
const uint32_t abcclass_Error = 17;
const uint32_t abcclass_DefinitionError = 18;
const uint32_t abcclass_EvalError = 19;
const uint32_t abcclass_RangeError = 20;
const uint32_t abcclass_ReferenceError = 21;
const uint32_t abcclass_SecurityError = 22;
const uint32_t abcclass_SyntaxError = 23;
const uint32_t abcclass_TypeError = 24;
const uint32_t abcclass_URIError = 25;
const uint32_t abcclass_VerifyError = 26;
const uint32_t abcclass_UninitializedError = 27;
const uint32_t abcclass_ArgumentError = 28;
const uint32_t abcclass_Date = 29;
const uint32_t abcclass_RegExp = 30;
const uint32_t abcclass_XML = 31;
const uint32_t abcclass_XMLList = 32;
const uint32_t abcclass_QName = 33;

/* methods */
const uint32_t native_script_function_avmplus_getQualifiedSuperclassName = 0;
const uint32_t native_script_function_avmplus_getQualifiedClassName = 1;
const uint32_t native_script_function_avmplus_describeTypeJSON = 7;
const uint32_t native_script_function_isXMLName = 8;
const uint32_t native_script_function_unescape = 9;
const uint32_t native_script_function_escape = 10;
const uint32_t native_script_function_parseFloat = 11;
const uint32_t native_script_function_parseInt = 12;
const uint32_t native_script_function_isFinite = 13;
const uint32_t native_script_function_isNaN = 14;
const uint32_t native_script_function_encodeURIComponent = 15;
const uint32_t native_script_function_encodeURI = 16;
const uint32_t native_script_function_decodeURIComponent = 17;
const uint32_t native_script_function_decodeURI = 18;
const uint32_t Object_private__hasOwnProperty = 20;
const uint32_t Object_private__propertyIsEnumerable = 21;
const uint32_t Object_protected__setPropertyIsEnumerable = 22;
const uint32_t Object_private__isPrototypeOf = 23;
const uint32_t Object_private__toString = 24;
const uint32_t Class_prototype_get = 38;
const uint32_t Function_prototype_get = 46;
const uint32_t Function_prototype_set = 47;
const uint32_t Function_length_get = 48;
const uint32_t Function_AS3_call = 49;
const uint32_t Function_AS3_apply = 50;
const uint32_t Namespace_prefix_get = 55;
const uint32_t Namespace_uri_get = 56;
const uint32_t Number_private__numberToString = 72;
const uint32_t Number_private__convert = 73;
const uint32_t Number_private__minValue = 74;
const uint32_t String_AS3_fromCharCode = 124;
const uint32_t String_private__match = 125;
const uint32_t String_private__replace = 126;
const uint32_t String_private__search = 127;
const uint32_t String_private__split = 128;
const uint32_t String_length_get = 129;
const uint32_t String_private__indexOf = 130;
const uint32_t String_AS3_indexOf = 131;
const uint32_t String_private__lastIndexOf = 132;
const uint32_t String_AS3_lastIndexOf = 133;
const uint32_t String_private__charAt = 134;
const uint32_t String_AS3_charAt = 135;
const uint32_t String_private__charCodeAt = 136;
const uint32_t String_AS3_charCodeAt = 137;
const uint32_t String_AS3_localeCompare = 139;
const uint32_t String_private__slice = 143;
const uint32_t String_AS3_slice = 144;
const uint32_t String_private__substring = 146;
const uint32_t String_AS3_substring = 147;
const uint32_t String_private__substr = 148;
const uint32_t String_AS3_substr = 149;
const uint32_t String_AS3_toLowerCase = 150;
const uint32_t String_AS3_toUpperCase = 152;
const uint32_t Array_private__pop = 179;
const uint32_t Array_private__reverse = 180;
const uint32_t Array_private__concat = 181;
const uint32_t Array_private__shift = 182;
const uint32_t Array_private__slice = 183;
const uint32_t Array_private__unshift = 184;
const uint32_t Array_private__splice = 185;
const uint32_t Array_private__sort = 186;
const uint32_t Array_private__sortOn = 187;
const uint32_t Array_private__indexOf = 188;
const uint32_t Array_private__lastIndexOf = 189;
const uint32_t Array_private__every = 190;
const uint32_t Array_private__filter = 191;
const uint32_t Array_private__forEach = 192;
const uint32_t Array_private__map = 193;
const uint32_t Array_private__some = 194;
const uint32_t Array_length_get = 195;
const uint32_t Array_length_set = 196;
const uint32_t Array_AS3_pop = 199;
const uint32_t Array_AS3_push = 200;
const uint32_t Array_AS3_unshift = 205;
const uint32_t __AS3___vec_Vector_object_private__every = 239;
const uint32_t __AS3___vec_Vector_object_private__forEach = 240;
const uint32_t __AS3___vec_Vector_object_private__some = 241;
const uint32_t __AS3___vec_Vector_object_private__sort = 242;
const uint32_t __AS3___vec_Vector_object_private_type_set = 244;
const uint32_t __AS3___vec_Vector_object_private_type_get = 245;
const uint32_t __AS3___vec_Vector_object_length_get = 247;
const uint32_t __AS3___vec_Vector_object_length_set = 248;
const uint32_t __AS3___vec_Vector_object_fixed_set = 249;
const uint32_t __AS3___vec_Vector_object_fixed_get = 250;
const uint32_t __AS3___vec_Vector_object_AS3_push = 258;
const uint32_t __AS3___vec_Vector_object_private__reverse = 259;
const uint32_t __AS3___vec_Vector_object_private__spliceHelper = 263;
const uint32_t __AS3___vec_Vector_object_AS3_unshift = 265;
const uint32_t __AS3___vec_Vector_object_private__filter = 266;
const uint32_t __AS3___vec_Vector_object_private__map = 267;
const uint32_t __AS3___vec_Vector_object_AS3_pop = 271;
const uint32_t __AS3___vec_Vector_int_private__every = 301;
const uint32_t __AS3___vec_Vector_int_private__forEach = 302;
const uint32_t __AS3___vec_Vector_int_private__some = 303;
const uint32_t __AS3___vec_Vector_int_private__sort = 304;
const uint32_t __AS3___vec_Vector_int_length_get = 306;
const uint32_t __AS3___vec_Vector_int_length_set = 307;
const uint32_t __AS3___vec_Vector_int_fixed_set = 308;
const uint32_t __AS3___vec_Vector_int_fixed_get = 309;
const uint32_t __AS3___vec_Vector_int_AS3_push = 317;
const uint32_t __AS3___vec_Vector_int_private__reverse = 318;
const uint32_t __AS3___vec_Vector_int_private__spliceHelper = 322;
const uint32_t __AS3___vec_Vector_int_AS3_unshift = 324;
const uint32_t __AS3___vec_Vector_int_private__filter = 325;
const uint32_t __AS3___vec_Vector_int_private__map = 326;
const uint32_t __AS3___vec_Vector_int_AS3_pop = 330;
const uint32_t __AS3___vec_Vector_uint_private__every = 360;
const uint32_t __AS3___vec_Vector_uint_private__forEach = 361;
const uint32_t __AS3___vec_Vector_uint_private__some = 362;
const uint32_t __AS3___vec_Vector_uint_private__sort = 363;
const uint32_t __AS3___vec_Vector_uint_length_get = 365;
const uint32_t __AS3___vec_Vector_uint_length_set = 366;
const uint32_t __AS3___vec_Vector_uint_fixed_set = 367;
const uint32_t __AS3___vec_Vector_uint_fixed_get = 368;
const uint32_t __AS3___vec_Vector_uint_AS3_push = 376;
const uint32_t __AS3___vec_Vector_uint_private__reverse = 377;
const uint32_t __AS3___vec_Vector_uint_private__spliceHelper = 381;
const uint32_t __AS3___vec_Vector_uint_AS3_unshift = 383;
const uint32_t __AS3___vec_Vector_uint_private__filter = 384;
const uint32_t __AS3___vec_Vector_uint_private__map = 385;
const uint32_t __AS3___vec_Vector_uint_AS3_pop = 389;
const uint32_t __AS3___vec_Vector_double_private__every = 419;
const uint32_t __AS3___vec_Vector_double_private__forEach = 420;
const uint32_t __AS3___vec_Vector_double_private__some = 421;
const uint32_t __AS3___vec_Vector_double_private__sort = 422;
const uint32_t __AS3___vec_Vector_double_length_get = 424;
const uint32_t __AS3___vec_Vector_double_length_set = 425;
const uint32_t __AS3___vec_Vector_double_fixed_set = 426;
const uint32_t __AS3___vec_Vector_double_fixed_get = 427;
const uint32_t __AS3___vec_Vector_double_AS3_push = 435;
const uint32_t __AS3___vec_Vector_double_private__reverse = 436;
const uint32_t __AS3___vec_Vector_double_private__spliceHelper = 440;
const uint32_t __AS3___vec_Vector_double_AS3_unshift = 442;
const uint32_t __AS3___vec_Vector_double_private__filter = 443;
const uint32_t __AS3___vec_Vector_double_private__map = 444;
const uint32_t __AS3___vec_Vector_double_AS3_pop = 448;
const uint32_t Math_private__min = 462;
const uint32_t Math_private__max = 463;
const uint32_t Math_abs = 464;
const uint32_t Math_acos = 465;
const uint32_t Math_asin = 466;
const uint32_t Math_atan = 467;
const uint32_t Math_ceil = 468;
const uint32_t Math_cos = 469;
const uint32_t Math_exp = 470;
const uint32_t Math_floor = 471;
const uint32_t Math_log = 472;
const uint32_t Math_round = 473;
const uint32_t Math_sin = 474;
const uint32_t Math_sqrt = 475;
const uint32_t Math_tan = 476;
const uint32_t Math_atan2 = 477;
const uint32_t Math_pow = 478;
const uint32_t Math_max = 479;
const uint32_t Math_min = 480;
const uint32_t Math_random = 481;
const uint32_t Error_getErrorMessage = 486;
const uint32_t Error_getStackTrace = 490;
const uint32_t Date_parse = 557;
const uint32_t Date_UTC = 558;
const uint32_t Date_AS3_valueOf = 559;
const uint32_t Date_private__toString = 560;
const uint32_t Date_private__setTime = 561;
const uint32_t Date_private__get = 562;
const uint32_t Date_AS3_getUTCFullYear = 571;
const uint32_t Date_AS3_getUTCMonth = 572;
const uint32_t Date_AS3_getUTCDate = 573;
const uint32_t Date_AS3_getUTCDay = 574;
const uint32_t Date_AS3_getUTCHours = 575;
const uint32_t Date_AS3_getUTCMinutes = 576;
const uint32_t Date_AS3_getUTCSeconds = 577;
const uint32_t Date_AS3_getUTCMilliseconds = 578;
const uint32_t Date_AS3_getFullYear = 579;
const uint32_t Date_AS3_getMonth = 580;
const uint32_t Date_AS3_getDate = 581;
const uint32_t Date_AS3_getDay = 582;
const uint32_t Date_AS3_getHours = 583;
const uint32_t Date_AS3_getMinutes = 584;
const uint32_t Date_AS3_getSeconds = 585;
const uint32_t Date_AS3_getMilliseconds = 586;
const uint32_t Date_AS3_getTimezoneOffset = 587;
const uint32_t Date_AS3_getTime = 588;
const uint32_t Date_private__setFullYear = 589;
const uint32_t Date_private__setMonth = 590;
const uint32_t Date_private__setDate = 591;
const uint32_t Date_private__setHours = 592;
const uint32_t Date_private__setMinutes = 593;
const uint32_t Date_private__setSeconds = 594;
const uint32_t Date_private__setMilliseconds = 595;
const uint32_t Date_private__setUTCFullYear = 596;
const uint32_t Date_private__setUTCMonth = 597;
const uint32_t Date_private__setUTCDate = 598;
const uint32_t Date_private__setUTCHours = 599;
const uint32_t Date_private__setUTCMinutes = 600;
const uint32_t Date_private__setUTCSeconds = 601;
const uint32_t Date_private__setUTCMilliseconds = 602;
const uint32_t RegExp_source_get = 656;
const uint32_t RegExp_global_get = 657;
const uint32_t RegExp_ignoreCase_get = 658;
const uint32_t RegExp_multiline_get = 659;
const uint32_t RegExp_lastIndex_get = 660;
const uint32_t RegExp_lastIndex_set = 661;
const uint32_t RegExp_dotall_get = 662;
const uint32_t RegExp_extended_get = 663;
const uint32_t RegExp_AS3_exec = 664;
const uint32_t XML_ignoreComments_get = 710;
const uint32_t XML_ignoreComments_set = 711;
const uint32_t XML_ignoreProcessingInstructions_get = 712;
const uint32_t XML_ignoreProcessingInstructions_set = 713;
const uint32_t XML_ignoreWhitespace_get = 714;
const uint32_t XML_ignoreWhitespace_set = 715;
const uint32_t XML_prettyPrinting_get = 716;
const uint32_t XML_prettyPrinting_set = 717;
const uint32_t XML_prettyIndent_get = 718;
const uint32_t XML_prettyIndent_set = 719;
const uint32_t XML_AS3_toString = 723;
const uint32_t XML_AS3_hasOwnProperty = 724;
const uint32_t XML_AS3_propertyIsEnumerable = 725;
const uint32_t XML_AS3_addNamespace = 726;
const uint32_t XML_AS3_appendChild = 727;
const uint32_t XML_AS3_attribute = 728;
const uint32_t XML_AS3_attributes = 729;
const uint32_t XML_AS3_child = 730;
const uint32_t XML_AS3_childIndex = 731;
const uint32_t XML_AS3_children = 732;
const uint32_t XML_AS3_comments = 733;
const uint32_t XML_AS3_contains = 734;
const uint32_t XML_AS3_copy = 735;
const uint32_t XML_AS3_descendants = 736;
const uint32_t XML_AS3_elements = 737;
const uint32_t XML_AS3_hasComplexContent = 738;
const uint32_t XML_AS3_hasSimpleContent = 739;
const uint32_t XML_AS3_inScopeNamespaces = 740;
const uint32_t XML_AS3_insertChildAfter = 741;
const uint32_t XML_AS3_insertChildBefore = 742;
const uint32_t XML_AS3_localName = 744;
const uint32_t XML_AS3_name = 745;
const uint32_t XML_private__namespace = 746;
const uint32_t XML_AS3_namespaceDeclarations = 748;
const uint32_t XML_AS3_nodeKind = 749;
const uint32_t XML_AS3_normalize = 750;
const uint32_t XML_AS3_parent = 751;
const uint32_t XML_AS3_processingInstructions = 752;
const uint32_t XML_AS3_prependChild = 753;
const uint32_t XML_AS3_removeNamespace = 754;
const uint32_t XML_AS3_replace = 755;
const uint32_t XML_AS3_setChildren = 756;
const uint32_t XML_AS3_setLocalName = 757;
const uint32_t XML_AS3_setName = 758;
const uint32_t XML_AS3_setNamespace = 759;
const uint32_t XML_AS3_text = 760;
const uint32_t XML_AS3_toXMLString = 761;
const uint32_t XML_AS3_notification = 762;
const uint32_t XML_AS3_setNotification = 763;
const uint32_t XMLList_AS3_toString = 805;
const uint32_t XMLList_AS3_hasOwnProperty = 807;
const uint32_t XMLList_AS3_propertyIsEnumerable = 808;
const uint32_t XMLList_AS3_attribute = 809;
const uint32_t XMLList_AS3_attributes = 810;
const uint32_t XMLList_AS3_child = 811;
const uint32_t XMLList_AS3_children = 812;
const uint32_t XMLList_AS3_comments = 813;
const uint32_t XMLList_AS3_contains = 814;
const uint32_t XMLList_AS3_copy = 815;
const uint32_t XMLList_AS3_descendants = 816;
const uint32_t XMLList_AS3_elements = 817;
const uint32_t XMLList_AS3_hasComplexContent = 818;
const uint32_t XMLList_AS3_hasSimpleContent = 819;
const uint32_t XMLList_AS3_length = 820;
const uint32_t XMLList_AS3_name = 821;
const uint32_t XMLList_AS3_normalize = 822;
const uint32_t XMLList_AS3_parent = 823;
const uint32_t XMLList_AS3_processingInstructions = 824;
const uint32_t XMLList_AS3_text = 825;
const uint32_t XMLList_AS3_toXMLString = 826;
const uint32_t XMLList_AS3_addNamespace = 827;
const uint32_t XMLList_AS3_appendChild = 828;
const uint32_t XMLList_AS3_childIndex = 829;
const uint32_t XMLList_AS3_inScopeNamespaces = 830;
const uint32_t XMLList_AS3_insertChildAfter = 831;
const uint32_t XMLList_AS3_insertChildBefore = 832;
const uint32_t XMLList_AS3_nodeKind = 833;
const uint32_t XMLList_private__namespace = 834;
const uint32_t XMLList_AS3_localName = 836;
const uint32_t XMLList_AS3_namespaceDeclarations = 837;
const uint32_t XMLList_AS3_prependChild = 838;
const uint32_t XMLList_AS3_removeNamespace = 839;
const uint32_t XMLList_AS3_replace = 840;
const uint32_t XMLList_AS3_setChildren = 841;
const uint32_t XMLList_AS3_setLocalName = 842;
const uint32_t XMLList_AS3_setName = 843;
const uint32_t XMLList_AS3_setNamespace = 844;
const uint32_t QName_localName_get = 848;
const uint32_t QName_uri_get = 849;

extern double Math_private__min_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_private__max_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_abs_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_acos_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_asin_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_atan_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_ceil_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_cos_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_exp_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_floor_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_log_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_round_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_sin_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_sqrt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_tan_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_atan2_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_pow_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_max_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_min_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Math_random_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Error_getErrorMessage_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Error_getStackTrace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_parse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_UTC_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_valueOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Date_private__toString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setTime_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCFullYear_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCMonth_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCDate_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCDay_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCHours_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCMinutes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCSeconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getUTCMilliseconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getFullYear_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getMonth_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getDate_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getDay_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getHours_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getMinutes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getSeconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getMilliseconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getTimezoneOffset_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_AS3_getTime_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setFullYear_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setMonth_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setDate_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setHours_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setMinutes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setSeconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setMilliseconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCFullYear_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCMonth_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCDate_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCHours_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCMinutes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCSeconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Date_private__setUTCMilliseconds_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_source_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_global_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_ignoreCase_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_multiline_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_lastIndex_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_lastIndex_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_dotall_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_extended_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox RegExp_AS3_exec_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreComments_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreComments_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreProcessingInstructions_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreProcessingInstructions_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreWhitespace_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_ignoreWhitespace_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_prettyPrinting_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_prettyPrinting_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_prettyIndent_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_prettyIndent_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_toString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_hasOwnProperty_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_propertyIsEnumerable_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_addNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_appendChild_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_attribute_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_attributes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_child_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_childIndex_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_children_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_comments_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_contains_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_copy_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_descendants_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_elements_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_hasComplexContent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_hasSimpleContent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_inScopeNamespaces_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_insertChildAfter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_insertChildBefore_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_localName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_name_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_private__namespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_namespaceDeclarations_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_nodeKind_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_normalize_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_parent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_processingInstructions_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_prependChild_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_removeNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_replace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_setChildren_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_setLocalName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_setName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_setNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_text_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_toXMLString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_notification_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XML_AS3_setNotification_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_toString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_hasOwnProperty_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_propertyIsEnumerable_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_attribute_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_attributes_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_child_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_children_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_comments_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_contains_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_copy_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_descendants_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_elements_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_hasComplexContent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_hasSimpleContent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_length_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_name_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_normalize_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_parent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_processingInstructions_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_text_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_toXMLString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_addNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_appendChild_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_childIndex_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_inScopeNamespaces_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_insertChildAfter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_insertChildBefore_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_nodeKind_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_private__namespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_localName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_namespaceDeclarations_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_prependChild_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_removeNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_replace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_setChildren_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_setLocalName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_setName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox XMLList_AS3_setNamespace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox QName_localName_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox QName_uri_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Object_private__hasOwnProperty_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Object_private__propertyIsEnumerable_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Object_protected__setPropertyIsEnumerable_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Object_private__isPrototypeOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Object_private__toString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Class_prototype_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Function_prototype_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Function_prototype_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Function_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Function_AS3_call_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Function_AS3_apply_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Namespace_prefix_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Namespace_uri_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Number_private__numberToString_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Number_private__convert_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double Number_private__minValue_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_fromCharCode_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__match_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__replace_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__search_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__split_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__indexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_indexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__lastIndexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_lastIndexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__charAt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_charAt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double String_private__charCodeAt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double String_AS3_charCodeAt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_localeCompare_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__slice_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_slice_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__substring_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_substring_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_private__substr_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_substr_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_toLowerCase_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox String_AS3_toUpperCase_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__reverse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__concat_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__shift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__slice_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__splice_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__sort_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__sortOn_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__indexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__lastIndexOf_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__every_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__filter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__forEach_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__map_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_private__some_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_length_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_AS3_pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_AS3_push_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox Array_AS3_unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_decodeURI_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_decodeURIComponent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_encodeURI_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_encodeURIComponent_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_isNaN_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_isFinite_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double native_script_function_parseInt_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double native_script_function_parseFloat_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_escape_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_unescape_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_isXMLName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__every_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__forEach_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__some_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__sort_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private_type_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private_type_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_length_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_fixed_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_fixed_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_AS3_push_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__reverse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__spliceHelper_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_AS3_unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__filter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_private__map_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_object_AS3_pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__every_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__forEach_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__some_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__sort_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_length_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_fixed_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_fixed_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_AS3_push_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__reverse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__spliceHelper_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_AS3_unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__filter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_private__map_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_int_AS3_pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__every_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__forEach_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__some_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__sort_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_length_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_fixed_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_fixed_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_AS3_push_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__reverse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__spliceHelper_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_AS3_unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__filter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_private__map_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_uint_AS3_pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__every_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__forEach_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__some_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__sort_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_length_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_length_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_fixed_set_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_fixed_get_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_AS3_push_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__reverse_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__spliceHelper_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_AS3_unshift_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__filter_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox __AS3___vec_Vector_double_private__map_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern double __AS3___vec_Vector_double_AS3_pop_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_avmplus_describeTypeJSON_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_avmplus_getQualifiedClassName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
extern AvmBox native_script_function_avmplus_getQualifiedSuperclassName_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);

#ifdef VMCFG_INDIRECT_NATIVE_THUNKS

extern AvmBox builtin_a2a_oaoa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__filter_thunk  builtin_a2a_oaoa_thunk
#define Array_private__map_thunk  builtin_a2a_oaoa_thunk

extern AvmBox builtin_v2a_ouuuai_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_object_private__spliceHelper_thunk  builtin_v2a_ouuuai_thunk
#define __AS3___vec_Vector_uint_private__spliceHelper_thunk  builtin_v2a_ouuuai_thunk
#define __AS3___vec_Vector_int_private__spliceHelper_thunk  builtin_v2a_ouuuai_thunk
#define __AS3___vec_Vector_double_private__spliceHelper_thunk  builtin_v2a_ouuuai_thunk

extern double builtin_d2d_si_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__charCodeAt_thunk  builtin_d2d_si_opti0_thunk

extern double builtin_d2d_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_AS3_getUTCDate_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCMonth_thunk  builtin_d2d_o_thunk
#define Date_AS3_getHours_thunk  builtin_d2d_o_thunk
#define Date_AS3_getFullYear_thunk  builtin_d2d_o_thunk
#define Date_AS3_valueOf_thunk  builtin_d2d_o_thunk
#define Date_AS3_getSeconds_thunk  builtin_d2d_o_thunk
#define Date_AS3_getTimezoneOffset_thunk  builtin_d2d_o_thunk
#define Date_AS3_getMinutes_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCSeconds_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCMinutes_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCFullYear_thunk  builtin_d2d_o_thunk
#define Date_AS3_getMilliseconds_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCDay_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCHours_thunk  builtin_d2d_o_thunk
#define Number_private__minValue_thunk  builtin_d2d_o_thunk
#define Date_AS3_getTime_thunk  builtin_d2d_o_thunk
#define Date_AS3_getDay_thunk  builtin_d2d_o_thunk
#define __AS3___vec_Vector_double_AS3_pop_thunk  builtin_d2d_o_thunk
#define Date_AS3_getUTCMilliseconds_thunk  builtin_d2d_o_thunk
#define Date_AS3_getMonth_thunk  builtin_d2d_o_thunk
#define Math_random_thunk  builtin_d2d_o_thunk
#define Date_AS3_getDate_thunk  builtin_d2d_o_thunk

extern AvmBox builtin_b2a_oa_optakAvmThunkUndefined_u_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_AS3_propertyIsEnumerable_thunk  builtin_b2a_oa_optakAvmThunkUndefined_u_thunk
#define XMLList_AS3_propertyIsEnumerable_thunk  builtin_b2a_oa_optakAvmThunkUndefined_u_thunk
#define XML_AS3_hasOwnProperty_thunk  builtin_b2a_oa_optakAvmThunkUndefined_u_thunk
#define XMLList_AS3_hasOwnProperty_thunk  builtin_b2a_oa_optakAvmThunkUndefined_u_thunk

extern AvmBox builtin_i2a_ss_optakAvmThunkUndefined_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_localeCompare_thunk  builtin_i2a_ss_optakAvmThunkUndefined_thunk

extern AvmBox builtin_a2a_osa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__match_thunk  builtin_a2a_osa_thunk

extern AvmBox builtin_func_s2a_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_avmplus_getQualifiedClassName_thunk  builtin_func_s2a_oa_thunk
#define native_script_function_avmplus_getQualifiedSuperclassName_thunk  builtin_func_s2a_oa_thunk

extern AvmBox builtin_func_b2a_oa_optakAvmThunkUndefined_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_isXMLName_thunk  builtin_func_b2a_oa_optakAvmThunkUndefined_thunk

extern AvmBox builtin_u2a_oao_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__unshift_thunk  builtin_u2a_oao_thunk

extern AvmBox builtin_a2a_oo_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_AS3_setNotification_thunk  builtin_a2a_oo_thunk

extern AvmBox builtin_s2a_n_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Namespace_uri_get_thunk  builtin_s2a_n_thunk

extern AvmBox builtin_s2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_AS3_toXMLString_thunk  builtin_s2a_o_thunk
#define XML_AS3_toString_thunk  builtin_s2a_o_thunk
#define Error_getStackTrace_thunk  builtin_s2a_o_thunk
#define QName_localName_get_thunk  builtin_s2a_o_thunk
#define XMLList_AS3_toString_thunk  builtin_s2a_o_thunk
#define XML_AS3_nodeKind_thunk  builtin_s2a_o_thunk
#define RegExp_source_get_thunk  builtin_s2a_o_thunk
#define XMLList_AS3_toXMLString_thunk  builtin_s2a_o_thunk
#define XMLList_AS3_nodeKind_thunk  builtin_s2a_o_thunk

extern AvmBox builtin_s2a_odi_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Number_private__numberToString_thunk  builtin_s2a_odi_thunk

extern double builtin_d2d_odd_optdkAvmThunkNegInfinity_optdkAvmThunkNegInfinity_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Math_max_thunk  builtin_d2d_odd_optdkAvmThunkNegInfinity_optdkAvmThunkNegInfinity_rest_thunk

extern AvmBox builtin_v2a_ou_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_uint_length_set_thunk  builtin_v2a_ou_thunk
#define __AS3___vec_Vector_object_length_set_thunk  builtin_v2a_ou_thunk
#define __AS3___vec_Vector_int_length_set_thunk  builtin_v2a_ou_thunk
#define __AS3___vec_Vector_double_length_set_thunk  builtin_v2a_ou_thunk
#define Array_length_set_thunk  builtin_v2a_ou_thunk

extern AvmBox builtin_a2a_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__pop_thunk  builtin_a2a_oa_thunk
#define XML_AS3_removeNamespace_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_appendChild_thunk  builtin_a2a_oa_thunk
#define XML_AS3_addNamespace_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_prependChild_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_attribute_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_addNamespace_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_child_thunk  builtin_a2a_oa_thunk
#define XML_AS3_appendChild_thunk  builtin_a2a_oa_thunk
#define Array_private__shift_thunk  builtin_a2a_oa_thunk
#define XML_AS3_prependChild_thunk  builtin_a2a_oa_thunk
#define XML_AS3_child_thunk  builtin_a2a_oa_thunk
#define XML_AS3_attribute_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_removeNamespace_thunk  builtin_a2a_oa_thunk
#define Array_private__reverse_thunk  builtin_a2a_oa_thunk
#define XMLList_AS3_setChildren_thunk  builtin_a2a_oa_thunk
#define XML_AS3_setChildren_thunk  builtin_a2a_oa_thunk

extern AvmBox builtin_i2a_ssd_optsAvmThunkConstant_AvmString_58_____undefined_____opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_indexOf_thunk  builtin_i2a_ssd_optsAvmThunkConstant_AvmString_58_____undefined_____opti0_thunk

extern double builtin_d2d_o_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_private__setUTCDate_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setFullYear_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setMinutes_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCMonth_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCSeconds_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setHours_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setDate_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCMinutes_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setMonth_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCHours_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setSeconds_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCFullYear_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setMilliseconds_thunk  builtin_d2d_o_rest_thunk
#define Date_private__setUTCMilliseconds_thunk  builtin_d2d_o_rest_thunk

extern AvmBox builtin_func_b2a_od_optakAvmThunkUndefined_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_isNaN_thunk  builtin_func_b2a_od_optakAvmThunkUndefined_thunk
#define native_script_function_isFinite_thunk  builtin_func_b2a_od_optakAvmThunkUndefined_thunk

extern AvmBox builtin_i2a_ssd_optsAvmThunkConstant_AvmString_58_____undefined_____opti2147483647_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_lastIndexOf_thunk  builtin_i2a_ssd_optsAvmThunkConstant_AvmString_58_____undefined_____opti2147483647_thunk

extern double builtin_d2d_oaaaaaaa_opti1_opti0_opti0_opti0_opti0_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_UTC_thunk  builtin_d2d_oaaaaaaa_opti1_opti0_opti0_opti0_opti0_rest_thunk

extern AvmBox builtin_i2a_s_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_length_get_thunk  builtin_i2a_s_thunk

extern double builtin_func_d2d_os_optsAvmThunkConstant_AvmString_60_____NaN_____thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_parseFloat_thunk  builtin_func_d2d_os_optsAvmThunkConstant_AvmString_60_____NaN_____thunk

extern AvmBox builtin_b2a_oaa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Object_private__isPrototypeOf_thunk  builtin_b2a_oaa_thunk

extern AvmBox builtin_a2a_oaaa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__sortOn_thunk  builtin_a2a_oaaa_thunk

extern AvmBox builtin_u2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_uint_length_get_thunk  builtin_u2a_o_thunk
#define __AS3___vec_Vector_int_length_get_thunk  builtin_u2a_o_thunk
#define Array_length_get_thunk  builtin_u2a_o_thunk
#define __AS3___vec_Vector_uint_AS3_pop_thunk  builtin_u2a_o_thunk
#define __AS3___vec_Vector_object_length_get_thunk  builtin_u2a_o_thunk
#define __AS3___vec_Vector_double_length_get_thunk  builtin_u2a_o_thunk

extern AvmBox builtin_func_a2a_oau_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_avmplus_describeTypeJSON_thunk  builtin_func_a2a_oau_thunk

extern AvmBox builtin_v2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_double_private__reverse_thunk  builtin_v2a_o_thunk
#define __AS3___vec_Vector_uint_private__reverse_thunk  builtin_v2a_o_thunk
#define __AS3___vec_Vector_object_private__reverse_thunk  builtin_v2a_o_thunk
#define __AS3___vec_Vector_int_private__reverse_thunk  builtin_v2a_o_thunk

extern AvmBox builtin_a2a_oa_optakAvmThunkUndefined_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Function_AS3_call_thunk  builtin_a2a_oa_optakAvmThunkUndefined_rest_thunk

extern AvmBox builtin_s2a_si_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__charAt_thunk  builtin_s2a_si_opti0_thunk

extern AvmBox builtin_s2a_osaa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__replace_thunk  builtin_s2a_osaa_thunk

extern AvmBox builtin_b2a_oas_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Object_private__hasOwnProperty_thunk  builtin_b2a_oas_thunk
#define Object_private__propertyIsEnumerable_thunk  builtin_b2a_oas_thunk

extern AvmBox builtin_i2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_AS3_childIndex_thunk  builtin_i2a_o_thunk
#define XML_prettyIndent_get_thunk  builtin_i2a_o_thunk
#define __AS3___vec_Vector_int_AS3_pop_thunk  builtin_i2a_o_thunk
#define XMLList_AS3_length_thunk  builtin_i2a_o_thunk
#define RegExp_lastIndex_get_thunk  builtin_i2a_o_thunk
#define XMLList_AS3_childIndex_thunk  builtin_i2a_o_thunk
#define Function_length_get_thunk  builtin_i2a_o_thunk

extern AvmBox builtin_a2a_ooa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_object_private__map_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_double_private__filter_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_object_private__filter_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_int_private__map_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_uint_private__map_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_int_private__filter_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_double_private__map_thunk  builtin_a2a_ooa_thunk
#define __AS3___vec_Vector_uint_private__filter_thunk  builtin_a2a_ooa_thunk

extern double builtin_d2d_sd_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_charCodeAt_thunk  builtin_d2d_sd_opti0_thunk

extern AvmBox builtin_s2a_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Object_private__toString_thunk  builtin_s2a_oa_thunk

extern AvmBox builtin_b2a_oaoa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_double_private__every_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_int_private__some_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_object_private__some_thunk  builtin_b2a_oaoa_thunk
#define Array_private__some_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_double_private__some_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_uint_private__some_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_object_private__every_thunk  builtin_b2a_oaoa_thunk
#define Array_private__every_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_int_private__every_thunk  builtin_b2a_oaoa_thunk
#define __AS3___vec_Vector_uint_private__every_thunk  builtin_b2a_oaoa_thunk

extern AvmBox builtin_i2a_ssi_opti2147483647_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__lastIndexOf_thunk  builtin_i2a_ssi_opti2147483647_thunk

extern AvmBox builtin_b2a_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XMLList_AS3_contains_thunk  builtin_b2a_oa_thunk
#define XML_AS3_contains_thunk  builtin_b2a_oa_thunk

extern AvmBox builtin_s2a_sd_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_charAt_thunk  builtin_s2a_sd_opti0_thunk

extern AvmBox builtin_v2a_oaoa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_double_private__forEach_thunk  builtin_v2a_oaoa_thunk
#define __AS3___vec_Vector_object_private__forEach_thunk  builtin_v2a_oaoa_thunk
#define __AS3___vec_Vector_int_private__forEach_thunk  builtin_v2a_oaoa_thunk
#define __AS3___vec_Vector_uint_private__forEach_thunk  builtin_v2a_oaoa_thunk
#define Array_private__forEach_thunk  builtin_v2a_oaoa_thunk

extern AvmBox builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_decodeURI_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk
#define native_script_function_escape_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk
#define native_script_function_unescape_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk
#define native_script_function_encodeURI_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk
#define native_script_function_encodeURIComponent_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk
#define native_script_function_decodeURIComponent_thunk  builtin_func_s2a_os_optsAvmThunkConstant_AvmString_58_____undefined_____thunk

extern AvmBox builtin_s2a_s_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_toLowerCase_thunk  builtin_s2a_s_thunk
#define String_AS3_toUpperCase_thunk  builtin_s2a_s_thunk

extern AvmBox builtin_s2a_sdd_opti0_opti2147483647_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_substr_thunk  builtin_s2a_sdd_opti0_opti2147483647_thunk
#define String_AS3_substring_thunk  builtin_s2a_sdd_opti0_opti2147483647_thunk
#define String_AS3_slice_thunk  builtin_s2a_sdd_opti0_opti2147483647_thunk

extern double builtin_d2d_odd_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Math_private__min_thunk  builtin_d2d_odd_thunk
#define Math_atan2_thunk  builtin_d2d_odd_thunk
#define Math_pow_thunk  builtin_d2d_odd_thunk
#define Math_private__max_thunk  builtin_d2d_odd_thunk

extern AvmBox builtin_a2a_n_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Namespace_prefix_get_thunk  builtin_a2a_n_thunk

extern AvmBox builtin_a2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XMLList_AS3_inScopeNamespaces_thunk  builtin_a2a_o_thunk
#define XML_AS3_notification_thunk  builtin_a2a_o_thunk
#define XML_AS3_inScopeNamespaces_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_copy_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_children_thunk  builtin_a2a_o_thunk
#define XML_AS3_attributes_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_text_thunk  builtin_a2a_o_thunk
#define __AS3___vec_Vector_object_AS3_pop_thunk  builtin_a2a_o_thunk
#define Class_prototype_get_thunk  builtin_a2a_o_thunk
#define XML_AS3_name_thunk  builtin_a2a_o_thunk
#define XML_AS3_namespaceDeclarations_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_name_thunk  builtin_a2a_o_thunk
#define QName_uri_get_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_normalize_thunk  builtin_a2a_o_thunk
#define XML_AS3_text_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_namespaceDeclarations_thunk  builtin_a2a_o_thunk
#define Function_prototype_get_thunk  builtin_a2a_o_thunk
#define XML_AS3_children_thunk  builtin_a2a_o_thunk
#define XML_AS3_parent_thunk  builtin_a2a_o_thunk
#define __AS3___vec_Vector_object_private_type_get_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_comments_thunk  builtin_a2a_o_thunk
#define XML_AS3_localName_thunk  builtin_a2a_o_thunk
#define XML_AS3_copy_thunk  builtin_a2a_o_thunk
#define XML_AS3_normalize_thunk  builtin_a2a_o_thunk
#define Array_AS3_pop_thunk  builtin_a2a_o_thunk
#define XML_AS3_comments_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_attributes_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_localName_thunk  builtin_a2a_o_thunk
#define XMLList_AS3_parent_thunk  builtin_a2a_o_thunk

extern AvmBox builtin_a2a_osau_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__split_thunk  builtin_a2a_osau_thunk

extern AvmBox builtin_v2a_oasb_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Object_protected__setPropertyIsEnumerable_thunk  builtin_v2a_oasb_thunk

extern AvmBox builtin_a2a_os_optsAvmThunkConstant_AvmString_0__________thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define RegExp_AS3_exec_thunk  builtin_a2a_os_optsAvmThunkConstant_AvmString_0__________thunk

extern AvmBox builtin_u2a_o_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_uint_AS3_push_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_object_AS3_push_thunk  builtin_u2a_o_rest_thunk
#define Array_AS3_unshift_thunk  builtin_u2a_o_rest_thunk
#define Array_AS3_push_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_int_AS3_unshift_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_uint_AS3_unshift_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_double_AS3_unshift_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_object_AS3_unshift_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_int_AS3_push_thunk  builtin_u2a_o_rest_thunk
#define __AS3___vec_Vector_double_AS3_push_thunk  builtin_u2a_o_rest_thunk

extern AvmBox builtin_s2a_oi_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_private__toString_thunk  builtin_s2a_oi_thunk
#define Error_getErrorMessage_thunk  builtin_s2a_oi_thunk

extern AvmBox builtin_i2a_ssi_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__indexOf_thunk  builtin_i2a_ssi_opti0_thunk

extern AvmBox builtin_s2a_o_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_AS3_fromCharCode_thunk  builtin_s2a_o_rest_thunk

extern AvmBox builtin_i2a_oaai_opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__lastIndexOf_thunk  builtin_i2a_oaai_opti0_thunk

extern double builtin_d2d_od_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_private__setTime_thunk  builtin_d2d_od_thunk
#define Math_cos_thunk  builtin_d2d_od_thunk
#define Math_ceil_thunk  builtin_d2d_od_thunk
#define Math_acos_thunk  builtin_d2d_od_thunk
#define Math_abs_thunk  builtin_d2d_od_thunk
#define Math_atan_thunk  builtin_d2d_od_thunk
#define Math_asin_thunk  builtin_d2d_od_thunk
#define Math_exp_thunk  builtin_d2d_od_thunk
#define Math_round_thunk  builtin_d2d_od_thunk
#define Math_log_thunk  builtin_d2d_od_thunk
#define Math_tan_thunk  builtin_d2d_od_thunk
#define Math_sin_thunk  builtin_d2d_od_thunk
#define Math_sqrt_thunk  builtin_d2d_od_thunk
#define Math_floor_thunk  builtin_d2d_od_thunk

extern AvmBox builtin_a2a_oaa_optakAvmThunkUndefined_optakAvmThunkUndefined_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Function_AS3_apply_thunk  builtin_a2a_oaa_optakAvmThunkUndefined_optakAvmThunkUndefined_thunk

extern AvmBox builtin_s2a_sii_opti0_opti2147483647_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__slice_thunk  builtin_s2a_sii_opti0_opti2147483647_thunk
#define String_private__substr_thunk  builtin_s2a_sii_opti0_opti2147483647_thunk
#define String_private__substring_thunk  builtin_s2a_sii_opti0_opti2147483647_thunk

extern AvmBox builtin_v2a_ob_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_uint_fixed_set_thunk  builtin_v2a_ob_thunk
#define XML_prettyPrinting_set_thunk  builtin_v2a_ob_thunk
#define __AS3___vec_Vector_object_fixed_set_thunk  builtin_v2a_ob_thunk
#define XML_ignoreComments_set_thunk  builtin_v2a_ob_thunk
#define XML_ignoreWhitespace_set_thunk  builtin_v2a_ob_thunk
#define __AS3___vec_Vector_double_fixed_set_thunk  builtin_v2a_ob_thunk
#define XML_ignoreProcessingInstructions_set_thunk  builtin_v2a_ob_thunk
#define __AS3___vec_Vector_int_fixed_set_thunk  builtin_v2a_ob_thunk

extern AvmBox builtin_v2a_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XMLList_AS3_setName_thunk  builtin_v2a_oa_thunk
#define __AS3___vec_Vector_object_private_type_set_thunk  builtin_v2a_oa_thunk
#define XMLList_AS3_setLocalName_thunk  builtin_v2a_oa_thunk
#define Function_prototype_set_thunk  builtin_v2a_oa_thunk
#define XMLList_AS3_setNamespace_thunk  builtin_v2a_oa_thunk
#define XML_AS3_setName_thunk  builtin_v2a_oa_thunk
#define XML_AS3_setNamespace_thunk  builtin_v2a_oa_thunk
#define XML_AS3_setLocalName_thunk  builtin_v2a_oa_thunk

extern double builtin_d2d_oa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_parse_thunk  builtin_d2d_oa_thunk

extern AvmBox builtin_v2a_oi_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_prettyIndent_set_thunk  builtin_v2a_oi_thunk
#define RegExp_lastIndex_set_thunk  builtin_v2a_oi_thunk

extern double builtin_d2d_oi_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Date_private__get_thunk  builtin_d2d_oi_thunk

extern double builtin_func_d2d_osi_optsAvmThunkConstant_AvmString_60_____NaN_____opti0_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define native_script_function_parseInt_thunk  builtin_func_d2d_osi_optsAvmThunkConstant_AvmString_60_____NaN_____opti0_thunk

extern AvmBox builtin_a2a_oaa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_AS3_insertChildAfter_thunk  builtin_a2a_oaa_thunk
#define XMLList_AS3_insertChildBefore_thunk  builtin_a2a_oaa_thunk
#define XMLList_AS3_insertChildAfter_thunk  builtin_a2a_oaa_thunk
#define XML_AS3_insertChildBefore_thunk  builtin_a2a_oaa_thunk
#define XML_AS3_replace_thunk  builtin_a2a_oaa_thunk
#define XMLList_AS3_replace_thunk  builtin_a2a_oaa_thunk

extern AvmBox builtin_b2a_o_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define RegExp_multiline_get_thunk  builtin_b2a_o_thunk
#define __AS3___vec_Vector_double_fixed_get_thunk  builtin_b2a_o_thunk
#define RegExp_dotall_get_thunk  builtin_b2a_o_thunk
#define XMLList_AS3_hasComplexContent_thunk  builtin_b2a_o_thunk
#define XML_prettyPrinting_get_thunk  builtin_b2a_o_thunk
#define __AS3___vec_Vector_uint_fixed_get_thunk  builtin_b2a_o_thunk
#define XML_ignoreComments_get_thunk  builtin_b2a_o_thunk
#define __AS3___vec_Vector_int_fixed_get_thunk  builtin_b2a_o_thunk
#define RegExp_global_get_thunk  builtin_b2a_o_thunk
#define RegExp_ignoreCase_get_thunk  builtin_b2a_o_thunk
#define __AS3___vec_Vector_object_fixed_get_thunk  builtin_b2a_o_thunk
#define XMLList_AS3_hasSimpleContent_thunk  builtin_b2a_o_thunk
#define XML_AS3_hasSimpleContent_thunk  builtin_b2a_o_thunk
#define XML_ignoreWhitespace_get_thunk  builtin_b2a_o_thunk
#define RegExp_extended_get_thunk  builtin_b2a_o_thunk
#define XML_AS3_hasComplexContent_thunk  builtin_b2a_o_thunk
#define XML_ignoreProcessingInstructions_get_thunk  builtin_b2a_o_thunk

extern AvmBox builtin_i2a_osa_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define String_private__search_thunk  builtin_i2a_osa_thunk

extern AvmBox builtin_a2a_oadd_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__slice_thunk  builtin_a2a_oadd_thunk

extern AvmBox builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XMLList_AS3_elements_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk
#define XML_AS3_descendants_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk
#define XML_AS3_processingInstructions_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk
#define XML_AS3_elements_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk
#define XMLList_AS3_descendants_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk
#define XMLList_AS3_processingInstructions_thunk  builtin_a2a_oa_optsAvmThunkConstant_AvmString_503___________thunk

extern AvmBox builtin_a2a_oai_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define XML_private__namespace_thunk  builtin_a2a_oai_thunk
#define XMLList_private__namespace_thunk  builtin_a2a_oai_thunk

extern AvmBox builtin_i2a_oaai_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Array_private__indexOf_thunk  builtin_i2a_oaai_thunk

extern AvmBox builtin_s2a_odii_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Number_private__convert_thunk  builtin_s2a_odii_thunk

extern double builtin_d2d_odd_optdkAvmThunkInfinity_optdkAvmThunkInfinity_rest_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define Math_min_thunk  builtin_d2d_odd_optdkAvmThunkInfinity_optdkAvmThunkInfinity_rest_thunk

extern AvmBox builtin_a2a_oao_thunk(AvmMethodEnv env, uint32_t argc, AvmBox* argv);
#define __AS3___vec_Vector_uint_private__sort_thunk  builtin_a2a_oao_thunk
#define __AS3___vec_Vector_object_private__sort_thunk  builtin_a2a_oao_thunk
#define Array_private__concat_thunk  builtin_a2a_oao_thunk
#define __AS3___vec_Vector_int_private__sort_thunk  builtin_a2a_oao_thunk
#define __AS3___vec_Vector_double_private__sort_thunk  builtin_a2a_oao_thunk
#define Array_private__splice_thunk  builtin_a2a_oao_thunk
#define Array_private__sort_thunk  builtin_a2a_oao_thunk

#endif // VMCFG_INDIRECT_NATIVE_THUNKS

class SlotOffsetsAndAsserts;
// Object$
//-----------------------------------------------------------
class ObjectClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_ObjectClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_ObjectClass.get_length(); } \
    private: \
        avmplus::NativeID::ObjectClassSlots m_slots_ObjectClass
//-----------------------------------------------------------

// Class$
//-----------------------------------------------------------
class ClassClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_ClassClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_ClassClass.get_length(); } \
    private: \
        avmplus::NativeID::ClassClassSlots m_slots_ClassClass
//-----------------------------------------------------------

// Class
//-----------------------------------------------------------
class ClassClosureSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ClassClosure \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ClassClosureSlots EmptySlotsStruct_ClassClosure
//-----------------------------------------------------------

// Function$
//-----------------------------------------------------------
class FunctionClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_FunctionClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_FunctionClass.get_length(); } \
    private: \
        avmplus::NativeID::FunctionClassSlots m_slots_FunctionClass
//-----------------------------------------------------------

// Function
//-----------------------------------------------------------
class FunctionObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_FunctionObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::FunctionObjectSlots EmptySlotsStruct_FunctionObject
//-----------------------------------------------------------

// Namespace$
//-----------------------------------------------------------
class NamespaceClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE AvmBox get_length() const { return m_length; }
private:
    ATOM_WB m_length;
};
#define DECLARE_SLOTS_NamespaceClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE AvmBox get_length() const { return m_slots_NamespaceClass.get_length(); } \
    private: \
        avmplus::NativeID::NamespaceClassSlots m_slots_NamespaceClass
//-----------------------------------------------------------

// Boolean$
//-----------------------------------------------------------
class BooleanClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_BooleanClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_BooleanClass.get_length(); } \
    private: \
        avmplus::NativeID::BooleanClassSlots m_slots_BooleanClass
//-----------------------------------------------------------

// Number$
//-----------------------------------------------------------
class NumberClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
    REALLY_INLINE int32_t get_private_DTOSTR_FIXED() const { return m_private_DTOSTR_FIXED; }
    REALLY_INLINE int32_t get_private_DTOSTR_PRECISION() const { return m_private_DTOSTR_PRECISION; }
    REALLY_INLINE int32_t get_private_DTOSTR_EXPONENTIAL() const { return m_private_DTOSTR_EXPONENTIAL; }
    REALLY_INLINE double get_NaN() const { return m_NaN; }
    REALLY_INLINE double get_NEGATIVE_INFINITY() const { return m_NEGATIVE_INFINITY; }
    REALLY_INLINE double get_POSITIVE_INFINITY() const { return m_POSITIVE_INFINITY; }
    REALLY_INLINE double get_MIN_VALUE() const { return m_MIN_VALUE; }
    REALLY_INLINE double get_MAX_VALUE() const { return m_MAX_VALUE; }
private:
    int32_t m_length;
    int32_t m_private_DTOSTR_FIXED;
    int32_t m_private_DTOSTR_PRECISION;
    int32_t m_private_DTOSTR_EXPONENTIAL;
    double m_NaN;
    double m_NEGATIVE_INFINITY;
    double m_POSITIVE_INFINITY;
    double m_MIN_VALUE;
    double m_MAX_VALUE;
};
#define DECLARE_SLOTS_NumberClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_NumberClass.get_length(); } \
        REALLY_INLINE int32_t get_private_DTOSTR_FIXED() const { return m_slots_NumberClass.get_private_DTOSTR_FIXED(); } \
        REALLY_INLINE int32_t get_private_DTOSTR_PRECISION() const { return m_slots_NumberClass.get_private_DTOSTR_PRECISION(); } \
        REALLY_INLINE int32_t get_private_DTOSTR_EXPONENTIAL() const { return m_slots_NumberClass.get_private_DTOSTR_EXPONENTIAL(); } \
        REALLY_INLINE double get_NaN() const { return m_slots_NumberClass.get_NaN(); } \
        REALLY_INLINE double get_NEGATIVE_INFINITY() const { return m_slots_NumberClass.get_NEGATIVE_INFINITY(); } \
        REALLY_INLINE double get_POSITIVE_INFINITY() const { return m_slots_NumberClass.get_POSITIVE_INFINITY(); } \
        REALLY_INLINE double get_MIN_VALUE() const { return m_slots_NumberClass.get_MIN_VALUE(); } \
        REALLY_INLINE double get_MAX_VALUE() const { return m_slots_NumberClass.get_MAX_VALUE(); } \
    private: \
        avmplus::NativeID::NumberClassSlots m_slots_NumberClass
//-----------------------------------------------------------

// int$
//-----------------------------------------------------------
class IntClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_MIN_VALUE() const { return m_MIN_VALUE; }
    REALLY_INLINE int32_t get_MAX_VALUE() const { return m_MAX_VALUE; }
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_MIN_VALUE;
    int32_t m_MAX_VALUE;
    int32_t m_length;
};
#define DECLARE_SLOTS_IntClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_MIN_VALUE() const { return m_slots_IntClass.get_MIN_VALUE(); } \
        REALLY_INLINE int32_t get_MAX_VALUE() const { return m_slots_IntClass.get_MAX_VALUE(); } \
        REALLY_INLINE int32_t get_length() const { return m_slots_IntClass.get_length(); } \
    private: \
        avmplus::NativeID::IntClassSlots m_slots_IntClass
//-----------------------------------------------------------

// uint$
//-----------------------------------------------------------
class UIntClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE uint32_t get_MIN_VALUE() const { return m_MIN_VALUE; }
    REALLY_INLINE uint32_t get_MAX_VALUE() const { return m_MAX_VALUE; }
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    uint32_t m_MIN_VALUE;
    uint32_t m_MAX_VALUE;
    int32_t m_length;
};
#define DECLARE_SLOTS_UIntClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE uint32_t get_MIN_VALUE() const { return m_slots_UIntClass.get_MIN_VALUE(); } \
        REALLY_INLINE uint32_t get_MAX_VALUE() const { return m_slots_UIntClass.get_MAX_VALUE(); } \
        REALLY_INLINE int32_t get_length() const { return m_slots_UIntClass.get_length(); } \
    private: \
        avmplus::NativeID::UIntClassSlots m_slots_UIntClass
//-----------------------------------------------------------

// String$
//-----------------------------------------------------------
class StringClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_StringClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_StringClass.get_length(); } \
    private: \
        avmplus::NativeID::StringClassSlots m_slots_StringClass
//-----------------------------------------------------------

// Array$
//-----------------------------------------------------------
class ArrayClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE uint32_t get_CASEINSENSITIVE() const { return m_CASEINSENSITIVE; }
    REALLY_INLINE uint32_t get_DESCENDING() const { return m_DESCENDING; }
    REALLY_INLINE uint32_t get_UNIQUESORT() const { return m_UNIQUESORT; }
    REALLY_INLINE uint32_t get_RETURNINDEXEDARRAY() const { return m_RETURNINDEXEDARRAY; }
    REALLY_INLINE uint32_t get_NUMERIC() const { return m_NUMERIC; }
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    uint32_t m_CASEINSENSITIVE;
    uint32_t m_DESCENDING;
    uint32_t m_UNIQUESORT;
    uint32_t m_RETURNINDEXEDARRAY;
    uint32_t m_NUMERIC;
    int32_t m_length;
};
#define DECLARE_SLOTS_ArrayClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE uint32_t get_CASEINSENSITIVE() const { return m_slots_ArrayClass.get_CASEINSENSITIVE(); } \
        REALLY_INLINE uint32_t get_DESCENDING() const { return m_slots_ArrayClass.get_DESCENDING(); } \
        REALLY_INLINE uint32_t get_UNIQUESORT() const { return m_slots_ArrayClass.get_UNIQUESORT(); } \
        REALLY_INLINE uint32_t get_RETURNINDEXEDARRAY() const { return m_slots_ArrayClass.get_RETURNINDEXEDARRAY(); } \
        REALLY_INLINE uint32_t get_NUMERIC() const { return m_slots_ArrayClass.get_NUMERIC(); } \
        REALLY_INLINE int32_t get_length() const { return m_slots_ArrayClass.get_length(); } \
    private: \
        avmplus::NativeID::ArrayClassSlots m_slots_ArrayClass
//-----------------------------------------------------------

// Array
//-----------------------------------------------------------
class ArrayObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ArrayObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ArrayObjectSlots EmptySlotsStruct_ArrayObject
//-----------------------------------------------------------

// __AS3__.vec::Vector$
//-----------------------------------------------------------
class VectorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_VectorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::VectorClassSlots EmptySlotsStruct_VectorClass
//-----------------------------------------------------------

// __AS3__.vec::Vector
//-----------------------------------------------------------
class ObjectVectorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ObjectVectorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ObjectVectorObjectSlots EmptySlotsStruct_ObjectVectorObject
//-----------------------------------------------------------

// __AS3__.vec::Vector$object$
//-----------------------------------------------------------
class ObjectVectorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ObjectVectorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ObjectVectorClassSlots EmptySlotsStruct_ObjectVectorClass
//-----------------------------------------------------------

// __AS3__.vec::Vector$int$
//-----------------------------------------------------------
class IntVectorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_IntVectorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::IntVectorClassSlots EmptySlotsStruct_IntVectorClass
//-----------------------------------------------------------

// __AS3__.vec::Vector$int
//-----------------------------------------------------------
class IntVectorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_IntVectorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::IntVectorObjectSlots EmptySlotsStruct_IntVectorObject
//-----------------------------------------------------------

// __AS3__.vec::Vector$uint$
//-----------------------------------------------------------
class UIntVectorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_UIntVectorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::UIntVectorClassSlots EmptySlotsStruct_UIntVectorClass
//-----------------------------------------------------------

// __AS3__.vec::Vector$uint
//-----------------------------------------------------------
class UIntVectorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_UIntVectorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::UIntVectorObjectSlots EmptySlotsStruct_UIntVectorObject
//-----------------------------------------------------------

// __AS3__.vec::Vector$double$
//-----------------------------------------------------------
class DoubleVectorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_DoubleVectorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::DoubleVectorClassSlots EmptySlotsStruct_DoubleVectorClass
//-----------------------------------------------------------

// __AS3__.vec::Vector$double
//-----------------------------------------------------------
class DoubleVectorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_DoubleVectorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::DoubleVectorObjectSlots EmptySlotsStruct_DoubleVectorObject
//-----------------------------------------------------------

// private::MethodClosure$
//-----------------------------------------------------------
class MethodClosureClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_MethodClosureClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::MethodClosureClassSlots EmptySlotsStruct_MethodClosureClass
//-----------------------------------------------------------

// private::MethodClosure
//-----------------------------------------------------------
class MethodClosureSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_MethodClosure \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::MethodClosureSlots EmptySlotsStruct_MethodClosure
//-----------------------------------------------------------

// Math$
//-----------------------------------------------------------
class MathClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE double get_E() const { return m_E; }
    REALLY_INLINE double get_LN10() const { return m_LN10; }
    REALLY_INLINE double get_LN2() const { return m_LN2; }
    REALLY_INLINE double get_LOG10E() const { return m_LOG10E; }
    REALLY_INLINE double get_LOG2E() const { return m_LOG2E; }
    REALLY_INLINE double get_PI() const { return m_PI; }
    REALLY_INLINE double get_SQRT1_2() const { return m_SQRT1_2; }
    REALLY_INLINE double get_SQRT2() const { return m_SQRT2; }
    REALLY_INLINE double get_private_NegInfinity() const { return m_private_NegInfinity; }
private:
    double m_E;
    double m_LN10;
    double m_LN2;
    double m_LOG10E;
    double m_LOG2E;
    double m_PI;
    double m_SQRT1_2;
    double m_SQRT2;
    double m_private_NegInfinity;
};
#define DECLARE_SLOTS_MathClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE double get_E() const { return m_slots_MathClass.get_E(); } \
        REALLY_INLINE double get_LN10() const { return m_slots_MathClass.get_LN10(); } \
        REALLY_INLINE double get_LN2() const { return m_slots_MathClass.get_LN2(); } \
        REALLY_INLINE double get_LOG10E() const { return m_slots_MathClass.get_LOG10E(); } \
        REALLY_INLINE double get_LOG2E() const { return m_slots_MathClass.get_LOG2E(); } \
        REALLY_INLINE double get_PI() const { return m_slots_MathClass.get_PI(); } \
        REALLY_INLINE double get_SQRT1_2() const { return m_slots_MathClass.get_SQRT1_2(); } \
        REALLY_INLINE double get_SQRT2() const { return m_slots_MathClass.get_SQRT2(); } \
        REALLY_INLINE double get_private_NegInfinity() const { return m_slots_MathClass.get_private_NegInfinity(); } \
    private: \
        avmplus::NativeID::MathClassSlots m_slots_MathClass
//-----------------------------------------------------------

// Error$
//-----------------------------------------------------------
class ErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_ErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_ErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::ErrorClassSlots m_slots_ErrorClass
//-----------------------------------------------------------

// Error
//-----------------------------------------------------------
class ErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_private__errorID() const { return m_private__errorID; }
    void set_private__errorID(int32_t newVal);
    REALLY_INLINE AvmBox get_message() const { return m_message; }
    void set_message(ErrorObject* obj, AvmBox newVal);
    REALLY_INLINE AvmBox get_name() const { return m_name; }
    void set_name(ErrorObject* obj, AvmBox newVal);
private:
    int32_t m_private__errorID;
    ATOM_WB m_message;
    ATOM_WB m_name;
};
REALLY_INLINE void ErrorObjectSlots::set_private__errorID(int32_t newVal) { m_private__errorID = newVal; }
REALLY_INLINE void ErrorObjectSlots::set_message(ErrorObject* obj, AvmBox newVal)
{
    m_message.set(((ScriptObject*)obj)->gc(), obj, newVal);
}
REALLY_INLINE void ErrorObjectSlots::set_name(ErrorObject* obj, AvmBox newVal)
{
    m_name.set(((ScriptObject*)obj)->gc(), obj, newVal);
}
#define DECLARE_SLOTS_ErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_private__errorID() const { return m_slots_ErrorObject.get_private__errorID(); } \
        REALLY_INLINE void set_private__errorID(int32_t newVal) { m_slots_ErrorObject.set_private__errorID(newVal); } \
        REALLY_INLINE AvmBox get_message() const { return m_slots_ErrorObject.get_message(); } \
        REALLY_INLINE void set_message(AvmBox newVal) { m_slots_ErrorObject.set_message(this, newVal); } \
        REALLY_INLINE AvmBox get_name() const { return m_slots_ErrorObject.get_name(); } \
        REALLY_INLINE void set_name(AvmBox newVal) { m_slots_ErrorObject.set_name(this, newVal); } \
    private: \
        avmplus::NativeID::ErrorObjectSlots m_slots_ErrorObject
//-----------------------------------------------------------

// DefinitionError$
//-----------------------------------------------------------
class DefinitionErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_DefinitionErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_DefinitionErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::DefinitionErrorClassSlots m_slots_DefinitionErrorClass
//-----------------------------------------------------------

// DefinitionError
//-----------------------------------------------------------
class DefinitionErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_DefinitionErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::DefinitionErrorObjectSlots EmptySlotsStruct_DefinitionErrorObject
//-----------------------------------------------------------

// EvalError$
//-----------------------------------------------------------
class EvalErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_EvalErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_EvalErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::EvalErrorClassSlots m_slots_EvalErrorClass
//-----------------------------------------------------------

// EvalError
//-----------------------------------------------------------
class EvalErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_EvalErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::EvalErrorObjectSlots EmptySlotsStruct_EvalErrorObject
//-----------------------------------------------------------

// RangeError$
//-----------------------------------------------------------
class RangeErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_RangeErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_RangeErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::RangeErrorClassSlots m_slots_RangeErrorClass
//-----------------------------------------------------------

// RangeError
//-----------------------------------------------------------
class RangeErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_RangeErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::RangeErrorObjectSlots EmptySlotsStruct_RangeErrorObject
//-----------------------------------------------------------

// ReferenceError$
//-----------------------------------------------------------
class ReferenceErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_ReferenceErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_ReferenceErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::ReferenceErrorClassSlots m_slots_ReferenceErrorClass
//-----------------------------------------------------------

// ReferenceError
//-----------------------------------------------------------
class ReferenceErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ReferenceErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ReferenceErrorObjectSlots EmptySlotsStruct_ReferenceErrorObject
//-----------------------------------------------------------

// SecurityError$
//-----------------------------------------------------------
class SecurityErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_SecurityErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_SecurityErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::SecurityErrorClassSlots m_slots_SecurityErrorClass
//-----------------------------------------------------------

// SecurityError
//-----------------------------------------------------------
class SecurityErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_SecurityErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::SecurityErrorObjectSlots EmptySlotsStruct_SecurityErrorObject
//-----------------------------------------------------------

// SyntaxError$
//-----------------------------------------------------------
class SyntaxErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_SyntaxErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_SyntaxErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::SyntaxErrorClassSlots m_slots_SyntaxErrorClass
//-----------------------------------------------------------

// SyntaxError
//-----------------------------------------------------------
class SyntaxErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_SyntaxErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::SyntaxErrorObjectSlots EmptySlotsStruct_SyntaxErrorObject
//-----------------------------------------------------------

// TypeError$
//-----------------------------------------------------------
class TypeErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_TypeErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_TypeErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::TypeErrorClassSlots m_slots_TypeErrorClass
//-----------------------------------------------------------

// TypeError
//-----------------------------------------------------------
class TypeErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_TypeErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::TypeErrorObjectSlots EmptySlotsStruct_TypeErrorObject
//-----------------------------------------------------------

// URIError$
//-----------------------------------------------------------
class URIErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_URIErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_URIErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::URIErrorClassSlots m_slots_URIErrorClass
//-----------------------------------------------------------

// URIError
//-----------------------------------------------------------
class URIErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_URIErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::URIErrorObjectSlots EmptySlotsStruct_URIErrorObject
//-----------------------------------------------------------

// VerifyError$
//-----------------------------------------------------------
class VerifyErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_VerifyErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_VerifyErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::VerifyErrorClassSlots m_slots_VerifyErrorClass
//-----------------------------------------------------------

// VerifyError
//-----------------------------------------------------------
class VerifyErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_VerifyErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::VerifyErrorObjectSlots EmptySlotsStruct_VerifyErrorObject
//-----------------------------------------------------------

// UninitializedError$
//-----------------------------------------------------------
class UninitializedErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_UninitializedErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_UninitializedErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::UninitializedErrorClassSlots m_slots_UninitializedErrorClass
//-----------------------------------------------------------

// UninitializedError
//-----------------------------------------------------------
class UninitializedErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_UninitializedErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::UninitializedErrorObjectSlots EmptySlotsStruct_UninitializedErrorObject
//-----------------------------------------------------------

// ArgumentError$
//-----------------------------------------------------------
class ArgumentErrorClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_ArgumentErrorClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_ArgumentErrorClass.get_length(); } \
    private: \
        avmplus::NativeID::ArgumentErrorClassSlots m_slots_ArgumentErrorClass
//-----------------------------------------------------------

// ArgumentError
//-----------------------------------------------------------
class ArgumentErrorObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_ArgumentErrorObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::ArgumentErrorObjectSlots EmptySlotsStruct_ArgumentErrorObject
//-----------------------------------------------------------

// Date$
//-----------------------------------------------------------
class DateClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_DateClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_DateClass.get_length(); } \
    private: \
        avmplus::NativeID::DateClassSlots m_slots_DateClass
//-----------------------------------------------------------

// Date
//-----------------------------------------------------------
class DateObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_DateObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::DateObjectSlots EmptySlotsStruct_DateObject
//-----------------------------------------------------------

// RegExp$
//-----------------------------------------------------------
class RegExpClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE int32_t get_length() const { return m_length; }
private:
    int32_t m_length;
};
#define DECLARE_SLOTS_RegExpClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE int32_t get_length() const { return m_slots_RegExpClass.get_length(); } \
    private: \
        avmplus::NativeID::RegExpClassSlots m_slots_RegExpClass
//-----------------------------------------------------------

// RegExp
//-----------------------------------------------------------
class RegExpObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_RegExpObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::RegExpObjectSlots EmptySlotsStruct_RegExpObject
//-----------------------------------------------------------

// XML$
//-----------------------------------------------------------
class XMLClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE AvmBox get_length() const { return m_length; }
private:
    ATOM_WB m_length;
};
#define DECLARE_SLOTS_XMLClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE AvmBox get_length() const { return m_slots_XMLClass.get_length(); } \
    private: \
        avmplus::NativeID::XMLClassSlots m_slots_XMLClass
//-----------------------------------------------------------

// XML
//-----------------------------------------------------------
class XMLObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_XMLObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::XMLObjectSlots EmptySlotsStruct_XMLObject
//-----------------------------------------------------------

// XMLList$
//-----------------------------------------------------------
class XMLListClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE AvmBox get_length() const { return m_length; }
private:
    ATOM_WB m_length;
};
#define DECLARE_SLOTS_XMLListClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE AvmBox get_length() const { return m_slots_XMLListClass.get_length(); } \
    private: \
        avmplus::NativeID::XMLListClassSlots m_slots_XMLListClass
//-----------------------------------------------------------

// XMLList
//-----------------------------------------------------------
class XMLListObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_XMLListObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::XMLListObjectSlots EmptySlotsStruct_XMLListObject
//-----------------------------------------------------------

// QName$
//-----------------------------------------------------------
class QNameClassSlots
{
    friend class SlotOffsetsAndAsserts;
public:
    REALLY_INLINE AvmBox get_length() const { return m_length; }
private:
    ATOM_WB m_length;
};
#define DECLARE_SLOTS_QNameClass \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
    protected: \
        REALLY_INLINE AvmBox get_length() const { return m_slots_QNameClass.get_length(); } \
    private: \
        avmplus::NativeID::QNameClassSlots m_slots_QNameClass
//-----------------------------------------------------------

// QName
//-----------------------------------------------------------
class QNameObjectSlots
{
    friend class SlotOffsetsAndAsserts;
public:
private:
};
#define DECLARE_SLOTS_QNameObject \
    private: \
        friend class avmplus::NativeID::SlotOffsetsAndAsserts; \
        typedef avmplus::NativeID::QNameObjectSlots EmptySlotsStruct_QNameObject
//-----------------------------------------------------------

} }
