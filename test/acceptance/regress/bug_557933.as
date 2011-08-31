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
* See http://bugzilla.mozilla.org/show_bug.cgi?id=557933
*
*/
//-----------------------------------------------------------------------------

var SECTION = "557933";
var VERSION = "";
var TITLE   = "parseIndex/getIntItem reject leading zeroes";
var bug = "557933";

startTest();
writeHeaderToLog(SECTION + " " + TITLE);
var testcases = getTestCases();
test();

function concat_strs(a,b)
{
    return a + b;
}

function getTestCases() {
    var array:Array = new Array();
    var item:int = 0;
    
    var tests = [
        { key: 9,                       expect: 1 },
        { key: "9",                     expect: 1 },
        { key: "09",                    expect: 31 },
        { key: "009",                   expect: 3 },
        { key: "0009",                  expect: 4 },
        { key: 8,                       expect: 8 },
        { key: 0x8,                     expect: 8 },
        { key: 0x08,                    expect: 8 },
        { key: "8",                     expect: 8 },
        { key: "0x8",                   expect: 9 },
        { key: "0x08",                  expect: 10 },
        { key: -7,                      expect: 14 },
        { key: -07,                     expect: 14 },
        { key: -007,                    expect: 14 },
        { key: "-7",                    expect: 14 },
        { key: "-07",                   expect: 15 },
        { key: "-007",                  expect: 16 },
        { key: 6,                       expect: 24 },
        { key: 6.0,                     expect: 24 },
        { key: 6.00,                    expect: 24 },
        { key: 06.0,                    expect: 24 },
        { key: 06.00,                   expect: 24 },
        { key: 006.0,                   expect: 24 },
        { key: 006.00,                  expect: 24 },
        { key: "6",                     expect: 24 },
        { key: "6.0",                   expect: 25 },
        { key: "6.00",                  expect: 26 },
        { key: "06.0",                  expect: 27 },
        { key: "06.00",                 expect: 28 },
        { key: "006.0",                 expect: 29 },
        { key: "006.00",                expect: 30 },
        { key: concat_strs("0","9"),    expect: 31 },
    ]

    var o:* = []; // must be type *, not Array

    for (var i = 0; i < tests.length; ++i)
    {
        o[tests[i].key] = i;
    }
        
    for (var i = 0; i < tests.length; ++i)
    {
        array[item++] = new TestCase(SECTION, String(item) + ": " + typeof(tests[i].key) + " " + String(tests[i].key), tests[i].expect, o[tests[i].key]);
    }

    return array;
}
