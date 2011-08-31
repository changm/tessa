#!/bin/bash
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is [Open Source Virtual Machine.].
#
# The Initial Developer of the Original Code is
# Adobe System Incorporated.
# Portions created by the Initial Developer are Copyright (C) 2010
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Adobe AS3 Team
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK ***** */
# acts as a proxy to remotely run abc files via a ssh call to the remote machine
# usage: ./ssh_shell.sh <vmargs> file.abc
# assumes the shell is deployed to $SSH_SHELL_REMOTE_DIR/avmshell
#


if [ "$SSH_SHELL_REMOTE_USER" = "" ] ||
   [ "$SSH_SHELL_REMOTE_HOST" = "" ] ||
   [ "$SSH_SHELL_REMOTE_DIR" = "" ];
then
    echo "missing environment variable: "
    echo "SSH_SHELL_REMOTE_USER" = "$SSH_SHELL_REMOTE_USER"
    echo "SSH_SHELL_REMOTE_HOST" = "$SSH_SHELL_REMOTE_HOST"
    echo "SSH_SHELL_REMOTE_DIR" = "$SSH_SHELL_REMOTE_DIR"
    exit 1
fi

filelist=""
flatfilelist=""
if [ "$1" = "" ]
then
    ssh $SSH_SHELL_REMOTE_USER@$SSH_SHELL_REMOTE_HOST "cd $SSH_SHELL_REMOTE_DIR;./avmshell"
else
    args=""
    for a in $*
    do
       echo "$a" | grep ".*\.abc" > /dev/null
       res=$?
       if [ "$res" = "0" ]
       then
           file=$a
           flatfile=`basename $a`
           filelist="$filelist $flatfile"
           scp $file $SSH_SHELL_REMOTE_USER@$SSH_SHELL_REMOTE_HOST:$SSH_SHELL_REMOTE_DIR/$flatfile > /dev/null
           args="$args $flatfile"       
       else
           args="$args $a"
       fi
    done
    # workaround for not returning exit code, run a shell script and print exit code to stdout
    ssh $SSH_SHELL_REMOTE_USER@$SSH_SHELL_REMOTE_HOST "cd $SSH_SHELL_REMOTE_DIR;./ssh-shell-runner.sh $args" > /tmp/stdout
    ret=`cat /tmp/stdout | grep "EXITCODE=" | awk -F= '{printf("%d",$2)}'`
    for a in $filelist
    do
        ssh $SSH_SHELL_REMOTE_USER@$SSH_SHELL_REMOTE_HOST "cd $SSH_SHELL_REMOTE_DIR;rm $a"
    done
    cat /tmp/stdout
    rm -f /tmp/stdout
    exit $ret
fi
