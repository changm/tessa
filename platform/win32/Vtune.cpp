/* -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
/* vi: set ts=4 sw=4 expandtab: (add to ~/.vimrc: set modeline modelines=5) */
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
* The Original Code is [Open Source Virtual Machine.].
*
* The Initial Developer of the Original Code is
* Adobe System Incorporated.
* Portions created by the Initial Developer are Copyright (C) 2004-2006
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*   Adobe AS3 Team. Modified by Shengnan Cong@Intel.
*   Tamarin. Refactored by Rick Reitmaier
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
#include "avmplus.h"

#ifdef INTEL_VTUNE
#include "JITProfiling.h"

namespace avmplus
{
	iJIT_IsProfilingActiveFlags profiler;

   /***
    * Mason's modified VTune settings
	*/
	void VTune_NotifyLoad(char *methodName, int methodId, void* methodAddress, unsigned int jitCompiledSize) {
		iJIT_Method_Load methodLoad;
		memset(&methodLoad, 0, sizeof(iJIT_Method_Load));

	    methodLoad.method_id = methodId;              
	    methodLoad.method_name = methodName;          
	    methodLoad.method_load_address = methodAddress;  
	    methodLoad.method_size = jitCompiledSize;       

		// Constants in this example
	    methodLoad.line_number_size = 0;        // Line Table size in number of entries - Zero if none
	    methodLoad.line_number_table = NULL;    // Pointer to the begining of the line numbers info array
	    methodLoad.class_id = 0;                // uniq class ID
	    methodLoad.class_file_name = NULL;      // class file name 
	    methodLoad.source_file_name = NULL;     // source file name
	    methodLoad.user_data = NULL;            // bits supplied by the user for saving in the JIT file...
	    methodLoad.user_data_size = 0;          // the size of the user data buffer
	    methodLoad.env = iJDE_JittingAPI;
	    
	    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &methodLoad);
	}

	void NotifyUnload(int methodIdToUnload) {
	    iJIT_Method_Id methodId;
	    methodId.method_id = methodIdToUnload;
	    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_UNLOAD_START, &methodId);
	} 

	void VTUNE_CallBack(void* userData, iJIT_ModeFlags flags) {
		(void) userData;
		if (flags == 0x00) {
	        printf("No notification since VTune analyzer is not running\n");
	    }

		if (flags & iJIT_BE_NOTIFY_ON_LOAD) {
	        printf("Notify VTune analyzer of jitted functions that are loaded\n");
	    }

		if (flags & iJIT_BE_NOTIFY_ON_UNLOAD) {
	        printf("Notify VTune analyzer of jitted functions that are unloaded\n");
	    }

		if (flags & iJIT_BE_NOTIFY_ON_METHOD_ENTRY) {
	        printf("Notify VTune analyzer on entry of jitted functions\n");
	    }

		if (flags & iJIT_BE_NOTIFY_ON_METHOD_EXIT) {
	        printf("Notify VTune analyzer on exit of jitted functions\n");
	    }
	}

	void VTune_ShutdownNotifications() {
	    int iResult;
	    iResult = iJIT_NotifyEvent(iJVM_EVENT_TYPE_SHUTDOWN, NULL);
	    printf("Notify with iJVM_EVENT_TYPE_SHUTDOWN returned %d\n", iResult );
	} //end ShutdownNotifications

	void VTune_FinishJitProfiling() {
		VTune_ShutdownNotifications();
	}

	void VTune_InitJitProfiling() {
		//Register the call back to notifiy application of profiling mode changes
	    iJIT_RegisterCallbackEx( NULL, reinterpret_cast<iJIT_ModeChangedEx>(&VTUNE_CallBack));
	}

}	// End namespace


#endif