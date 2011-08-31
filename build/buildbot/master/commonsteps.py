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
# a
#  The Original Code is [Open Source Virtual Machine.].
# 
#  The Initial Developer of the Original Code is
#  Adobe System Incorporated.
#  Portions created by the Initial Developer are Copyright (C) 2009-2010
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

from buildbot.process import factory
from buildbot.steps.source import Mercurial
from buildbot.steps.shell import *
from custom.buildbot_ext.steps.shellAddons import *
from buildbot.steps.trigger import Trigger


####### Utility Functions for ignoring certain file types
def startPerformanceRun(change):
    for name in change.files:
        if name.endswith(('.cpp','.h','.py','asm','.as')):
            return True
        elif name.endswith('.abc'):
            # Only run if abc is not in esc dir
            if '/esc/' not in name:
                return True
    return False
    
def startCompile(change):
    '''Determine whether we want to start a compile pass based on the files that
        have changed.  Only skip compile if ALL changes are in the ignore criteria.
    '''
    compile = True
    for name in change.files:
        # ignore all changes to buildbot master files
        if ('/buildbot/master/' in name) or ('utils/hooks/' in name):
            compile = False
        # ignore changes to the runsmokes*.txt files
        elif ('runsmokes' in name) and name.endswith('.txt'):
            compile = False
        else:
            return True
    return compile



############################
####### COMMON BUILD STEPS
############################

# For an explanation of the WithProperties('(%(silent:-)s') syntax see
# http://djmitche.github.com/buildbot/docs/current/#Using-Build-Properties

def compile_generic(name, shellname, args, upload):
    # factory.addStep(compile_generic(name="Release", shellname="avmshell", args="--enable-shell", upload="false"))
    # upload: if true build will be uploaded to asteam, this is normaly done in the upload buildstep
    #         but is here for shells that are compiled in deep-testing
    return BuildShellCommand(
            command=['../all/compile-generic.sh', WithProperties('%s','revision'), '%s' % args, '%s' % shellname, '%s' % upload],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting %s build...' % name,
            descriptionDone='finished %s build' % name,
            name="Build_%s" % name,
            workdir="../repo/build/buildbot/slaves/scripts"
            )


def test_generic(name, shellname, vmargs, config, scriptargs):
    # factory.addStep(test_generic("Release", "avmshell", "", "", ""))
    return TestSuiteShellCommand(
            command=['../all/run-acceptance-generic.sh', WithProperties('%s','revision'), '%s' % shellname, '%s' % vmargs, '%s' % config, '%s' % scriptargs],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to run %s vmtests...' % name,
            descriptionDone='finished %s vmtests' % name,
            name="Testsuite_%s" % name,
            workdir="../repo/build/buildbot/slaves/scripts"
            )
    

def test_emulator_generic(name, shellname, vmargs, config, scriptargs):
    # factory.addStep(test_emulator_generic("Release", "avmshell", "", "", ""))
    return TestSuiteShellCommand(
            command=['../all/run-acceptance-emulator-generic.sh', WithProperties('%s','revision'), '%s' % shellname, '%s' % vmargs, '%s' % config, '%s' % scriptargs],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to run %s vmtests...' % name,
            descriptionDone='finished %s vmtests' % name,
            name="Testsuite_%s" % name,
            workdir="../repo/build/buildbot/slaves/scripts"
            )


sync_clean = ShellCommand(
            command=["rm", "-Rf", "repo"],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Remove the old repository...',
            descriptionDone='Finished Removing the old repository',
            name='Source_Clean',
            workdir="../",
            haltOnFailure="True")

def sync_clone(url):
    return ShellCommand(
            command=["hg", "clone", url, "repo"],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Cloning the source repository...',
            descriptionDone='Finished cloning the source repository',
            name='Source_Clone',
            workdir="../",
            haltOnFailure="True")


