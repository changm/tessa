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
* See http://bugzilla.mozilla.org/show_bug.cgi?id=526295
*
*/
//-----------------------------------------------------------------------------

package {
    public class BadCode {
        
        public var state:Object;
        
        public function BadCode() {
            state = new Object();        
            state.x = 1;
        }

        // Calling this code will throw the Error object thrown in the throwAnError() method
        public final function goodCode():void {
            try {              
                throwAnError();                        
                state.x = state.x - 1;
            }
            finally {
            }
        }
        
        // Call this code will not throw the Error, instead a "undefined" exception is thrown from this
        // method directly.
        public final function badCode():void {          
            try {              
                throwAnError();                        
                state.x--;
            }
            finally {
            }
        }

        public final function throwAnError():void {
            throw new Error();
        }
    }
    
    startTest();
    err = "no error";
    var foo:BadCode = new BadCode();
    try {
        foo.goodCode();
    } catch (e) {
        err = grabError(e, e.toString());
        AddTestCase("goodCode", "Error", err );
    }
    
    
    err = "no error";
    try {
        foo.badCode();
    } catch (e) {
        err = grabError(e, e.toString());
        AddTestCase("badCode", "Error", err );
    }
    test();

}


