#! /usr/bin/python
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
#
#  Hook script used by tamarin team on tamarin-redux and tamarin-central.
#
# For documentation on hook scripts see:
#   http://hgbook.red-bean.com/read/handling-repository-events-with-hooks.html
#   http://mercurial.selenic.com/wiki/MercurialApi

# This file is to be run using a pretxncommit hook
# Place this in your .hg/hgrc file in the repo:
#
# [hooks]
# pretxncommit.master = python:/path/to/tamarin-commit-hook.py:master_hook


import sys, re, os
from mercurial import hg, ui, commands, cmdutil, patch
from mercurial.node import hex, short

def master_hook(ui, repo, **kwargs):
    ui.debug('running tamarin master_hook\n')
    ui.debug('kwargs: %s\n' % kwargs)
    # The mercurial hook script expects the equivalent of an exit code back from
    # this call:
    #   False = 0 = No Error : allow push
    #   True = 1 = Error : abort push
    error = False
    error = error or diff_check(ui, repo, **kwargs)

    if error:
        # Save the commit message so it can be reused by user
        desc = repo[repo[kwargs['node']].rev()].description()
        ui.debug('Description: %s\n' % desc)
        try:
            f = open('%s/.hg/commit.save' % repo.root, 'w')
            f.write(desc)
            f.close()
            ui.warn('Commit message saved to .hg/commit.save\nSaved message can be recommitted using -l .hg/commit.save\n')
        except IOError:
            ui.warn('Error writing .hg/commit.save file')

    return error

def diff_check(ui, repo, **kwargs):
    ui.debug('running diff_check\n')

    # get all the change contexts for this commit
    # kwargs['node'] returns the first changecontext nodeid
    changecontexts = [repo[i] for i in range(repo[kwargs['node']].rev(), len(repo))]
    # check for tabs
    def tabCheck(line):
        tab = line.find('\t')
        if tab >= 0:    # find returns -1 if not found
            return True, tab
        return False, tab

    def windowsLineendingsCheck(line):
        if line.endswith('\r'):
            return True, len(line)-1
        return False, 0

    def trailingWhitespaceCheck(line):
        if len(line.strip()) > 1:   # skip empty lines (will have a +)
            m = re.match(r'\+.*?(\s+$)', line)
            if m:
                return True, m.start(1)
        return False, 0

    # check for tabs - exit if user chooses to abort
    if checkChangeCtxDiff(ui, repo, changecontexts, tabCheck,
                          'Tab', ('.cpp', '.c', '.h', '.as', '.abs', '.py')):
        return True

    if checkChangeCtxDiff(ui, repo, changecontexts, windowsLineendingsCheck,
                          'Windows line ending', ('.cpp', '.c', '.h', '.as', '.abs', '.py')):
        return True

    if checkChangeCtxDiff(ui, repo, changecontexts, trailingWhitespaceCheck,
                          'Trailing Whitespace', ('.cpp', '.c', '.h', '.as', '.abs', '.py')):
        return True

    return False

def checkChangeCtxDiff(ui, repo, changecontexts, testFunc, testDesc, fileEndings):
    '''Loop through each diff for each change and run the testFunc against each line'''
    ui.debug('Checking %s\n' % testDesc)
    for ctx in changecontexts:
        # Get the diff for each change and file
        for file in [f for f in ctx.files() if f.endswith(fileEndings)]:
            ui.debug('checking change: %s, file: %s\n' % (short(ctx.node()), file))
            fmatch = cmdutil.matchfiles(repo,[file])
            # diff from this nodes parent to current node
            diff = ''.join(patch.diff(repo, ctx.parents()[0].node(), ctx.node(), fmatch)).split('\n')
            for i in range(3, len(diff)):    # start checking after diff header
                line = diff[i]
                if line.startswith('@@'):
                    diffLocation = line
                # only check new lines added/modified in the file
                if line.startswith('+'):
                    ui.debug('\nchecking line for %s: %s\n\n' % (testDesc, line))
                    testResult, errorLocation = testFunc(line)
                    if testResult:
                        ui.warn('\n%s(s) found in %s for rev %s (change %s):\n' %
                                (testDesc, file, ctx.rev(), short(ctx.node())))
                        ui.warn('%s\n' % diffLocation)
                        ui.warn('%s\n' % line)
                        ui.warn('%s^\n' % (' '*errorLocation,)) # show a pointer to error
                        try:
                            response = ui.promptchoice('(n)o, (y)es, (a)llow %ss for current file\n' % testDesc +
                                                    'Are you sure you want to commit this change? [n]: ' ,
                                                   (('&No'), ('&Yes'), ('&Allow')), 0)
                        except AttributeError:
                            ui.warn('This commit hook requires that you have mercurial 1.4+ installed.  Please upgrade your hg installation.')
                            response = 0
                        if response == 1:
                            # next occurance in file
                            continue
                        elif response == 2:
                            # next file
                            break
                        else:
                            ui.warn('Aborting commit due to %s.\n' % testDesc)
                            # error = True
                            return True
    return False
