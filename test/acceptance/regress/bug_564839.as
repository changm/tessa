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
* See http://bugzilla.mozilla.org/show_bug.cgi?id=557275
*
*/
//-----------------------------------------------------------------------------

var SECTION = "564839";
var VERSION = "";
var TITLE   = "Strings that end in charCode(0) convert to Number differently from 10.0 ";
var bug = "564839";

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

    var str = "";
    
    str = "4"; str += String.fromCharCode(52); 
    status = "new Number " + escape(str);
    expect = '44';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4"; str += String.fromCharCode(0); 
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4"; str += String.fromCharCode(0); str += "4";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "4.2"; str += String.fromCharCode(0);
    status = "new Number " + escape(str);
    expect = '4.2';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4"; str += String.fromCharCode(0); str += ".2";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4."; str += String.fromCharCode(0); str += "2";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4.2"; str += String.fromCharCode(0);
    status = "new Number " + escape(str);
    expect = '4.2';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4e5"; str += String.fromCharCode(0);
    status = "new Number " + escape(str);
    expect = '400000';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4e"; str += String.fromCharCode(0); str += "5";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4"; str += String.fromCharCode(0); str += "e5";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4.2e5"; str += String.fromCharCode(0);
    status = "new Number " + escape(str);
    expect = '420000';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4.2e"; str += String.fromCharCode(0); str += "5";
    status = "new Number " + escape(str);
    expect = '4.2';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4.2"; str += String.fromCharCode(0); str += "e5";
    status = "new Number " + escape(str);
    expect = '4.2';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4."; str += String.fromCharCode(0); str += "2e5";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "4"; str += String.fromCharCode(0); str += ".2e5";
    status = "new Number " + escape(str);
    expect = '4';
    actual = new Number(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);
    
    str = "44"; str += String.fromCharCode(0); 
    status = "parseInt " + escape(str);
    expect = '44';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "4"; str += String.fromCharCode(0); str += "4";
    status = "parseInt " + escape(str);
    expect = '4';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "0x44"; str += String.fromCharCode(0); 
    status = "parseInt " + escape(str);
    expect = '68';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "0x4"; str += String.fromCharCode(0); str += "4";
    status = "parseInt " + escape(str);
    expect = '4';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "0x"; str += String.fromCharCode(0); str += "44";
    status = "parseInt " + escape(str);
    expect = 'NaN';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    str = "0"; str += String.fromCharCode(0); str += "x44";
    status = "parseInt " + escape(str);
    expect = '0';
    actual = parseInt(str);
    array[item++] = new TestCase(SECTION, status, expect, actual);

    return array;
}
