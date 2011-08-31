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
#  Portions created by the Initial Developer are Copyright (C) 2009
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

#export vmbuilds=ftp://ftp.mozilla.org/pub/js/tamarin/builds
export vmbuilds=ftp://10.60.48.47/
#export ascbuilds=ftp://ftp.mozilla.org/pub/js/tamarin/builds/asc/latest
export ascbuilds=ftp://10.60.48.47/asc/latest/

## Used by performance slaves for locking
export sleeptime=`expr $RANDOM % 60 + 60`

## Used by make in the build scripts
export make_opt=""

## Only set the branch if it has not already been set
echo branch [before]: $branch
if [ "$branch" == "" ]; then
    export branch=tamarin-redux
fi
export branch=`echo $branch | sed 's/-deep//' | sed 's/-performance//'`
echo branch [after]: $branch

export shellABC=shell_toplevel.abc
export builtinABC=builtin.abc


workdir=`pwd`

# create the builds directory if it doesn't exist
if [ ! -d "${workdir}/../../../../../builds" ]; then
    mkdir -p ${workdir}/../../../../../builds
fi

if [ "$OSTYPE" == "cygwin" ]; then
    export basedir=`cygpath -m ${workdir}/../../../..`
    export buildsdir=`cygpath -m ${basedir}/../builds`
else
    export basedir=`cd ${workdir}/../../../..; pwd`
    export buildsdir=`cd ${basedir}/../builds; pwd`
fi

# builds for windows mobile (used by emulator)
export shell_release_arm=avmshell_arm$shell_extension


export shell=avmshell$shell_extension
export shell_selftest=avmshell$shell_extension
export shell_release=avmshell$shell_extension
export shell_release_wordcode=avmshell_wordcode$shell_extension
export shell_debug=avmshell_d$shell_extension
export shell_release_debugger=avmshell_s$shell_extension
export shell_debug_debugger=avmshell_sd$shell_extension

export shell_release_64=avmshell_64$shell_extension
export shell_release_wordcode_64=avmshell_wordcode_64$shell_extension
export shell_debug_64=avmshell_d_64$shell_extension
export shell_release_debugger_64=avmshell_s_64$shell_extension
export shell_debug_debugger_64=avmshell_sd_64$shell_extension
export shell_selftest_64=avmshell_64$shell_extension

export shell_release_min=avmshell_min$shell_extension
export shell_debug_min=avmshell_d_min$shell_extension

export jit="-Ojit"
export interp="-Dinterp"


export ftp_asteam=10.60.48.47

export scp_mozilla="stage.mozilla.org:/home/ftp/pub/js/tamarin/builds"


baselineBranchTime=tamarin-flash
baselineConfigTime=
baselineBuildTime=458340
baselineConfigTimeJIT=${mir}
baselineBuildTimeJIT=458340
baselineConfigTimeInterp=${interp}
baselineBuildTimeInterp=458340

baslineBranchMemory=${branch}
baselineConfigMemory=
baselineBuildMemory=966
baselineConfigMemoryJIT=${jit}
baselineBuildMemoryJIT=966
baselineConfigMemoryInterp=${interp}
baselineBuildMemoryInterp=966

configDefault=
configJIT=${jit}
configInterp=${interp}

# List of processes that should NEVER be running when the build is not
# currently running any tests. This list of process will be killed if the
# process is found. Process must not contain extension as cygwin will return
# the process without the extension. Used in all/util-process-clean.sh
export proc_names="avmshell ceremote"

# Helper functions to supress output
function beginSilent () {
    test "$silent" = "true" && {
        test "$logfile" = "" && {
            echo "logfile not defined, using generic name: output.log"
            logfile=output.log
        }
        # create log file in builds dir
        mkdir -p $buildsdir/${change}-${changeid}/$platform
        logfile=$buildsdir/${change}-${changeid}/$platform/$logfile
        touch $logfile
        exec 6>&1                # Link file descriptor #6 with stdout. (Saves stdout)
        exec >> $logfile 2>&1    # Redirect all stdout and stderr output to be appended to $logfile
    }
}

function endSilent () {
    test "$silent" = "true" && {
        . $workdir/../all/util-upload-ftp-asteam.sh $logfile $ftp_asteam/$branch/${change}-${changeid}/$platform/
        exec 1>&6 6>&-      # Restore stdout and close file descriptor #6.
        # ${logfile##*/} == filename from end of path
        echo "Build log can be found here: http://asteam.macromedia.com/builds/$branch/${change}-${changeid}/$platform/${logfile##*/}"
    }
}


