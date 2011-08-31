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
 * Portions created by the Initial Developer are Copyright (C) 2007-2008
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


/*
    This file is the original source for bug_555705.abc_ 

    bug_555705.abc_ was created by changing offset 18c from 00 to 01 in the
    generated abc for this file (asc.jar version 14710).
    
*/



package {
    function f() { 
        print("Hello, world!");
    }
    function g() {
        print("Bug 555705 - should not leak memory: PASSED!");
    }
    class C {
        function hello() {
            try {
                f();
            }
            catch (e1 : RangeError) {
                print("Range");
            }
            catch (e2) {
                if (e2.toString().substr(0,24) == "VerifyError: Error #1025") {
                    print("Modified abc throws 'VerifyError: Error #1025: An invalid register 1 was accessed.' PASSED!");
                } else {
                    print("Unexpected Error Thrown FAILED! : "+e2.toString());
                }
            }
            finally {
                g();
            }
        }
    }
    (new C).hello();
}
