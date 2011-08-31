/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is JavaScript Engine testing utilities.
 *
 * The Initial Developer of the Original Code is
 * Jeff Walden <jwalden+code@mit.edu>.
 * Portions created by the Initial Developer are Copyright (C) 2006
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
 * ***** END LICENSE BLOCK ***** */

startTest();

var gTestfile = 'regress-364104.js';
//-----------------------------------------------------------------------------
var BUGNUMBER     = "364104";
var summary = "Array.prototype.indexOf, Array.prototype.lastIndexOf issues " +
  "with the optional second fromIndex argument";
var actual, expect;

printBugNumber(BUGNUMBER);
printStatus(summary);

/**************
 * BEGIN TEST *
 **************/

var failed = false;

AddTestCase('[2].indexOf(2) == 0', true, [2].indexOf(2) == 0);          // indexOf: not finding 2!?
AddTestCase('[2].indexOf(2, 0) == 0', true, [2].indexOf(2, 0) == 0);    // indexOf: not interpreting explicit second argument 0!
AddTestCase('[2].indexOf(2, 1) == -1', true, [2].indexOf(2, 1) == -1);  // indexOf: ignoring second argument with value equal to array length!
AddTestCase('[2].indexOf(2, 2) == -1', true, [2].indexOf(2, 2) == -1);  // indexOf: ignoring second argument greater than array length!
AddTestCase('[2].indexOf(2, 17) == -1', true, [2].indexOf(2, 17) == -1);// indexOf: ignoring large second argument!
AddTestCase('[2].indexOf(2, -5) == 0', true, [2].indexOf(2, -5) == 0);  // indexOf: calculated fromIndex < 0, should search entire array!         
AddTestCase('[2, 3].indexOf(2, -1) == -1', true, [2, 3].indexOf(2, -1) == -1);  // indexOf: not handling index == (-1 + 2), element 2 correctly!
AddTestCase('[2, 3].indexOf(3, -1) == 1', true, [2, 3].indexOf(3, -1) == 1);    // indexOf: not handling index == (-1 + 2), element 3 correctly!
AddTestCase('[2].lastIndexOf(2) == 0', true, [2].lastIndexOf(2) == 0);          // lastIndexOf: not finding 2!?
AddTestCase('[2].lastIndexOf(2, 1) == 0', true, [2].lastIndexOf(2, 1) == 0);    // lastIndexOf: not interpreting explicit second argument 1!?
AddTestCase('[2].lastIndexOf(2, 17) == 0', true, [2].lastIndexOf(2, 17) == 0);   // lastIndexOf: should have searched entire array!
AddTestCase('[2].lastIndexOf(2, -5) == -1', true, [2].lastIndexOf(2, -5) == -1); // lastIndexOf: -5 + 1 < 0, so array shouldn't be searched!
AddTestCase('[2].lastIndexOf(2, -2) == -1', true, [2].lastIndexOf(2, -2) == -1); // lastIndexOf: -2 + 1 < 0, so array shouldn't be searched!
AddTestCase('[2, 3].lastIndexOf(2, -1) == 0', true, [2, 3].lastIndexOf(2, -1) == 0); // lastIndexOf: not handling index == (-1 + 2), element 2 correctly!
AddTestCase('[2, 3].lastIndexOf(3, -1) == 1', true, [2, 3].lastIndexOf(3, -1) == 1); // lastIndexOf: not handling index == (-1 + 2), element 3 correctly!
AddTestCase('[2, 3].lastIndexOf(2, -2) == 0', true, [2, 3].lastIndexOf(2, -2) == 0); // lastIndexOf: not handling index == (-2 + 2), element 2 correctly!
AddTestCase('[2, 3].lastIndexOf(3, -2) == -1', true, [2, 3].lastIndexOf(3, -2) == -1); // lastIndexOf: not handling index == (-2 + 2), element 3 correctly!
AddTestCase('[2, 3].lastIndexOf(2, -3) == -1', true, [2, 3].lastIndexOf(2, -3) == -1); // lastIndexOf: calculated fromIndex < 0, shouldn't search array for 2!
AddTestCase('[2, 3].lastIndexOf(3, -3) == -1', true, [2, 3].lastIndexOf(3, -3) == -1); // lastIndexOf: calculated fromIndex < 0, shouldn't search array for 3!

test();