sync_clone_sandbox = SandboxClone(
            dest="repo",
            changeDir="changes/processed",
            description='Cloning the source repository...',
            descriptionDone='Finished cloning the source repository',
            name='Source_Clone',
            workdir="../",
            haltOnFailure="True")

sync_update = ShellCommand(
            command=["hg", "update", "--clean",  "--rev", WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Updating the source repository...',
            descriptionDone='Finished updating the source repository',
            name='Source_Update',
            workdir="../repo",
            haltOnFailure="True")

def bb_slaveupdate(slave):
    return ShellCommand(
            command=['cp','-R','repo/build/buildbot/slaves/%s/scripts' % slave, 'repo/build/buildbot/slaves/scripts'],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            workdir='../',
            description='Updating SLAVE buildscripts',
            name='BB_SLAVEUpdate',
            haltOnFailure="True"
            )

bb_lockacquire = BuildShellCommand(
            command=['../all/lock-acquire.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Acquiring machine lock...',
            descriptionDone='Acquired machine lock...',
            name="LockAcquire",
            workdir="../repo/build/buildbot/slaves/scripts")

bb_lockrelease = BuildShellCommand(
            command=['../all/lock-release.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Releasing machine lock...',
            descriptionDone='Released machine lock...',
            name="LockRelease",
            workdir="../repo/build/buildbot/slaves/scripts")

compile_builtin = BuildShellCommand(
            command=['../all/build-builtinabc.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to build builtin.abc..',
            descriptionDone='builtin.abc build',
            name="Compile_builtin.abc",
            workdir="../repo/build/buildbot/slaves/scripts")

compile_buildcheck = BuildShellCheckCommand(
            command=['../all/build-check.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting build check...',
            descriptionDone='build check completed',
            name='Build_Check',
            workdir="../repo/build/buildbot/slaves/scripts")

## Local version runs a local script and not the common
compile_buildcheck_local = BuildShellCheckCommand(
            command=['./build-check.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting build check...',
            descriptionDone='build check completed',
            name='Build_Check',
            workdir="../repo/build/buildbot/slaves/scripts")

compile_testmedia = BuildShellCommand(
            command=['../all/build-acceptance-tests.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to build test abcs...',
            descriptionDone='vm test abcs built.',
            name="Compile_AS_testcases",
            workdir="../repo/build/buildbot/slaves/scripts")

download_testmedia = BuildShellCommand(
            command=['../all/download-acceptance-tests.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to downloads test abcs...',
            descriptionDone='vm test abcs downloaded.',
            name="Download_AS_testcases",
            workdir="../repo/build/buildbot/slaves/scripts",
            haltOnFailure="True" )

test_smoke = TestSuiteShellCommand(
            command=['../all/run-smoketests.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to run smoke tests...',
            descriptionDone='finished smoke tests.',
            name="SmokeTest",
            workdir="../repo/build/buildbot/slaves/scripts")

test_emulator_smoke_mobile = TestSuiteShellCommand(
            command=['../all/run-smoketests-arm-emulator.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to run smoke tests...',
            descriptionDone='finished smoke tests.',
            name="SmokeTest",
            workdir="../repo/build/buildbot/slaves/scripts")

test_selftest = TestSuiteShellCommand(
            command=['../all/run-selftest.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting selftest release...',
            descriptionDone='finished selftest release.',
            name="Testsuite_Selftest",
            workdir="../repo/build/buildbot/slaves/scripts")

test_commandline = TestSuiteShellCommand(
            command=['../all/run-commandline-tests.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting commandline tests...',
            descriptionDone='finished commandline tests.',
            name="Testsuite_Commandline",
            workdir="../repo/build/buildbot/slaves/scripts")

test_differential = TestSuiteShellCommand(
            command=['../all/run-acceptance-avmdiff.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting vm acceptance differential testing...',
            descriptionDone='finished vm acceptance differential testing.',
            name="Testsuite_Differential",
            workdir="../repo/build/buildbot/slaves/scripts")

test_misc = TestSuiteShellCommand(
            command=['../all/run-misc-tests.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting to run misc tests...',
            descriptionDone='finished misc tests.',
            name="MiscTest",
            workdir="../repo/build/buildbot/slaves/scripts")

util_upload_asteam = BuildShellCheckCommand(
            command=['../all/upload-asteam.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Upload bits to ASTEAM...',
            descriptionDone='Upload to ASTEAM completed',
            name='Upload_ASTEAM',
            workdir="../repo/build/buildbot/slaves/scripts")

## Local version runs a local script and not the common
util_upload_asteam_local = BuildShellCheckCommand(
            command=['./upload-asteam.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Upload bits to ASTEAM...',
            descriptionDone='Upload to ASTEAM completed',
            name='Upload_ASTEAM',
            workdir="../repo/build/buildbot/slaves/scripts")

util_upload_mozilla = BuildShellCheckCommand(
            command=['../all/upload-mozilla.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Upload bits to MOZILLA...',
            descriptionDone='Upload to MOZILLA completed',
            name='Upload_MOZILLA',
            workdir="../repo/build/buildbot/slaves/scripts")

## Local version runs a local script and not the common
util_upload_mozilla_local = BuildShellCheckCommand(
            command=['./upload-mozilla.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Upload bits to MOZILLA...',
            descriptionDone='Upload to MOZILLA completed',
            name='Upload_MOZILLA',
            workdir="../repo/build/buildbot/slaves/scripts")

util_process_clean = BuildShellCommand(
            command=['../all/util-process-clean.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Zombie hunting...',
            descriptionDone='Zombie hunt completed',
            name='Util_ZombieKiller',
            workdir="../repo/build/buildbot/slaves/scripts",
            alwaysRun="True" )

util_clean_buildsdir = BuildShellCommand(
            command=['../all/util-clean-buildsdir.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Remove build folders older than one week...',
            descriptionDone='Finished removing build folders',
            name="Clean_Build_Dir",            
            workdir="../repo/build/buildbot/slaves/scripts",
            alwaysRun="True" )

perf_prepare = BuildShellCommand(
            command=['./prepare.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='Preparing for performance run...',
            descriptionDone='Preparation complete...',
            name="Preparation",
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release = PerfShellCommand(
            command=['../all/run-performance-release.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release performance tests...',
            descriptionDone='finished release performance tests.',
            name='Release',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release_arm = PerfShellCommand(
            command=['../all/run-performance-release-arm.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release performance tests...',
            descriptionDone='finished release performance tests.',
            name='Release',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release_interp = PerfShellCommand(
            command=['../all/run-performance-release-interp.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release-interp performance tests...',
            descriptionDone='finished release-interp performance tests.',
            name='ReleaseInterp',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release_arm_interp = PerfShellCommand(
            command=['../all/run-performance-release-arm-interp.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release-interp performance tests...',
            descriptionDone='finished release-interp performance tests.',
            name='ReleaseInterp',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release_jit = PerfShellCommand(
            command=['../all/run-performance-release-jit.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release-jit performance tests...',
            descriptionDone='finished release-jit performance tests.',
            name='ReleaseJIT',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

perf_release_arm_jit = PerfShellCommand(
            command=['../all/run-performance-release-arm-jit.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release-jit performance tests...',
            descriptionDone='finished release-jit performance tests.',
            name='ReleaseJIT',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

deep_codecoverage = BuildShellCommand(
            command=['./run-code-coverage.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting code coverage...',
            descriptionDone='finished code coverage',
            name='CodeCoverage',
            timeout=3600,
            workdir="../repo/build/buildbot/slaves/scripts")

deep_release_esc = BuildShellCommand(
            command=['../all/run-release-esc.sh', WithProperties('%s','revision')],
            env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
            description='starting release-esc tests...',
            descriptionDone='finished release-esc tests.',
            name='Release-esc',
            workdir="../repo/build/buildbot/slaves/scripts")
            

