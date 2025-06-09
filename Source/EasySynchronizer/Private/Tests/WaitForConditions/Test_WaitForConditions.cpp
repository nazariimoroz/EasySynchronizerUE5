#include "Test_WaitForConditions_Conditions.h"
#include "EasySynchronizer/EasySyncSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTest_WaitForConditions, "EasySynchronizer.Tests.WaitForConditions",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTest_WaitForConditions::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

	UEasySyncSubsystem::Get(World)
		->Broadcast<UTest_GlobalProperty2Condition>(World, EasySync::NoConditionHint);

	UEasySyncSubsystem::Get(World)
		->Broadcast<UTest_Property1Condition>(World, {nullptr});

	return true;
}
