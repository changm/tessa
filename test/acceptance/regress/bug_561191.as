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

var SECTION = "561191";
var VERSION = "";
var TITLE   = "unescape bugginess";
var bug = "561191";

startTest();
writeHeaderToLog(SECTION + " " + TITLE);
var testcases = getTestCases();
test();

function getTestCases() {
    var array:Array = new Array();
    var item:int = 0;
    
    var tests = [
        { xml: "<a foo='bar'/>",        expect: '<a foo="bar"/>' },
        { xml: "<a foo='&bar'/>",        expect: '<a foo="&amp;bar"/>' },
        { xml: "<a foo='bar&'/>",        expect: '<a foo="bar&amp;"/>' },
        { xml: "<a foo='bar&;'/>",        expect: '<a foo="bar&amp;;"/>' },
        { xml: "<a foo='b&a;r'/>",        expect: '<a foo="b&amp;a;r"/>' },
        { xml: "<a foo='&b;ar&;'/>",        expect: '<a foo="&amp;b;ar&amp;;"/>' },
        { xml: "<a foo='&b;ar&;'/>",        expect: '<a foo="&amp;b;ar&amp;;"/>' },
        { xml: "<a foo='&123b;ar&;'/>",        expect: '<a foo="&amp;123b;ar&amp;;"/>' },
        { xml: "<a foo='&bxxx;ar&;'/>",        expect: '<a foo="&amp;bxxx;ar&amp;;"/>' },
        { xml: "<a foo='&bx;ar&;'/>",        expect: '<a foo="&amp;bx;ar&amp;;"/>' },
        { xml: "<a foo='&by;ar&;'/>",        expect: '<a foo="&amp;by;ar&amp;;"/>' },
        { xml: "<a foo='&#33;'/>",        expect: '<a foo="!"/>' },
        { xml: "<a foo='&#3;'/>",        expect: '<a foo="' + String.fromCharCode(3) + '"/>' },
        { xml: "<a foo='&#x3;'/>",        expect: '<a foo="' + String.fromCharCode(3) + '"/>' },
        { xml: "<a foo='&#x33;'/>",        expect: '<a foo="3"/>' },
        { xml: "<a foo='&#33;ar&;'/>",        expect: '<a foo="!ar&amp;;"/>' },
        { xml: "<a foo='&#3;ar&;'/>",        expect: '<a foo="' + String.fromCharCode(3) + 'ar&amp;;"/>' },
        { xml: "<a foo='&#x3;ar&;'/>",        expect: '<a foo="' + String.fromCharCode(3) + 'ar&amp;;"/>' },
        { xml: "<a foo='&#x33;ar&;'/>",        expect: '<a foo="3ar&amp;;"/>' },
    ]

    for (var i = 0; i < tests.length; ++i)
    {
        var x:XML = new XML(tests[i].xml);
        array[item++] = new TestCase(SECTION, tests[i].xml, tests[i].expect, x.toXMLString());
    }

    return array;
}
