#                     DO NOT EDIT THIS FILE
# 
#  This file has been generated by the script core/avmfeatures.as,
#  from a set of configuration parameters in that file.
# 
#  If you feel you need to make changes below, instead edit the configuration
#  file and rerun it to get a new version of this file.
# 
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

def featureSettings(o):
    args = ""
    arg = o.getBoolArg("debugger")
    if (arg == True):
        args += "-DAVMFEATURE_DEBUGGER=1 "
    if (arg == False):
        args += "-DAVMFEATURE_DEBUGGER=0 "
    arg = o.getBoolArg("debugger-stub")
    if (arg == True):
        args += "-DAVMFEATURE_DEBUGGER_STUB=1 "
    if (arg == False):
        args += "-DAVMFEATURE_DEBUGGER_STUB=0 "
    arg = o.getBoolArg("allocation-sampler")
    if (arg == True):
        args += "-DAVMFEATURE_ALLOCATION_SAMPLER=1 "
    if (arg == False):
        args += "-DAVMFEATURE_ALLOCATION_SAMPLER=0 "
    arg = o.getBoolArg("vtune")
    if (arg == True):
        args += "-DAVMFEATURE_VTUNE=1 "
    if (arg == False):
        args += "-DAVMFEATURE_VTUNE=0 "
    if o.getBoolArg("jit"):
        args += "-DAVMFEATURE_JIT=1 -DAVMFEATURE_WORDCODE_INTERP=0 "
    arg = o.getBoolArg("aot")
    if (arg == True):
        args += "-DAVMFEATURE_AOT=1 "
    if (arg == False):
        args += "-DAVMFEATURE_AOT=0 "
    if o.getBoolArg("abc-interp"):
        args += "-DAVMFEATURE_ABC_INTERP=1 -DAVMFEATURE_WORDCODE_INTERP=0 "
    if o.getBoolArg("wordcode-interp"):
        args += "-DAVMFEATURE_WORDCODE_INTERP=1 -DAVMFEATURE_ABC_INTERP=0 -DAVMFEATURE_JIT=0 "
    arg = o.getBoolArg("threaded-interp")
    if (arg == True):
        args += "-DAVMFEATURE_THREADED_INTERP=1 "
    if (arg == False):
        args += "-DAVMFEATURE_THREADED_INTERP=0 "
    arg = o.getBoolArg("selftest")
    if (arg == True):
        args += "-DAVMFEATURE_SELFTEST=1 "
    if (arg == False):
        args += "-DAVMFEATURE_SELFTEST=0 "
    arg = o.getBoolArg("eval")
    if (arg == True):
        args += "-DAVMFEATURE_EVAL=1 "
    if (arg == False):
        args += "-DAVMFEATURE_EVAL=0 "
    arg = o.getBoolArg("protect-jitmem")
    if (arg == True):
        args += "-DAVMFEATURE_PROTECT_JITMEM=1 "
    if (arg == False):
        args += "-DAVMFEATURE_PROTECT_JITMEM=0 "
    arg = o.getBoolArg("shared-gcheap")
    if (arg == True):
        args += "-DAVMFEATURE_SHARED_GCHEAP=1 "
    if (arg == False):
        args += "-DAVMFEATURE_SHARED_GCHEAP=0 "
    arg = o.getBoolArg("use-system-malloc")
    if (arg == True):
        args += "-DAVMFEATURE_USE_SYSTEM_MALLOC=1 "
    if (arg == False):
        args += "-DAVMFEATURE_USE_SYSTEM_MALLOC=0 "
    arg = o.getBoolArg("cpp-exceptions")
    if (arg == True):
        args += "-DAVMFEATURE_CPP_EXCEPTIONS=1 "
    if (arg == False):
        args += "-DAVMFEATURE_CPP_EXCEPTIONS=0 "
    arg = o.getBoolArg("interior-pointers")
    if (arg == True):
        args += "-DAVMFEATURE_INTERIOR_POINTERS=1 "
    if (arg == False):
        args += "-DAVMFEATURE_INTERIOR_POINTERS=0 "
    arg = o.getBoolArg("jni")
    if (arg == True):
        args += "-DAVMFEATURE_JNI=1 "
    if (arg == False):
        args += "-DAVMFEATURE_JNI=0 "
    arg = o.getBoolArg("heap-alloca")
    if (arg == True):
        args += "-DAVMFEATURE_HEAP_ALLOCA=1 "
    if (arg == False):
        args += "-DAVMFEATURE_HEAP_ALLOCA=0 "
    arg = o.getBoolArg("static-function-ptrs")
    if (arg == True):
        args += "-DAVMFEATURE_STATIC_FUNCTION_PTRS=1 "
    if (arg == False):
        args += "-DAVMFEATURE_STATIC_FUNCTION_PTRS=0 "
    arg = o.getBoolArg("indirect-native-thunks")
    if (arg == True):
        args += "-DAVMFEATURE_INDIRECT_NATIVE_THUNKS=1 "
    if (arg == False):
        args += "-DAVMFEATURE_INDIRECT_NATIVE_THUNKS=0 "
    arg = o.getBoolArg("override-global-new")
    if (arg == True):
        args += "-DAVMFEATURE_OVERRIDE_GLOBAL_NEW=1 "
    if (arg == False):
        args += "-DAVMFEATURE_OVERRIDE_GLOBAL_NEW=0 "
    arg = o.getBoolArg("memory-profiler")
    if (arg == True):
        args += "-DAVMFEATURE_MEMORY_PROFILER=1 "
    if (arg == False):
        args += "-DAVMFEATURE_MEMORY_PROFILER=0 "
    arg = o.getBoolArg("cache-gqcn")
    if (arg == True):
        args += "-DAVMFEATURE_CACHE_GQCN=1 "
    if (arg == False):
        args += "-DAVMFEATURE_CACHE_GQCN=0 "
    arg = o.getBoolArg("api-versioning")
    if (arg == True):
        args += "-DAVMFEATURE_API_VERSIONING=1 "
    if (arg == False):
        args += "-DAVMFEATURE_API_VERSIONING=0 "
    arg = o.getBoolArg("sin-cos-nonfinite")
    if (arg == True):
        args += "-DAVMTWEAK_SIN_COS_NONFINITE=1 "
    if (arg == False):
        args += "-DAVMTWEAK_SIN_COS_NONFINITE=0 "
    return args
