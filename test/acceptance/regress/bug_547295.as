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
* See http://bugzilla.mozilla.org/show_bug.cgi?id=547295
*
*/
//-----------------------------------------------------------------------------

var SECTION = "547295";
var VERSION = "";
var TITLE   = "XMLParser::unescape does not handle lowercase hex values";
var bug = "547295";

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

    status = 'lowercase';
    var lowercase:XML = new XML ("<foo>bar &#x6a;</foo>");
    expect = "bar j";
    actual = lowercase;
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    
    status = 'UPPERCASE';
    var uppercase:XML = new XML ("<foo>bar &#x6A;</foo>");
    expect = "bar j";
    actual = uppercase;
    array[item++] = new TestCase(SECTION, status, expect, actual);



    return array;
}
