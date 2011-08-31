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
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
 * ***** END LICENSE BLOCK *****
*
*
* See http://bugzilla.mozilla.org/show_bug.cgi?id=550946
*
*/
//-----------------------------------------------------------------------------

var SECTION = "eregress_550946";
var VERSION = "";
var TITLE   = "RegExp conformance test";
var bug = "550946";

startTest();
writeHeaderToLog(SECTION + " " + TITLE);
var testcases = getTestCases();
test();

function getTestCases() {
    var array:Array = new Array();
    var item:int = 0;
    var status:String = '';
    var actual:String = '';
    var expect:String= '';

    status = inSection(1);
    var str:String = "\u5F0A\u793E\u304C\u5236\u4F5C\u5354\u529B\u3057\u307Ehttp://link1.com/\u3000\u5C11\u5E74\u30BF\u30B1\u30B7\u3082\u30EA\u30CBhttp://link2.com/";
    var myPattern:RegExp = /(http:\/\/[\x21-\x7e]+)/gi;
    var match_array:Array = str.match(myPattern);
    expect = "http://link1.com/,http://link2.com/";
    actual = match_array.toString();
    array[item++] = new TestCase(SECTION, status, expect, actual);


    return array;
}

