namespace avmplus {
#ifdef INTEL_VTUNE
	typedef enum iJIT_ModeFlags;

	void VTune_NotifyLoad(char *methodName, int methodId, void* methodAddress, unsigned int jitCompiledSize); 
	void VTune_InitJitProfiling(); 
	void VTune_FinishJitProfiling(); 
	void VTune_ShutdownNotifications(); 
	void VTUNE_CallBack(void* userData, iJIT_ModeFlags flags); 
	void NotifyUnload(int methodIdToUnload); 
#endif
}