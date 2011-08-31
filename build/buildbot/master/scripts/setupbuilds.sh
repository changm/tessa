#!/bin/bash
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
#  Portions created by the Initial Developer are Copyright (C) 2007-2010
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
(set -o igncr) 2>/dev/null && set -o igncr; # comment is needed

branch=$1
buildnum=$2

test "$branch" = "" && {
    echo "You must specify a branch. usage: setupbuilds.sh BRANCH NUMBER"
    exit 0
}

test "$buildnum" = "" && {
    echo "You must specify a build number. usage: setupbuilds.sh BRANCH NUMBER"
    exit 0
}


keeplength="10"
tamarin=/home/ftp/pub/js/tamarin/builds
dir=`ls $tamarin`

# If the directory has already been created, exit out
test -d "$tamarin/$branch/$buildnum" &&
    exit 0

# Clean up the builds, only maintianing the last ${keeplength} number of builds
while true
do
    ls $tamarin/$branch | grep -v latest > build.files
    sz=`wc -l build.files | awk '{print $1}'`
    if [ "$sz" -ge "$keeplength" ]
    then
        last=`cat build.files | sort -n | head -1`
        echo "deleting directory $tamarin/$branch/$last"
        rm -fr $tamarin/$branch/$last
    else
        break
    fi
done


# Since each build slave calls this script, we only need to create the dirs
# if this is the first time calling the script for this build number
test "$buildnum" = "" || {
    echo "creating build directory $branch/$buildnum"
    mkdir $tamarin/$branch/$buildnum
    mkdir $tamarin/$branch/$buildnum/windows
    mkdir $tamarin/$branch/$buildnum/mac
    mkdir $tamarin/$branch/$buildnum/linux
    mkdir $tamarin/$branch/$buildnum/winmobile
    mkdir $tamarin/$branch/$buildnum/solaris
    mkdir $tamarin/$branch/$buildnum/android
    chmod -R 777 $tamarin/$branch
}
