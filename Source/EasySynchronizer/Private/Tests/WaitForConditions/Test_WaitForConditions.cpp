#include "Test_WaitForConditions_Actors.h"
#include "Test_WaitForConditions_Conditions.h"
#include "EasySynchronizer/EasySyncSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/Test_Utils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTest_WaitForConditions, "EasySynchronizer.Tests.WaitForConditions",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTest_WaitForConditions_PassWaiter, TSharedRef<bool>, bPassed);
bool FTest_WaitForConditions_PassWaiter::Update()
{
	return *bPassed;
}

bool FTest_WaitForConditions::RunTest(const FString& Parameters)
{
	EasySyncTest::bSomeGlobalVariable = false;

	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

	auto PassCount = MakeShared<int>(0);
	auto bPassed = MakeShared<bool>(false);

	const auto BroadcastActor1 = World->SpawnActor<ATest_WaitForConditions_BroadcastActor>();
	const auto BroadcastActor2 = World->SpawnActor<ATest_WaitForConditions_BroadcastActor>();
	const auto WaiterActor = World->SpawnActor<ATest_WaitForConditions_WaiterActor>();
	WaiterActor->OnDoneDelegate.BindLambda([this, PassCount, bPassed]
	{
		this->AddInfo("Passed!!!");
		(*bPassed) = true;
		if ( (*PassCount) != 4 )
		{
			this->AddError("WaiterActor Passed before called all conditions and tags");
			return;
		}
	});

	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(World, [this, PassCount, WaiterActor, BroadcastActor1, BroadcastActor2]
		{
			this->AddInfo("StartWaiting");
			(*PassCount)++;
			WaiterActor->StartWaiting(BroadcastActor1, BroadcastActor2);
		}), FMath::RandRange(0.2, 0.5), false);
	}

	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(World, [this, PassCount, WaiterActor, BroadcastActor1, BroadcastActor2]
		{
			this->AddInfo("BroadcastActor1->Broadcast");
			(*PassCount)++;
			BroadcastActor1->Broadcast();
		}), FMath::RandRange(0.2, 0.5), false);
	}

	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(World, [this, PassCount, WaiterActor, BroadcastActor1, BroadcastActor2]
		{
			this->AddInfo("BroadcastActor2->Broadcast");
			(*PassCount)++;
			BroadcastActor2->Broadcast();
		}), FMath::RandRange(0.2, 0.5), false);
	}

	{
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(World, [this, World, PassCount]
		{
			this->AddInfo("UTest_GlobalProperty2Condition->Broadcast");
			(*PassCount)++;
			EasySyncTest::bSomeGlobalVariable = true;
			UEasySyncSubsystem::Get(World)->Broadcast<UTest_GlobalProperty2Condition>(nullptr, EasySync::NoConditionHint);
		}), FMath::RandRange(0.2, 0.5), false);
	}

	ADD_LATENT_AUTOMATION_COMMAND(FTest_WaitForConditions_PassWaiter(bPassed));

	ADD_LATENT_AUTOMATION_COMMAND(EasySyncTest::FDelayedCallbackLatentCommand([=, this]
	{
		if (!(*bPassed))
		{
			this->AddError("Condition was not passed");
		}
	}));

	return true;
}
