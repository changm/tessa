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

from commonsteps import *

class sandbox:
    
    BRANCH = "sandbox"
    
    ####### SCHEDULERS
    from buildbot.scheduler import *
    # custom.buildbot_ext.scheduler import MUST happen after importing buildbot.scheduler
    from custom.buildbot_ext.scheduler import *
    
    #### SANDBOX
    compile = Scheduler(name="compile-sandbox", branch=BRANCH, treeStableTimer=30, properties={'silent':'true'},
                     builderNames=["windows-compile-sandbox", "windows64-compile-sandbox",
                                   "mac-intel-10.4-compile-sandbox", "mac-intel-10.5-compile-sandbox", "mac64-intel-compile-sandbox",
                                   "mac-ppc-10.4a-compile-sandbox", "mac-ppc-10.4b-compile-sandbox", 
                                   "mac-ppc-10.5a-compile-sandbox", "mac-ppc-10.5b-compile-sandbox", 
                                   "mac64-ppc-compile-sandbox",
                                   "linux-compile-sandbox", "linux64-compile-sandbox",
                                   "winmobile-emulator-compile-sandbox",
                                   "solaris-sparc-compile-sandbox", "solaris-sparc2-compile-sandbox",
                                   "android-compile-sandbox",
                                   "linux-arm-compile-sandbox", "linux-arm2-compile-sandbox",
                                   ])

    smoke = BuilderDependent(name="smoke-sandbox",upstream=compile, callbackInterval=60, properties={'silent':'true'},
                    builderNames=["windows-smoke-sandbox", "windows64-smoke-sandbox",
                                   "mac-intel-10.4-smoke-sandbox", "mac-intel-10.5-smoke-sandbox", "mac64-intel-smoke-sandbox",
                                   "mac-ppc-10.4a-smoke-sandbox", "mac-ppc-10.4b-smoke-sandbox", 
                                   "mac-ppc-10.5a-smoke-sandbox", "mac-ppc-10.5b-smoke-sandbox", 
                                   "mac64-ppc-smoke-sandbox",
                                   "linux-smoke-sandbox", "linux64-smoke-sandbox",
                                   "winmobile-emulator-smoke-sandbox",
                                   "solaris-sparc-smoke-sandbox", "solaris-sparc2-smoke-sandbox",
                                   "android-smoke-sandbox",
                                   "linux-arm-smoke-sandbox", "linux-arm2-smoke-sandbox"],
                    builderDependencies=[
                                  ["windows-smoke-sandbox", "windows-compile-sandbox"], 
                                  ["windows64-smoke-sandbox", "windows64-compile-sandbox"], 
                                  ["mac-intel-10.4-smoke-sandbox", "mac-intel-10.4-compile-sandbox"], 
                                  ["mac-intel-10.5-smoke-sandbox", "mac-intel-10.5-compile-sandbox"],
                                  ["mac64-intel-smoke-sandbox", "mac64-intel-compile-sandbox"],
                                  ["mac-ppc-10.4a-smoke-sandbox", "mac-intel-10.4-compile-sandbox"],
                                  ["mac-ppc-10.4b-smoke-sandbox", "mac-intel-10.4-compile-sandbox"],
                                  ["mac-ppc-10.5a-smoke-sandbox", "mac-intel-10.5-compile-sandbox"],
                                  ["mac-ppc-10.5b-smoke-sandbox", "mac-intel-10.5-compile-sandbox"],
                                  ["mac64-ppc-smoke-sandbox", "mac64-intel-compile-sandbox"],
                                  ["linux-smoke-sandbox", "linux-compile-sandbox"],
                                  ["linux64-smoke-sandbox", "linux64-compile-sandbox"],
                                  ["winmobile-emulator-smoke-sandbox", "winmobile-emulator-compile-sandbox"],
                                  ["solaris-sparc-smoke-sandbox", "solaris-sparc-compile-sandbox"],
                                  ["solaris-sparc2-smoke-sandbox", "solaris-sparc-compile-sandbox"],
                                  ["android-smoke-sandbox","android-compile-sandbox"],
                                  ["linux-arm-smoke-sandbox","linux-compile-sandbox"],
                                  ["linux-arm2-smoke-sandbox","linux-compile-sandbox"],
                                 ])

    test = BuilderDependent(name="test-sandbox",upstream=smoke, callbackInterval=60, properties={'silent':'true'},
                    builderNames=["windows-test-sandbox", "windows64-test-sandbox",
                                   "mac-intel-10.4-test-sandbox", "mac-intel-10.5-test-sandbox", "mac64-intel-test-sandbox",
                                   "mac-ppc-10.4a-test-sandbox", "mac-ppc-10.4b-test-sandbox", 
                                   "mac-ppc-10.5a-test-sandbox", "mac-ppc-10.5b-test-sandbox", 
                                   "mac64-ppc-test-sandbox",
                                   "linux-test-sandbox", "linux64-test-sandbox",
                                   "winmobile-emulator-test-sandbox",
                                   "solaris-sparc-test-sandbox", "solaris-sparc2-test-sandbox",
                                   "android-test-sandbox",
                                   "linux-arm-test-sandbox", "linux-arm2-test-sandbox"],
                    builderDependencies=[
                                  ["windows-test-sandbox", "windows-smoke-sandbox"], 
                                  ["windows64-test-sandbox", "windows64-smoke-sandbox"], 
                                  ["mac-intel-10.4-test-sandbox", "mac-intel-10.4-smoke-sandbox"], 
                                  ["mac-intel-10.5-test-sandbox", "mac-intel-10.5-smoke-sandbox"],
                                  ["mac64-intel-test-sandbox", "mac64-intel-smoke-sandbox"],
                                  ["mac-ppc-10.4a-test-sandbox", "mac-ppc-10.4a-smoke-sandbox"],
                                  ["mac-ppc-10.4b-test-sandbox", "mac-ppc-10.4b-smoke-sandbox"],
                                  ["mac-ppc-10.5a-test-sandbox", "mac-ppc-10.5a-smoke-sandbox"],
                                  ["mac-ppc-10.5b-test-sandbox", "mac-ppc-10.5b-smoke-sandbox"],
                                  ["mac64-ppc-test-sandbox", "mac64-ppc-smoke-sandbox"],
                                  ["linux-test-sandbox", "linux-smoke-sandbox"],
                                  ["linux64-test-sandbox", "linux64-smoke-sandbox"],
                                  ["winmobile-emulator-test-sandbox", "winmobile-emulator-smoke-sandbox"],
                                  ["solaris-sparc-test-sandbox", "solaris-sparc-smoke-sandbox"],
                                  ["solaris-sparc2-test-sandbox", "solaris-sparc2-smoke-sandbox"],
                                  ["android-test-sandbox", "android-smoke-sandbox"],
                                  ["linux-arm-test-sandbox", "linux-arm-smoke-sandbox"],
                                  ["linux-arm2-test-sandbox", "linux-arm2-smoke-sandbox"],
                                 ])

    schedulers = [compile, smoke, test]


    ################################################################################
    ################################################################################
    ####                                                                        ####
    ####                    SANDBOX COMPILE BUILDERS                            ####
    ####                                                                        ####
    ################################################################################
    ################################################################################

    #############################################
    #### builder for windows-compile-sandbox ####
    #############################################
    sb_windows_compile_factory = factory.BuildFactory()
    sb_windows_compile_factory.addStep(sync_clean)
    sb_windows_compile_factory.addStep(sync_clone_sandbox)
    sb_windows_compile_factory.addStep(sync_update)
    sb_windows_compile_factory.addStep(bb_slaveupdate(slave="windows"))
    sb_windows_compile_factory.addStep(compile_builtin)
    sb_windows_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell", args="--enable-shell", upload="false"))
    sb_windows_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_windows_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d", args="--enable-shell --enable-debug", upload="false"))
    sb_windows_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s", args="--enable-shell --enable-debugger", upload="false"))
    sb_windows_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_windows_compile_factory.addStep(BuildShellCommand(
                command=['../all/file-check.py', '../../../../../repo'],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='running file-check against source...',
                descriptionDone='finished file-check.',
                name="FileCheck",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_windows_compile_factory.addStep(compile_buildcheck)
    sb_windows_compile_factory.addStep(util_upload_asteam)


    sb_windows_compile_builder = {
                'name': "windows-compile-sandbox",
                'slavename': "asteamwin2",
                'factory': sb_windows_compile_factory,
                'builddir': './sandbox-windows-compile',
    }


    ###############################################
    #### builder for windows64-compile-sandbox ####
    ###############################################
    sb_windows_64_compile_factory = factory.BuildFactory()
    sb_windows_64_compile_factory.addStep(sync_clean)
    sb_windows_64_compile_factory.addStep(sync_clone_sandbox)
    sb_windows_64_compile_factory.addStep(sync_update)
    sb_windows_64_compile_factory.addStep(bb_slaveupdate(slave="windows64"))

    sb_windows_64_compile_builder = {
                'name': "windows64-compile-sandbox",
                'slavename': "asteamwin3",
                'factory': sb_windows_64_compile_factory,
                'builddir': './sandbox-windows64-compile',
    }


    ####################################################
    #### builder for mac-intel-10_4-compile-sandbox ####
    ####################################################
    sb_mac_intel_104_compile_factory = factory.BuildFactory()
    sb_mac_intel_104_compile_factory.addStep(sync_clean)
    sb_mac_intel_104_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_intel_104_compile_factory.addStep(sync_update)
    sb_mac_intel_104_compile_factory.addStep(bb_slaveupdate(slave="mac-intel-10_4"))
    sb_mac_intel_104_compile_factory.addStep(compile_builtin)
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell_104", args="--enable-shell", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode_104", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d_104", args="--enable-shell --enable-debug", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s_104", args="--enable-shell --enable-debugger", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd_104", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Release_PPC", shellname="avmshell_104_ppc", args="--enable-shell --target=ppc-darwin", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Release-wordcode_PPC", shellname="avmshell_wordcode_104_ppc", args="--enable-shell --enable-wordcode-interp --target=ppc-darwin", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="Debug_PPC", shellname="avmshell_d_104_ppc", args="--enable-shell --enable-debug --target=ppc-darwin", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="ReleaseDebugger_PPC", shellname="avmshell_s_104_ppc", args="--enable-shell --enable-debugger --target=ppc-darwin", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_generic(name="DebugDebugger_PPC", shellname="avmshell_sd_104_ppc", args="--enable-shell --enable-debug --enable-debugger --target=ppc-darwin", upload="false"))
    sb_mac_intel_104_compile_factory.addStep(compile_buildcheck_local)
    sb_mac_intel_104_compile_factory.addStep(util_upload_asteam_local)

    sb_mac_intel_104_compile_builder = {
                'name': "mac-intel-10.4-compile-sandbox",
                'slavename': "asteammac4",
                'factory': sb_mac_intel_104_compile_factory,
                'builddir': './sandbox-mac-intel-10_4-compile',
    }


    ####################################################
    #### builder for mac-intel-10_5-compile-sandbox ####
    ####################################################
    sb_mac_intel_105_compile_factory = factory.BuildFactory()
    sb_mac_intel_105_compile_factory.addStep(sync_clean)
    sb_mac_intel_105_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_intel_105_compile_factory.addStep(sync_update)
    sb_mac_intel_105_compile_factory.addStep(bb_slaveupdate(slave="mac-intel-10_5"))
    sb_mac_intel_105_compile_factory.addStep(compile_builtin)
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell", args="--enable-shell", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d", args="--enable-shell --enable-debug", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s", args="--enable-shell --enable-debugger", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Release_PPC", shellname="avmshell_ppc", args="--enable-shell --target=ppc-darwin", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Release-wordcode_PPC", shellname="avmshell_wordcode_ppc", args="--enable-shell --enable-wordcode-interp --target=ppc-darwin", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="Debug_PPC", shellname="avmshell_d_ppc", args="--enable-shell --enable-debug --target=ppc-darwin", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="ReleaseDebugger_PPC", shellname="avmshell_s_ppc", args="--enable-shell --enable-debugger --target=ppc-darwin", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_generic(name="DebugDebugger_PPC", shellname="avmshell_sd_ppc", args="--enable-shell --enable-debug --enable-debugger --target=ppc-darwin", upload="false"))
    sb_mac_intel_105_compile_factory.addStep(compile_buildcheck_local)
    sb_mac_intel_105_compile_factory.addStep(util_upload_asteam_local)

    sb_mac_intel_105_compile_builder = {
                'name': "mac-intel-10.5-compile-sandbox",
                'slavename': "asteammac1",
                'factory': sb_mac_intel_105_compile_factory,
                'builddir': './sandbox-mac-intel-10_5-compile',
    }


    ##################################################
    #### builder for mac-intel-64-compile-sandbox ####
    ##################################################
    sb_mac_intel_64_compile_factory = factory.BuildFactory()
    sb_mac_intel_64_compile_factory.addStep(sync_clean)
    sb_mac_intel_64_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_intel_64_compile_factory.addStep(sync_update)
    sb_mac_intel_64_compile_factory.addStep(bb_slaveupdate(slave="mac64-intel"))
    sb_mac_intel_64_compile_factory.addStep(compile_builtin)
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell_64", args="--enable-shell --target=x86_64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode_64", args="--enable-shell --enable-wordcode-interp --target=x86_64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d_64", args="--enable-shell --enable-debug --target=x86_64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s_64", args="--enable-shell --enable-debugger --target=x86_64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd_64", args="--enable-shell --enable-debug --enable-debugger --target=x86_64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Release_PPC", shellname="avmshell_64_ppc", args="--enable-shell --target=ppc64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Release-wordcode_PPC", shellname="avmshell_wordcode_64_ppc", args="--enable-shell --enable-wordcode-interp --target=ppc64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="Debug_PPC", shellname="avmshell_d_64_ppc", args="--enable-shell --enable-debug --target=ppc64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="ReleaseDebugger_PPC", shellname="avmshell_s_64_ppc", args="--enable-shell --enable-debugger --target=ppc64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_generic(name="DebugDebugger_PPC", shellname="avmshell_sd_64_ppc", args="--enable-shell --enable-debug --enable-debugger --target=ppc64-darwin", upload="false"))
    sb_mac_intel_64_compile_factory.addStep(compile_buildcheck_local)
    sb_mac_intel_64_compile_factory.addStep(util_upload_asteam_local)

    sb_mac_intel_64_compile_builder = {
                'name': "mac64-intel-compile-sandbox",
                'slavename': "asteammac1-64bit",
                'factory': sb_mac_intel_64_compile_factory,
                'builddir': './sandbox-mac64-intel-compile',
    }


    ###################################################
    #### builder for mac-ppc-10_4a-compile-sandbox ####
    ###################################################
    sb_mac_ppc_104a_compile_factory = factory.BuildFactory()
    sb_mac_ppc_104a_compile_factory.addStep(sync_clean)
    sb_mac_ppc_104a_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_ppc_104a_compile_factory.addStep(sync_update)
    sb_mac_ppc_104a_compile_factory.addStep(bb_slaveupdate(slave="mac-ppc-10_4"))

    sb_mac_ppc_104a_compile_builder = {
                'name': "mac-ppc-10.4a-compile-sandbox",
                'slavename': "asteammac6",
                'factory': sb_mac_ppc_104a_compile_factory,
                'builddir': './sandbox-mac-ppc-10_4a-compile',
    }


    ###################################################
    #### builder for mac-ppc-10_4b-compile-sandbox ####
    ###################################################
    sb_mac_ppc_104b_compile_factory = factory.BuildFactory()
    sb_mac_ppc_104b_compile_factory.addStep(sync_clean)
    sb_mac_ppc_104b_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_ppc_104b_compile_factory.addStep(sync_update)
    sb_mac_ppc_104b_compile_factory.addStep(bb_slaveupdate(slave="mac-ppc-10_4"))

    sb_mac_ppc_104b_compile_builder = {
                'name': "mac-ppc-10.4b-compile-sandbox",
                'slavename': "asteammac9",
                'factory': sb_mac_ppc_104b_compile_factory,
                'builddir': './sandbox-mac-ppc-10_4b-compile',
    }



    ###################################################
    #### builder for mac-ppc-10_5a-compile-sandbox ####
    ###################################################
    sb_mac_ppc_105a_compile_factory = factory.BuildFactory()
    sb_mac_ppc_105a_compile_factory.addStep(sync_clean)
    sb_mac_ppc_105a_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_ppc_105a_compile_factory.addStep(sync_update)
    sb_mac_ppc_105a_compile_factory.addStep(bb_slaveupdate(slave="mac-ppc-10_5"))

    sb_mac_ppc_105a_compile_builder = {
                'name': "mac-ppc-10.5a-compile-sandbox",
                'slavename': "tamarin-xserve",
                'factory': sb_mac_ppc_105a_compile_factory,
                'builddir': './sandbox-mac-ppc-10_5a-compile',
    }


    ###################################################
    #### builder for mac-ppc-10_5b-compile-sandbox ####
    ###################################################
    sb_mac_ppc_105b_compile_factory = factory.BuildFactory()
    sb_mac_ppc_105b_compile_factory.addStep(sync_clean)
    sb_mac_ppc_105b_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_ppc_105b_compile_factory.addStep(sync_update)
    sb_mac_ppc_105b_compile_factory.addStep(bb_slaveupdate(slave="mac-ppc-10_5"))

    sb_mac_ppc_105b_compile_builder = {
                'name': "mac-ppc-10.5b-compile-sandbox",
                'slavename': "tamarin-xserve2",
                'factory': sb_mac_ppc_105b_compile_factory,
                'builddir': './sandbox-mac-ppc-10_5b-compile',
    }


    ################################################
    #### builder for mac-ppc-64-compile-sandbox ####
    ################################################
    sb_mac_ppc_64_compile_factory = factory.BuildFactory()
    sb_mac_ppc_64_compile_factory.addStep(sync_clean)
    sb_mac_ppc_64_compile_factory.addStep(sync_clone_sandbox)
    sb_mac_ppc_64_compile_factory.addStep(sync_update)
    sb_mac_ppc_64_compile_factory.addStep(bb_slaveupdate(slave="mac64-ppc"))

    sb_mac_ppc_64_compile_builder = {
                'name': "mac64-ppc-compile-sandbox",
                'slavename': "asteammac5-64bit",
                'factory': sb_mac_ppc_64_compile_factory,
                'builddir': './sandbox-mac64-ppc-compile',
    }


    ###########################################
    #### builder for linux-compile-sandbox ####
    ###########################################
    sb_linux_compile_factory = factory.BuildFactory()
    sb_linux_compile_factory.addStep(sync_clean)
    sb_linux_compile_factory.addStep(sync_clone_sandbox)
    sb_linux_compile_factory.addStep(sync_update)
    sb_linux_compile_factory.addStep(bb_slaveupdate(slave="linux"))
    sb_linux_compile_factory.addStep(compile_builtin)
    sb_linux_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell", args="--enable-shell", upload="false"))
    sb_linux_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_linux_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d", args="--enable-shell --enable-debug", upload="false"))
    sb_linux_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s", args="--enable-shell --enable-debugger", upload="false"))
    sb_linux_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_linux_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-arm-neon --target=arm-linux --enable-sys-root-dir=/usr/local/arm-linux/debian5', 'avmshell_neon_arm', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'CXX': 'arm-none-linux-gnueabi-g++',
                    'CC' : 'arm-none-linux-gnueabi-gcc',
                    'LD' : 'arm-none-linux-gnueabi-ld',
                    'AR' : 'arm-none-linux-gnueabi-ar',
                },
                description='starting Release_arm-linux build...',
                descriptionDone='finished Release_arm-linux build.',
                name="Release_arm-linux",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_linux_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-debug --enable-arm-neon --target=arm-linux --enable-sys-root-dir=/usr/local/arm-linux/debian5', 'avmshell_neon_arm_d', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'CXX': 'arm-none-linux-gnueabi-g++',
                    'CC' : 'arm-none-linux-gnueabi-gcc',
                    'LD' : 'arm-none-linux-gnueabi-ld',
                    'AR' : 'arm-none-linux-gnueabi-ar',
                },
                description='starting Debug_arm-linux build...',
                descriptionDone='finished Debug_arm-linux build.',
                name="Debug_arm-linux",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_linux_compile_factory.addStep(compile_buildcheck_local)
    sb_linux_compile_factory.addStep(util_upload_asteam_local)

    sb_linux_compile_builder = {
                'name': "linux-compile-sandbox",
                'slavename': "asteamlin1",
                'factory': sb_linux_compile_factory,
                'builddir': './sandbox-linux-compile',
    }


    #############################################
    #### builder for linux64-compile-sandbox ####
    #############################################
    sb_linux_64_compile_factory = factory.BuildFactory()
    sb_linux_64_compile_factory.addStep(sync_clean)
    sb_linux_64_compile_factory.addStep(sync_clone_sandbox)
    sb_linux_64_compile_factory.addStep(sync_update)
    sb_linux_64_compile_factory.addStep(bb_slaveupdate(slave="linux64"))
    sb_linux_64_compile_factory.addStep(compile_builtin)
    sb_linux_64_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell_64", args="--enable-shell", upload="false"))
    sb_linux_64_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode_64", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_linux_64_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d_64", args="--enable-shell --enable-debug", upload="false"))
    sb_linux_64_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s_64", args="--enable-shell --enable-debugger", upload="false"))
    sb_linux_64_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd_64", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_linux_64_compile_factory.addStep(compile_testmedia)
    sb_linux_64_compile_factory.addStep(compile_buildcheck_local)
    sb_linux_64_compile_factory.addStep(util_upload_asteam_local)

    sb_linux_64_compile_builder = {
                'name': "linux64-compile-sandbox",
                'slavename': "asteamlin5",
                'factory': sb_linux_64_compile_factory,
                'builddir': './sandbox-linux64-compile',
    }


    ########################################################
    #### builder for winmobile-emulator-compile-sandbox ####
    ########################################################
    sb_winmobile_emulator_compile_factory = factory.BuildFactory()
    sb_winmobile_emulator_compile_factory.addStep(sync_clean)
    sb_winmobile_emulator_compile_factory.addStep(sync_clone_sandbox)
    sb_winmobile_emulator_compile_factory.addStep(sync_update)
    sb_winmobile_emulator_compile_factory.addStep(bb_slaveupdate(slave="winmobile-arm"))
    sb_winmobile_emulator_compile_factory.addStep(compile_builtin)
    sb_winmobile_emulator_compile_factory.addStep(compile_generic(name="ReleaseARM", shellname="avmshell_arm", args="--enable-shell --target=arm-windows", upload="false"))
    sb_winmobile_emulator_compile_factory.addStep(compile_generic(name="Release-wordcode-ARM", shellname="avmshell_wordcode_arm", args="--enable-shell --enable-wordcode-interp --target=arm-windows", upload="false"))
    sb_winmobile_emulator_compile_factory.addStep(compile_generic(name="Release-fpu-ARM", shellname="avmshell_fpu_arm", args="--enable-shell --enable-arm-fpu --target=arm-windows", upload="false"))
    sb_winmobile_emulator_compile_factory.addStep(compile_generic(name="DebugARM", shellname="avmshell_arm_d", args="--enable-shell --enable-debug --target=arm-windows", upload="false"))
    sb_winmobile_emulator_compile_factory.addStep(compile_generic(name="Debug-fpu-ARM", shellname="avmshell_fpu_arm_d", args="--enable-shell --enable-debug --enable-arm-fpu --target=arm-windows", upload="false"))
    sb_winmobile_emulator_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --target=x86_64-win', 'avmshell_64', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'compile64':'true',
                },
                description='starting Release64 build...',
                descriptionDone='finished Release64 build.',
                name="Release64",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_winmobile_emulator_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-wordcode-interp --target=x86_64-win', 'avmshell_wordcode_64', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'compile64':'true'
                },
                description='starting Release-wordcode64 build...',
                descriptionDone='finished Release-wordcode64 build.',
                name="Release-wordcode64",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_winmobile_emulator_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-debug --target=x86_64-win', 'avmshell_d_64', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'compile64':'true'
                },
                description='starting Debug64 build...',
                descriptionDone='finished Debug64 build.',
                name="Debug64",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_winmobile_emulator_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-debugger --target=x86_64-win', 'avmshell_s_64', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'compile64':'true'
                },
                description='starting ReleaseDebugger64 build...',
                descriptionDone='finished ReleaseDebugger64 build.',
                name="ReleaseDebugger64",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_winmobile_emulator_compile_factory.addStep(BuildShellCommand(
                command=['../all/compile-generic.sh', WithProperties('%s','revision'), '--enable-shell --enable-debug --enable-debugger --target=x86_64-win', 'avmshell_sd_64', 'false'],
                env={
                    'branch': WithProperties('%s','branch'),
                    'silent':WithProperties('%s','silent'),
                    'compile64':'true'
                },
                description='starting DebugDebugger64 build...',
                descriptionDone='finished DebugDebugger64 build.',
                name="DebugDebugger64",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_winmobile_emulator_compile_factory.addStep(compile_buildcheck_local)
    sb_winmobile_emulator_compile_factory.addStep(util_upload_asteam_local)

    sb_winmobile_emulator_compile_builder = {
                'name': "winmobile-emulator-compile-sandbox",
                'slavename': "asteamwin19",
                'factory': sb_winmobile_emulator_compile_factory,
                'builddir': './sandbox-winmobile-emulator-compile',
    }


    ###################################################
    #### builder for solaris-sparc-compile-sandbox ####
    ###################################################
    sb_solaris_sparc_compile_factory = factory.BuildFactory()
    sb_solaris_sparc_compile_factory.addStep(sync_clean)
    sb_solaris_sparc_compile_factory.addStep(sync_clone_sandbox)
    sb_solaris_sparc_compile_factory.addStep(sync_update)
    sb_solaris_sparc_compile_factory.addStep(bb_slaveupdate(slave="solaris-sparc"))
    sb_solaris_sparc_compile_factory.addStep(compile_builtin)
    sb_solaris_sparc_compile_factory.addStep(compile_generic(name="Release", shellname="avmshell", args="--enable-shell", upload="false"))
    sb_solaris_sparc_compile_factory.addStep(compile_generic(name="Release-wordcode", shellname="avmshell_wordcode", args="--enable-shell --enable-wordcode-interp", upload="false"))
    sb_solaris_sparc_compile_factory.addStep(compile_generic(name="Debug", shellname="avmshell_d", args="--enable-shell --enable-debug", upload="false"))
    sb_solaris_sparc_compile_factory.addStep(compile_generic(name="ReleaseDebugger", shellname="avmshell_s", args="--enable-shell --enable-debugger", upload="false"))
    sb_solaris_sparc_compile_factory.addStep(compile_generic(name="DebugDebugger", shellname="avmshell_sd", args="--enable-shell --enable-debug --enable-debugger", upload="false"))
    sb_solaris_sparc_compile_factory.addStep(compile_buildcheck_local)
    sb_solaris_sparc_compile_factory.addStep(util_upload_asteam)

    sb_solaris_sparc_compile_builder = {
                'name': "solaris-sparc-compile-sandbox",
                'slavename': "asteamsol4",
                'factory': sb_solaris_sparc_compile_factory,
                'builddir': './sandbox-solaris-sparc-compile',
    }
    
    
    ############################################
    #### builder for solaris-sparc2-compile ####
    ############################################
    sb_solaris_sparc2_compile_factory = factory.BuildFactory()
    sb_solaris_sparc2_compile_factory.addStep(sync_clean)
    sb_solaris_sparc2_compile_factory.addStep(sync_clone_sandbox)
    sb_solaris_sparc2_compile_factory.addStep(sync_update)
    sb_solaris_sparc2_compile_factory.addStep(bb_slaveupdate(slave="solaris-sparc"))
    
    sb_solaris_sparc2_compile_builder = {
                'name': "solaris-sparc2-compile-sandbox",
                'slavename': "asteamsol3",
                'factory': sb_solaris_sparc2_compile_factory,
                'builddir': './sandbox-solaris-sparc2-compile',
    }

    ###########################################
    #### builder for android on mac        ####
    ###########################################

    sb_android_compile_factory = factory.BuildFactory();
    sb_android_compile_factory.addStep(sync_clean)
    sb_android_compile_factory.addStep(sync_clone_sandbox)
    sb_android_compile_factory.addStep(sync_update)
    sb_android_compile_factory.addStep(bb_slaveupdate(slave="mac-intel-server"))
    sb_android_compile_factory.addStep(compile_builtin)
    sb_android_compile_factory.addStep(BuildShellCommand(
                command=['./build-debug-shell-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='building debug shell...',
                descriptionDone='finished building debug shell.',
                name="Build_Debug",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )
    sb_android_compile_factory.addStep(BuildShellCommand(
                command=['./build-release-shell-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='building release shell...',
                descriptionDone='finished building release shell.',
                name="Build_Release",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )
    sb_android_compile_factory.addStep(BuildShellCommand(
                command=['./build-check-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='running build check...',
                descriptionDone='finished build check.',
                name="Build_Check",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )
    sb_android_compile_factory.addStep(BuildShellCommand(
                command=['./upload-asteam-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='running upload to asteam...',
                descriptionDone='finished upload to asteam.',
                name="Upload_ASTEAM",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )

    sb_android_compile_builder = {
                'name': "android-compile-sandbox",
                'slavename': "asteammac12",
                'factory': sb_android_compile_factory,
                'builddir': './sandbox-android-compile',
    }
    
    ###############################
    #### builder for linux-arm ####
    ###############################
    sb_linux_arm_compile_factory = factory.BuildFactory()
    sb_linux_arm_compile_factory.addStep(sync_clean)
    sb_linux_arm_compile_factory.addStep(sync_clone_sandbox)
    sb_linux_arm_compile_factory.addStep(sync_update)
    sb_linux_arm_compile_factory.addStep(bb_slaveupdate(slave="linux-arm"))

    sb_linux_arm_compile_builder = {
                'name': "linux-arm-compile-sandbox",
                'slavename': "asteambeagleboard2",
                'factory': sb_linux_arm_compile_factory,
                'builddir': './sandbox-linux-arm-compile',
    }
    
    
    ################################
    #### builder for linux-arm2 ####
    ################################
    sb_linux_arm2_compile_factory = factory.BuildFactory()
    sb_linux_arm2_compile_factory.addStep(sync_clean)
    sb_linux_arm2_compile_factory.addStep(sync_clone_sandbox)
    sb_linux_arm2_compile_factory.addStep(sync_update)
    sb_linux_arm2_compile_factory.addStep(bb_slaveupdate(slave="linux-arm"))

    sb_linux_arm2_compile_builder = {
                'name': "linux-arm2-compile-sandbox",
                'slavename': "asteambeagle4",
                'factory': sb_linux_arm2_compile_factory,
                'builddir': './sandbox-linux-arm2-compile',
    }

    ################################################################################
    ################################################################################
    ####                                                                        ####
    ####                     SANDBOX SMOKE BUILDERS                             ####
    ####                                                                        ####
    ################################################################################
    ################################################################################




    ###########################################
    #### builder for windows-smoke-sandbox ####
    ###########################################
    sb_windows_smoke_factory = factory.BuildFactory()
    sb_windows_smoke_factory.addStep(download_testmedia)
    sb_windows_smoke_factory.addStep(test_smoke)
    sb_windows_smoke_factory.addStep(util_process_clean)

    sb_windows_smoke_builder = {
                'name': "windows-smoke-sandbox",
                'slavename': "asteamwin2",
                'factory': sb_windows_smoke_factory,
                'builddir': './sandbox-windows-smoke',
    }


    #############################################
    #### builder for windows64-smoke-sandbox ####
    #############################################
    sb_windows_64_smoke_factory = factory.BuildFactory()
    sb_windows_64_smoke_factory.addStep(download_testmedia)
    sb_windows_64_smoke_factory.addStep(test_smoke)
    sb_windows_64_smoke_factory.addStep(util_process_clean)

    sb_windows_64_smoke_builder = {
                'name': "windows64-smoke-sandbox",
                'slavename': "asteamwin3",
                'factory': sb_windows_64_smoke_factory,
                'builddir': './sandbox-windows64-smoke',
    }


    ##################################################
    #### builder for mac-intel-10_4-smoke-sandbox ####
    ##################################################
    sb_mac_intel_104_smoke_factory = factory.BuildFactory()
    sb_mac_intel_104_smoke_factory.addStep(download_testmedia)
    sb_mac_intel_104_smoke_factory.addStep(test_smoke)
    sb_mac_intel_104_smoke_factory.addStep(util_process_clean)

    sb_mac_intel_104_smoke_builder = {
                'name': "mac-intel-10.4-smoke-sandbox",
                'slavename': "asteammac4",
                'factory': sb_mac_intel_104_smoke_factory,
                'builddir': './sandbox-mac-intel-10_4-smoke',
    }


    ##################################################
    #### builder for mac-intel-10_5-smoke-sandbox ####
    ##################################################
    sb_mac_intel_105_smoke_factory = factory.BuildFactory()
    sb_mac_intel_105_smoke_factory.addStep(download_testmedia)
    sb_mac_intel_105_smoke_factory.addStep(test_smoke)
    sb_mac_intel_105_smoke_factory.addStep(util_process_clean)

    sb_mac_intel_105_smoke_builder = {
                'name': "mac-intel-10.5-smoke-sandbox",
                'slavename': "asteammac1",
                'factory': sb_mac_intel_105_smoke_factory,
                'builddir': './sandbox-mac-intel-10_5-smoke',
    }


    ###############################################
    #### builder for mac64-intel-smoke-sandbox ####
    ###############################################
    sb_mac_intel_64_smoke_factory = factory.BuildFactory()
    sb_mac_intel_64_smoke_factory.addStep(download_testmedia)
    sb_mac_intel_64_smoke_factory.addStep(test_smoke)
    sb_mac_intel_64_smoke_factory.addStep(util_process_clean)

    sb_mac_intel_64_smoke_builder = {
                'name': "mac64-intel-smoke-sandbox",
                'slavename': "asteammac1-64bit",
                'factory': sb_mac_intel_64_smoke_factory,
                'builddir': './sandbox-mac64-intel-smoke',
    }

    #################################################
    #### builder for mac-ppc-10_4a-smoke-sandbox ####
    #################################################
    sb_mac_ppc_104a_smoke_factory = factory.BuildFactory()
    sb_mac_ppc_104a_smoke_factory.addStep(download_testmedia)
    sb_mac_ppc_104a_smoke_factory.addStep(test_smoke)
    sb_mac_ppc_104a_smoke_factory.addStep(util_process_clean)

    sb_mac_ppc_104a_smoke_builder = {
                'name': "mac-ppc-10.4a-smoke-sandbox",
                'slavename': "asteammac6",
                'factory': sb_mac_ppc_104a_smoke_factory,
                'builddir': './sandbox-mac-ppc-10_4a-smoke',
    }


    #################################################
    #### builder for mac-ppc-10_4b-smoke-sandbox ####
    #################################################
    sb_mac_ppc_104b_smoke_factory = factory.BuildFactory()
    sb_mac_ppc_104b_smoke_factory.addStep(download_testmedia)
    sb_mac_ppc_104b_smoke_factory.addStep(test_smoke)
    sb_mac_ppc_104b_smoke_factory.addStep(util_process_clean)

    sb_mac_ppc_104b_smoke_builder = {
                'name': "mac-ppc-10.4b-smoke-sandbox",
                'slavename': "asteammac9",
                'factory': sb_mac_ppc_104b_smoke_factory,
                'builddir': './sandbox-mac-ppc-10_4b-smoke',
    }


    #################################################
    #### builder for mac-ppc-10_5a-smoke-sandbox ####
    #################################################
    sb_mac_ppc_105a_smoke_factory = factory.BuildFactory()
    sb_mac_ppc_105a_smoke_factory.addStep(download_testmedia)
    sb_mac_ppc_105a_smoke_factory.addStep(test_smoke)
    sb_mac_ppc_105a_smoke_factory.addStep(util_process_clean)

    sb_mac_ppc_105a_smoke_builder = {
                'name': "mac-ppc-10.5a-smoke-sandbox",
                'slavename': "tamarin-xserve",
                'factory': sb_mac_ppc_105a_smoke_factory,
                'builddir': './sandbox-mac-ppc-10_5a-smoke',
    }

    #################################################
    #### builder for mac-ppc-10_5b-smoke-sandbox ####
    #################################################
    sb_mac_ppc_105b_smoke_factory = factory.BuildFactory()
    sb_mac_ppc_105b_smoke_factory.addStep(download_testmedia)
    sb_mac_ppc_105b_smoke_factory.addStep(test_smoke)
    sb_mac_ppc_105b_smoke_factory.addStep(util_process_clean)

    sb_mac_ppc_105b_smoke_builder = {
                'name': "mac-ppc-10.5b-smoke-sandbox",
                'slavename': "tamarin-xserve2",
                'factory': sb_mac_ppc_105b_smoke_factory,
                'builddir': './sandbox-mac-ppc-10_5b-smoke',
    }


    #############################################
    #### builder for mac64-ppc-smoke-sandbox ####
    #############################################
    sb_mac_ppc_64_smoke_factory = factory.BuildFactory()
    sb_mac_ppc_64_smoke_factory.addStep(download_testmedia)
    sb_mac_ppc_64_smoke_factory.addStep(test_smoke)
    sb_mac_ppc_64_smoke_factory.addStep(util_process_clean)

    sb_mac_ppc_64_smoke_builder = {
                'name': "mac64-ppc-smoke-sandbox",
                'slavename': "asteammac5-64bit",
                'factory': sb_mac_ppc_64_smoke_factory,
                'builddir': './sandbox-mac64-ppc-smoke',
    }


    #########################################
    #### builder for linux-smoke-sandbox ####
    #########################################
    sb_linux_smoke_factory = factory.BuildFactory()
    sb_linux_smoke_factory.addStep(download_testmedia)
    sb_linux_smoke_factory.addStep(test_smoke)
    sb_linux_smoke_factory.addStep(util_process_clean)

    sb_linux_smoke_builder = {
                'name': "linux-smoke-sandbox",
                'slavename': "asteamlin1",
                'factory': sb_linux_smoke_factory,
                'builddir': './sandbox-linux-smoke',
    }


    ###########################################
    #### builder for linux64-smoke-sandbox ####
    ###########################################
    sb_linux_64_smoke_factory = factory.BuildFactory()
    sb_linux_64_smoke_factory.addStep(download_testmedia)
    sb_linux_64_smoke_factory.addStep(test_smoke)
    sb_linux_64_smoke_factory.addStep(util_process_clean)

    sb_linux_64_smoke_builder = {
                'name': "linux64-smoke-sandbox",
                'slavename': "asteamlin5",
                'factory': sb_linux_64_smoke_factory,
                'builddir': './sandbox-linux64-smoke',
    }


    ######################################################
    #### builder for winmobile-emulator-smoke-sandbox ####
    ######################################################
    sb_winmobile_emulator_smoke_factory = factory.BuildFactory()
    sb_winmobile_emulator_smoke_factory.addStep(download_testmedia)
    sb_winmobile_emulator_smoke_factory.addStep(test_emulator_smoke_mobile)
    sb_winmobile_emulator_smoke_factory.addStep(util_process_clean)

    sb_winmobile_emulator_smoke_builder = {
                'name': "winmobile-emulator-smoke-sandbox",
                'slavename': "asteamwin19",
                'factory': sb_winmobile_emulator_smoke_factory,
                'builddir': './sandbox-winmobile-emulator-smoke',
    }

    #################################################
    #### builder for solaris-sparc-smoke-sandbox ####
    #################################################
    sb_solaris_sparc_smoke_factory = factory.BuildFactory()
    sb_solaris_sparc_smoke_factory.addStep(download_testmedia)
    sb_solaris_sparc_smoke_factory.addStep(test_smoke)
    sb_solaris_sparc_smoke_factory.addStep(util_process_clean)

    sb_solaris_sparc_smoke_builder = {
                'name': "solaris-sparc-smoke-sandbox",
                'slavename': "asteamsol4",
                'factory': sb_solaris_sparc_smoke_factory,
                'builddir': './sandbox-solaris-sparc-smoke',
    }
    
    ##########################################
    #### builder for solaris-sparc2-smoke ####
    ##########################################
    sb_solaris_sparc2_smoke_factory = factory.BuildFactory()
    sb_solaris_sparc2_smoke_factory.addStep(download_testmedia)
    sb_solaris_sparc2_smoke_factory.addStep(test_smoke)
    sb_solaris_sparc2_smoke_factory.addStep(util_process_clean)

    sb_solaris_sparc2_smoke_builder = {
                'name': "solaris-sparc2-smoke-sandbox",
                'slavename': "asteamsol3",
                'factory': sb_solaris_sparc2_smoke_factory,
                'builddir': './sandbox-solaris-sparc2-smoke',
    }
    
    #########################################
    #### builder for android-smoke       ####
    #########################################
    sb_android_smoke_factory = factory.BuildFactory()
    sb_android_smoke_factory.addStep(download_testmedia)
    sb_android_smoke_factory.addStep(TestSuiteShellCommand(
                command=['./run-smoketests-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='starting to run smoke tests...',
                descriptionDone='finished smoke tests.',
                name="SmokeTest",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )
    sb_android_smoke_factory.addStep(util_process_clean)

    sb_android_smoke_builder = {
                'name': "android-smoke-sandbox",
                'slavename': "asteammac12",
                'factory': sb_android_smoke_factory,
                'builddir': './sanbox-android-smoke',
    }
    
    ###########################################
    #### builder for linxu-arm-smoke       ####
    ###########################################
    sb_linux_arm_smoke_factory = factory.BuildFactory()
    sb_linux_arm_smoke_factory.addStep(download_testmedia)
    sb_linux_arm_smoke_factory.addStep(TestSuiteShellCommand(
                command=['../all/run-smoketests.sh', WithProperties('%s','revision'), './runsmokes-arm.txt'],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='starting to run smoke tests...',
                descriptionDone='finished smoke tests.',
                name="SmokeTest",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_linux_arm_smoke_factory.addStep(util_process_clean)

    sb_linux_arm_smoke_builder = {
                'name': "linux-arm-smoke-sandbox",
                'slavename': "asteambeagleboard2",
                'factory': sb_linux_arm_smoke_factory,
                'builddir': './sandbox-linux-arm-smoke',
    }
    
    
    ###########################################
    #### builder for linxu-arm2-smoke      ####
    ###########################################
    sb_linux_arm2_smoke_factory = factory.BuildFactory()
    sb_linux_arm2_smoke_factory.addStep(download_testmedia)
    sb_linux_arm2_smoke_factory.addStep(TestSuiteShellCommand(
                command=['../all/run-smoketests.sh', WithProperties('%s','revision'), './runsmokes-arm.txt'],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='starting to run smoke tests...',
                descriptionDone='finished smoke tests.',
                name="SmokeTest",
                workdir="../repo/build/buildbot/slaves/scripts")
    )
    sb_linux_arm2_smoke_factory.addStep(util_process_clean)

    sb_linux_arm2_smoke_builder = {
                'name': "linux-arm2-smoke-sandbox",
                'slavename': "asteambeagle4",
                'factory': sb_linux_arm2_smoke_factory,
                'builddir': './sandbox-linux-arm2-smoke',
    }

    ################################################################################
    ################################################################################
    ####                                                                        ####
    ####                    SANDBOX TEST BUILDERS                               ####
    ####                                                                        ####
    ################################################################################
    ################################################################################




    ##########################################
    #### builder for windows-test-sandbox ####
    ##########################################
    sb_windows_test_factory = factory.BuildFactory()
    sb_windows_test_factory.addStep(test_commandline)
    sb_windows_test_factory.addStep(test_selftest)
    sb_windows_test_factory.addStep(test_generic(name="Release", shellname="avmshell", vmargs="", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell", vmargs="-Dinterp", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode", vmargs="-Dinterp", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell", vmargs="-Ojit", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s", vmargs="", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d", vmargs="", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd", vmargs="", config="", scriptargs=""))
    sb_windows_test_factory.addStep(test_differential)
    sb_windows_test_factory.addStep(util_process_clean)
    sb_windows_test_factory.addStep(util_clean_buildsdir)

    sb_windows_test_builder = {
                'name': "windows-test-sandbox",
                'slavename': "asteamwin2",
                'factory': sb_windows_test_factory,
                'builddir': './sandbox-windows-test',
    }

    ############################################
    #### builder for windows64-test-sandbox ####
    ############################################
    sb_windows_64_test_factory = factory.BuildFactory()
    sb_windows_64_test_factory.addStep(test_commandline)
    sb_windows_64_test_factory.addStep(test_selftest)
    sb_windows_64_test_factory.addStep(test_generic(name="Release", shellname="avmshell_64", vmargs="", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_64", vmargs="-Ojit", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_64", vmargs="", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_64", vmargs="", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_64", vmargs="", config="", scriptargs=""))
    sb_windows_64_test_factory.addStep(util_process_clean)
    sb_windows_64_test_factory.addStep(util_clean_buildsdir)

    sb_windows_64_test_builder = {
                'name': "windows64-test-sandbox",
                'slavename': "asteamwin3",
                'factory': sb_windows_64_test_factory,
                'builddir': './sandbox-windows64-test',
    }


    #################################################
    #### builder for mac-intel-10_4-test-sandbox ####
    #################################################
    sb_mac_intel_104_test_factory = factory.BuildFactory()
    sb_mac_intel_104_test_factory.addStep(test_commandline)
    sb_mac_intel_104_test_factory.addStep(test_selftest)
    sb_mac_intel_104_test_factory.addStep(test_generic(name="Release", shellname="avmshell_104", vmargs="", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_104", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_104", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_104", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_104", vmargs="", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_104", vmargs="", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_104", vmargs="", config="", scriptargs=""))
    sb_mac_intel_104_test_factory.addStep(test_differential)
    sb_mac_intel_104_test_factory.addStep(util_process_clean)
    sb_mac_intel_104_test_factory.addStep(util_clean_buildsdir)

    sb_mac_intel_104_test_builder = {
                'name': "mac-intel-10.4-test-sandbox",
                'slavename': "asteammac4",
                'factory': sb_mac_intel_104_test_factory,
                'builddir': './sandbox-mac-intel-10_4-test',
    }



    #################################################
    #### builder for mac-intel-10_5-test-sandbox ####
    #################################################
    sb_mac_intel_105_test_factory = factory.BuildFactory()
    sb_mac_intel_105_test_factory.addStep(test_commandline)
    sb_mac_intel_105_test_factory.addStep(test_selftest)
    sb_mac_intel_105_test_factory.addStep(test_generic(name="Release", shellname="avmshell", vmargs="", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s", vmargs="", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d", vmargs="", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd", vmargs="", config="", scriptargs=""))
    sb_mac_intel_105_test_factory.addStep(test_differential)
    sb_mac_intel_105_test_factory.addStep(util_process_clean)
    sb_mac_intel_105_test_factory.addStep(util_clean_buildsdir)

    sb_mac_intel_105_test_builder = {
                'name': "mac-intel-10.5-test-sandbox",
                'slavename': "asteammac1",
                'factory': sb_mac_intel_105_test_factory,
                'builddir': './sandbox-mac-intel-10_5-test',
    }

    ##############################################
    #### builder for mac64-intel-test-sandbox ####
    ##############################################
    sb_mac_intel_64_test_factory = factory.BuildFactory()
    sb_mac_intel_64_test_factory.addStep(test_commandline)
    sb_mac_intel_64_test_factory.addStep(test_selftest)
    sb_mac_intel_64_test_factory.addStep(test_generic(name="Release", shellname="avmshell_64", vmargs="", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_64", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_64", vmargs="", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_64", vmargs="", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_64", vmargs="", config="", scriptargs=""))
    sb_mac_intel_64_test_factory.addStep(util_process_clean)
    sb_mac_intel_64_test_factory.addStep(util_clean_buildsdir)

    sb_mac_intel_64_test_builder = {
                'name': "mac64-intel-test-sandbox",
                'slavename': "asteammac1-64bit",
                'factory': sb_mac_intel_64_test_factory,
                'builddir': './sandbox-mac64-intel-test',
    }


    ################################################
    #### builder for mac-ppc-10_4a-test-sandbox ####
    ################################################
    sb_mac_ppc_104a_test_factory = factory.BuildFactory()
    sb_mac_ppc_104a_test_factory.addStep(test_commandline)
    sb_mac_ppc_104a_test_factory.addStep(test_selftest)
    sb_mac_ppc_104a_test_factory.addStep(test_generic(name="Release", shellname="avmshell_104_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_104a_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_104_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_104a_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_104_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_104a_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_104_ppc", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_ppc_104a_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_104_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_104a_test_factory.addStep(util_process_clean)
    sb_mac_ppc_104a_test_factory.addStep(util_clean_buildsdir)

    sb_mac_ppc_104a_test_builder = {
                'name': "mac-ppc-10.4a-test-sandbox",
                'slavename': "asteammac6",
                'factory': sb_mac_ppc_104a_test_factory,
                'builddir': './sandbox-mac-ppc-10_4a-test',
    }


    ################################################
    #### builder for mac-ppc-10_4b-test-sandbox ####
    ################################################
    sb_mac_ppc_104b_test_factory = factory.BuildFactory()
    sb_mac_ppc_104b_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_104_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_104b_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_104_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_104b_test_factory.addStep(util_process_clean)
    sb_mac_ppc_104b_test_factory.addStep(util_clean_buildsdir)

    sb_mac_ppc_104b_test_builder = {
                'name': "mac-ppc-10.4b-test-sandbox",
                'slavename': "asteammac9",
                'factory': sb_mac_ppc_104b_test_factory,
                'builddir': './sandbox-mac-ppc-10_4b-test',
    }


    ################################################
    #### builder for mac-ppc-10_5a-test-sandbox ####
    ################################################
    sb_mac_ppc_105a_test_factory = factory.BuildFactory()
    sb_mac_ppc_105a_test_factory.addStep(test_commandline)
    sb_mac_ppc_105a_test_factory.addStep(test_selftest)
    sb_mac_ppc_105a_test_factory.addStep(test_generic(name="Release", shellname="avmshell_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_105a_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_105a_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_105a_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_ppc", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_ppc_105a_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_105a_test_factory.addStep(util_process_clean)
    sb_mac_ppc_105a_test_factory.addStep(util_clean_buildsdir)

    sb_mac_ppc_105a_test_builder = {
                'name': "mac-ppc-10.5a-test-sandbox",
                'slavename': "tamarin-xserve",
                'factory': sb_mac_ppc_105a_test_factory,
                'builddir': './sandbox-mac-ppc-10_5a-test',
    }

    ################################################
    #### builder for mac-ppc-10_5b-test-sandbox ####
    ################################################
    sb_mac_ppc_105b_test_factory = factory.BuildFactory()
    sb_mac_ppc_105b_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_105b_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_105b_test_factory.addStep(util_process_clean)
    sb_mac_ppc_105b_test_factory.addStep(util_clean_buildsdir)

    sb_mac_ppc_105b_test_builder = {
                'name': "mac-ppc-10.5b-test-sandbox",
                'slavename': "tamarin-xserve2",
                'factory': sb_mac_ppc_105b_test_factory,
                'builddir': './sandbox-mac-ppc-10_5b-test',
    }


    ############################################
    #### builder for mac64-ppc-test-sandbox ####
    ############################################
    sb_mac_ppc_64_test_factory = factory.BuildFactory()
    sb_mac_ppc_64_test_factory.addStep(test_commandline)
    sb_mac_ppc_64_test_factory.addStep(test_selftest)
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="Release", shellname="avmshell_64_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_64_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_64_ppc", vmargs="-Dinterp", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_64_ppc", vmargs="-Ojit", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_64_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_64_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_64_ppc", vmargs="", config="", scriptargs=""))
    sb_mac_ppc_64_test_factory.addStep(util_process_clean)
    sb_mac_ppc_64_test_factory.addStep(util_clean_buildsdir)

    sb_mac_ppc_64_test_builder = {
                'name': "mac64-ppc-test-sandbox",
                'slavename': "asteammac5-64bit",
                'factory': sb_mac_ppc_64_test_factory,
                'builddir': './sandbox-mac64-ppc-test',
    }


    ########################################
    #### builder for linux-test-sandbox ####
    ########################################
    sb_linux_test_factory = factory.BuildFactory()
    sb_linux_test_factory.addStep(test_commandline)
    sb_linux_test_factory.addStep(test_selftest)
    sb_linux_test_factory.addStep(test_generic(name="Release", shellname="avmshell", vmargs="", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell", vmargs="-Dinterp", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode", vmargs="-Dinterp", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell", vmargs="-Ojit", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s", vmargs="", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d", vmargs="", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd", vmargs="", config="", scriptargs=""))
    sb_linux_test_factory.addStep(test_differential)
    sb_linux_test_factory.addStep(util_process_clean)
    sb_linux_test_factory.addStep(util_clean_buildsdir)

    sb_linux_test_builder = {
                'name': "linux-test-sandbox",
                'slavename': "asteamlin1",
                'factory': sb_linux_test_factory,
                'builddir': './sandbox-linux-test',
    }


    ##########################################
    #### builder for linux64-test-sandbox ####
    ##########################################
    sb_linux_64_test_factory = factory.BuildFactory()
    sb_linux_64_test_factory.addStep(test_commandline)
    sb_linux_64_test_factory.addStep(test_selftest)
    sb_linux_64_test_factory.addStep(test_generic(name="Release", shellname="avmshell_64", vmargs="", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_64", vmargs="-Dinterp", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell_64", vmargs="-Ojit", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s_64", vmargs="", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d_64", vmargs="", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd_64", vmargs="", config="", scriptargs=""))
    sb_linux_64_test_factory.addStep(util_process_clean)
    sb_linux_64_test_factory.addStep(util_clean_buildsdir)

    sb_linux_64_test_builder = {
                'name': "linux64-test-sandbox",
                'slavename': "asteamlin5",
                'factory': sb_linux_64_test_factory,
                'builddir': './sandbox-linux64-test',
    }

    #####################################################
    #### builder for winmobile-emulator-test-sandbox ####
    #####################################################
    sb_winmobile_emulator_test_factory = factory.BuildFactory()
    sb_winmobile_emulator_test_factory.addStep(test_emulator_generic(name="Release", shellname="avmshell_arm", vmargs="", config="", scriptargs=""))
    sb_winmobile_emulator_test_factory.addStep(test_emulator_generic(name="Release-interp", shellname="avmshell_arm", vmargs="-Dinterp", config="", scriptargs=""))
    sb_winmobile_emulator_test_factory.addStep(test_emulator_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode_arm", vmargs="-Dinterp", config="", scriptargs=""))
    sb_winmobile_emulator_test_factory.addStep(test_emulator_generic(name="Release-jit", shellname="avmshell_arm", vmargs="-Ojit", config="", scriptargs=""))
    sb_winmobile_emulator_test_factory.addStep(util_process_clean)
    sb_winmobile_emulator_test_factory.addStep(util_clean_buildsdir)

    sb_winmobile_emulator_test_builder = {
                'name': "winmobile-emulator-test-sandbox",
                'slavename': "asteamwin19",
                'factory': sb_winmobile_emulator_test_factory,
                'builddir': './sandbox-winmobile-emulator-test',
    }


    ########################################
    #### builder for solaris-sparc-test ####
    ########################################
    sb_solaris_sparc_test_factory = factory.BuildFactory()
    sb_solaris_sparc_test_factory.addStep(test_commandline)
    sb_solaris_sparc_test_factory.addStep(test_selftest)
    sb_solaris_sparc_test_factory.addStep(test_generic(name="Debug", shellname="avmshell_d", vmargs="", config="", scriptargs=""))
    sb_solaris_sparc_test_factory.addStep(test_generic(name="DebugDebugger", shellname="avmshell_sd", vmargs="", config="", scriptargs=""))
    sb_solaris_sparc_test_factory.addStep(util_process_clean)
    sb_solaris_sparc_test_factory.addStep(util_clean_buildsdir)

    sb_solaris_sparc_test_builder = {
                'name': "solaris-sparc-test-sandbox",
                'slavename': "asteamsol4",
                'factory': sb_solaris_sparc_test_factory,
                'builddir': './sandbox-solaris-sparc-test',
    }
    
    
    #########################################
    #### builder for solaris-sparc2-test ####
    #########################################
    sb_solaris_sparc2_test_factory = factory.BuildFactory()
    sb_solaris_sparc2_test_factory.addStep(test_generic(name="Release", shellname="avmshell", vmargs="", config="", scriptargs=""))
    sb_solaris_sparc2_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell", vmargs="-Dinterp", config="", scriptargs=""))
    sb_solaris_sparc2_test_factory.addStep(test_generic(name="Release-wordcode-interp", shellname="avmshell_wordcode", vmargs="-Dinterp", config="", scriptargs=""))
    sb_solaris_sparc2_test_factory.addStep(test_generic(name="Release-jit", shellname="avmshell", vmargs="-Ojit", config="", scriptargs=""))
    sb_solaris_sparc2_test_factory.addStep(test_generic(name="ReleaseDebugger", shellname="avmshell_s", vmargs="", config="", scriptargs=""))
    sb_solaris_sparc2_test_factory.addStep(util_process_clean)
    sb_solaris_sparc2_test_factory.addStep(util_clean_buildsdir)

    sb_solaris_sparc2_test_builder = {
                'name': "solaris-sparc2-test-sandbox",
                'slavename': "asteamsol3",
                'factory': sb_solaris_sparc2_test_factory,
                'builddir': './sandbox-solaris-sparc2-test',
    }
    
    ########################################
    #### builder for android-test       ####
    ########################################
    sb_android_test_factory = factory.BuildFactory()
    sb_android_test_factory.addStep(TestSuiteShellCommand(
                command=['./run-acceptancetests-android.sh', WithProperties('%s','revision')],
                env={'branch': WithProperties('%s','branch'), 'silent':WithProperties('%s','silent')},
                description='starting to run acceptance tests...',
                descriptionDone='finished acceptance tests.',
                name="Testsuite_Release",
                workdir="../repo/build/buildbot/slaves/scripts",
                timeout=3600)
    )
    sb_android_test_factory.addStep(util_process_clean)
    sb_android_test_factory.addStep(util_clean_buildsdir)

    sb_android_test_builder = {
                'name': "android-test-sandbox",
                'slavename': "asteammac12",
                'factory': sb_android_test_factory,
                'builddir': './sandbox-android-test',
    }
    
    ##########################################
    #### builder for linux-arm-test       ####
    ##########################################
    sb_linux_arm_test_factory = factory.BuildFactory()
    sb_linux_arm_test_factory.addStep(test_selftest)
    sb_linux_arm_test_factory.addStep(test_generic(name="Release-softfloat", shellname="avmshell_neon_arm", vmargs="", config="", scriptargs=""))
    sb_linux_arm_test_factory.addStep(test_generic(name="Release-vfp", shellname="avmshell_neon_arm", vmargs="-Darm_arch 7 -Darm_vfp", config="", scriptargs=""))
    sb_linux_arm_test_factory.addStep(test_generic(name="Release-jit-vfp", shellname="avmshell_neon_arm", vmargs="-Darm_arch 7 -Darm_vfp -Ojit", config="", scriptargs=""))
    sb_linux_arm_test_factory.addStep(util_process_clean)
    sb_linux_arm_test_factory.addStep(util_clean_buildsdir)

    sb_linux_arm_test_builder = {
                'name': "linux-arm-test-sandbox",
                'slavename': "asteambeagleboard2",
                'factory': sb_linux_arm_test_factory,
                'builddir': './sandbox-linux-arm-test',
    }
    
    
    ##########################################
    #### builder for linux-arm2-test      ####
    ##########################################
    sb_linux_arm2_test_factory = factory.BuildFactory()
    sb_linux_arm2_test_factory.addStep(test_generic(name="Release-interp", shellname="avmshell_neon_arm", vmargs="-Dinterp", config="", scriptargs=""))
    sb_linux_arm2_test_factory.addStep(test_generic(name="Debug-vfp", shellname="avmshell_neon_arm_d", vmargs="-Darm_arch 7 -Darm_vfp", config="", scriptargs=""))
    sb_linux_arm2_test_factory.addStep(util_process_clean)
    sb_linux_arm2_test_factory.addStep(util_clean_buildsdir)

    sb_linux_arm2_test_builder = {
                'name': "linux-arm2-test-sandbox",
                'slavename': "asteambeagle4",
                'factory': sb_linux_arm2_test_factory,
                'builddir': './sandbox-linux-arm2-test',
    }
    
    
    
    
    builders = [
                sb_windows_compile_builder,
                sb_windows_64_compile_builder,
                sb_mac_intel_104_compile_builder,
                sb_mac_intel_105_compile_builder,
                sb_mac_intel_64_compile_builder,
                sb_mac_ppc_104a_compile_builder,
                sb_mac_ppc_104b_compile_builder,
                sb_mac_ppc_105a_compile_builder,
                sb_mac_ppc_105b_compile_builder,
                sb_mac_ppc_64_compile_builder,
                sb_linux_compile_builder,
                sb_linux_64_compile_builder,
                sb_winmobile_emulator_compile_builder,
                sb_solaris_sparc_compile_builder,
                sb_solaris_sparc2_compile_builder,
                sb_android_compile_builder,
                sb_linux_arm_compile_builder,
                sb_linux_arm2_compile_builder,
                
                sb_windows_smoke_builder,
                sb_windows_64_smoke_builder,
                sb_mac_intel_104_smoke_builder,
                sb_mac_intel_105_smoke_builder,
                sb_mac_intel_64_smoke_builder,
                sb_mac_ppc_104a_smoke_builder,
                sb_mac_ppc_104b_smoke_builder,
                sb_mac_ppc_105a_smoke_builder,
                sb_mac_ppc_105b_smoke_builder,
                sb_mac_ppc_64_smoke_builder,
                sb_linux_smoke_builder,
                sb_linux_64_smoke_builder,
                sb_winmobile_emulator_smoke_builder,
                sb_solaris_sparc_smoke_builder,
                sb_solaris_sparc2_smoke_builder,
                sb_android_smoke_builder,
                sb_linux_arm_smoke_builder,
                sb_linux_arm2_smoke_builder,
                
                sb_windows_test_builder,
                sb_windows_64_test_builder,
                sb_mac_intel_104_test_builder,
                sb_mac_intel_105_test_builder,
                sb_mac_intel_64_test_builder,
                sb_mac_ppc_104a_test_builder,
                sb_mac_ppc_104b_test_builder,
                sb_mac_ppc_105a_test_builder,
                sb_mac_ppc_105b_test_builder,
                sb_mac_ppc_64_test_builder,
                sb_linux_test_builder,
                sb_linux_64_test_builder,
                sb_winmobile_emulator_test_builder,
                sb_solaris_sparc_test_builder,
                sb_solaris_sparc2_test_builder,
                sb_android_test_builder,
                sb_linux_arm_test_builder,
                sb_linux_arm2_test_builder,

                ]

