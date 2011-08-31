#!/usr/bin/env python
# -*- python -*-
# ex: set syntax=python:
#  ***** BEGIN LICENSE BLOCK *****
#  Version: MPL 1.1/GPL 2.0/LGPL 2.1
# 
#  The contents of this file are subject to the Mozilla Public License Version
#  1.1 (the "License"); you may not use this file except in compliance with
#  the License. You may obtain a copy of the License at
#  http://www.mozilla.org/MPL/
# 
#  Software distributed under the License is distributed on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#  for the specific language governing rights and limitations under the
#  License.
# 
#  The Original Code is [Open Source Virtual Machine.].
# 
#  The Initial Developer of the Original Code is
#  Adobe System Incorporated.
#  Portions created by the Initial Developer are Copyright (C) 2010
#  the Initial Developer. All Rights Reserved.
# 
#  Contributor(s):
#    Adobe AS3 Team
# 
#  Alternatively, the contents of this file may be used under the terms of
#  either the GNU General Public License Version 2 or later (the "GPL"), or
#  the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
#  in which case the provisions of the GPL or the LGPL are applicable instead
#  of those above. If you wish to allow use of your version of this file only
#  under the terms of either the GPL or the LGPL, and not to allow others to
#  use your version of this file under the terms of the MPL, indicate your
#  decision by deleting the provisions above and replace them with the notice
#  and other provisions required by the GPL or the LGPL. If you do not delete
#  the provisions above, a recipient may use your version of this file under
#  the terms of any one of the MPL, the GPL or the LGPL.
# 
#  ***** END LICENSE BLOCK ****

import sys,socket,os,time

port=None
host=None
if os.environ.has_key("SHELLPORT"):
    try:
        port=int(os.environ.get("SHELLPORT"))
    except:
        print("error: parsing SHELLPORT")
if os.environ.has_key("SHELLSERVER"):
    host=os.environ.get("SHELLSERVER")

if (host==None or port==None):
    print("error: SHELLPORT and SHELLSERVER must be set")
    sys.exit(1)

args=""
for item in sys.argv[1:]:
    args+=item+" "


s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))
s.send("%s" % args)
result=''
timeout=300
starttime=time.time()
exitcode=0
while True:
    result+=s.recv(1024)
    if result.find("setup finished")>-1 or result.find("ok:")>-1:
        break
    if result.find("setup failed")>-1 or result.find("error:")>-1:
        exitcode=1
        break
    if time.time()-starttime>timeout:
        print("ERROR: timed out after %d sec" % timeout)
        exitcode=1
        break
print(result)
s.close()
sys.exit(exitcode)



