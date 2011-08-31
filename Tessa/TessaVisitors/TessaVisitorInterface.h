
#ifndef __TESSAVISITORS__TESSAVISITORINTERFACE__
#define __TESSAVISITORS__TESSAVISITORINTERFACE__
/***
 * Cheap hack to mimic interfaces like Java.
 * This provides the interface for all TessaVisitors
 */
namespace TessaVisitors {
	using namespace TessaInstructions;

	class TessaVisitorInterface : public MMgc::GCObject {
	public:
		TessaVisitorInterface();
		virtual ~TessaVisitorInterface();
		virtual void visit(TessaInstruction* tessaInstruction) = 0;
		virtual void visit(FindPropertyInstruction* findPropertyInstruction) = 0;
		virtual void visit(ConstantValueInstruction* constantValueInstruction) = 0;
		virtual void visit(ArrayOfInstructions* arrayOfInstructions) = 0;
		virtual void visit(ConstructInstruction* constructInstruction) = 0;
		virtual void visit(ConstructSuperInstruction* constructSuperInstruction) = 0;
		virtual void visit(ConstructPropertyInstruction* constructPropertyInstruction) = 0;
		virtual void visit(CallInstruction* callInstruction) = 0;
		virtual void visit(CallVirtualInstruction* callVirtualInstruction) = 0;
		virtual void visit(CallStaticInstruction* callStaticInstruction) = 0;
		virtual void visit(CallInterfaceInstruction* callInterfaceInstruction) = 0;
		virtual void visit(CallSuperInstruction* callSuperInstruction) = 0;
		virtual void visit(CallPropertyInstruction* callPropertyInstruction) = 0;
		virtual void visit(LoadVirtualMethodInstruction* loadVirtualMethodInstruction) = 0;
		virtual void visit(ReturnInstruction* returnInstruction) = 0; 
		virtual void visit(BinaryInstruction* binaryInstruction) = 0; 
		virtual void visit(ConditionInstruction* conditionInstruction) = 0;
		virtual void visit(TessaInstructions::UnaryInstruction* binaryInstruction) = 0; 
		virtual void visit(UnconditionalBranchInstruction* unconditionalBranchInstruction) = 0;
		virtual void visit(ConditionalBranchInstruction* conditionalBranchInstruction) = 0;
		virtual void visit(PhiInstruction* phiInstruction) = 0;
		virtual void visit(ParameterInstruction* parameterInstruction) = 0;
		virtual void visit(CoerceInstruction* coerceInstruction) = 0;
		virtual void visit(ConvertInstruction* convertInstruction) = 0;
		virtual void visit(GetPropertyInstruction* getPropertyInstruction) = 0;
		virtual void visit(SetPropertyInstruction* setPropertyInstruction) = 0;
		virtual void visit(InitPropertyInstruction* initPropertyInstruction) = 0;
		virtual void visit(GetSlotInstruction* getSlotInstruction) = 0;
		virtual void visit(SetSlotInstruction* setSlotInstruction) = 0;
		virtual void visit(NewArrayInstruction* newArrayInstruction) = 0;
		virtual void visit(NextNameInstruction* nextNameInstruction) = 0;
		virtual void visit(PushScopeInstruction* pushScopeInstruction) = 0;
		virtual void visit(PopScopeInstruction* popScopeInstruction) = 0;
		virtual void visit(WithInstruction* withInstruction) = 0;
		virtual void visit(TypeOfInstruction* typeOfInstruction) = 0;
		virtual void visit(GetScopeObjectInstruction* getScopeObjectInstruction) = 0;
		virtual void visit(GetGlobalScopeInstruction* getGlobalScopeInstruction) = 0;
		virtual void visit(HasMorePropertiesInstruction* hasMorePropertiesInstruction) = 0;
		virtual void visit(HasMorePropertiesObjectInstruction* hasMorePropertiesInstruction) = 0;
		virtual void visit(HasMorePropertiesRegisterInstruction* hasMorePropertiesIndexInstruction) = 0;
		virtual void visit(SwitchInstruction* switchInstruction) = 0;
		virtual void visit(CaseInstruction* caseInstruction) = 0;
		virtual void visit(NewObjectInstruction* newObjectInstruction) = 0;
		virtual void visit(NewActivationInstruction* newActiviationInstruction) = 0;
		virtual void visit(InlineBeginInstruction* inlineBeginInstruction) = 0;
	};
}

#endif