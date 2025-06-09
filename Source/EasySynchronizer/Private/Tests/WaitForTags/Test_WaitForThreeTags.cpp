#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationCommon.h"
#include "EasySynchronizer/EasySyncSubsystem.h"
#include "Test_WaitForThreeTagsActor.h"
#include "Tests/Test_EasySyncSubsystemWrapper.h"
#include "Tests/Test_Utils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTest_WaitForThreeTags, "EasySynchronizer.Tests.WaitForThreeTags",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/***************** Base Test 1 *****************/
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FTest_WaitForThreeTags_Test1, FTest_WaitForThreeTags*, _Test, UWorld*, _World);
bool FTest_WaitForThreeTags_Test1::Update()
{
	FTest_WaitForThreeTags* Test = _Test;
	UWorld* World = _World;

	const auto SyncSubsystemWrapper = MakeShared<FTest_EasySyncSubsystemWrapper>(UEasySyncSubsystem::Get(World));

	const auto TestActor = World->SpawnActor<ATest_WaitForThreeTagsActor>(ATest_WaitForThreeTagsActor::StaticClass());
	TestActor->Execute();

	if (SyncSubsystemWrapper->GetEntriesSet().Num() == 0)
	{
		Test->AddError("WaitFor have not been added correctly");
		return false;
	}

	auto EntryPtr = *SyncSubsystemWrapper->GetEntriesSet().begin();
	TSharedPtr<int> CalledCount = MakeShared<int>(0);

	{
		FTimerHandle TH;
		World->GetTimerManager().SetTimer(TH, FTimerDelegate::CreateWeakLambda(World, [=]
		{
			UEasySyncSubsystem::Get(World)->Broadcast(World, FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test1"));
			(*CalledCount) += 1;

			if (TestActor->PassCount != 0 && *CalledCount != 3)
			{
				Test->AddError(FString::Printf(TEXT("WaitFor have been passed only after %i tag"), *CalledCount));
			}
		}), FMath::RandRange(0.2f, 1.f), false);
	}

	{
		FTimerHandle TH;
		World->GetTimerManager().SetTimer(TH, FTimerDelegate::CreateWeakLambda(World, [=]
		{
			UEasySyncSubsystem::Get(World)->Broadcast(World, FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test2"));
			(*CalledCount) += 1;

			if (TestActor->PassCount != 0 && *CalledCount != 3)
			{
				Test->AddError(FString::Printf(TEXT("WaitFor have been passed only after %i tag"), *CalledCount));
			}
		}), FMath::RandRange(0.2f, 1.f), false);
	}
	{
		FTimerHandle TH;
		World->GetTimerManager().SetTimer(TH, FTimerDelegate::CreateWeakLambda(World, [=]
		{
			UEasySyncSubsystem::Get(World)->Broadcast(World, FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test3"));
			(*CalledCount) += 1;

			if (TestActor->PassCount != 0 && *CalledCount != 3)
			{
				Test->AddError(FString::Printf(TEXT("WaitFor have been passed only after %i tag"), *CalledCount));
			}
		}), FMath::RandRange(0.2f, 1.f), false);
	}

	ADD_LATENT_AUTOMATION_COMMAND(EasySyncTest::FDelayedCallbackLatentCommand([=]
	{
		UEasySyncSubsystem::Get(World)->Broadcast(World, FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test3"));

		if (TestActor->PassCount == 0)
		{
			Test->AddError("WaitFor have not been called");
			return;
		}
		if (TestActor->PassCount > 1)
		{
			Test->AddError("WaitFor must be called only once");
			return;
		}

		Test->AddErrorIfFalse(SyncSubsystemWrapper->GetEntriesSet().Num() == 0, "SyncEntry have not been removed from subsystem correctly");
		Test->AddErrorIfFalse(SyncSubsystemWrapper->GetSyncTags()[3].Num() == 0, "SyncEntry have not been removed from subsystem correctly");
		Test->AddErrorIfFalse(EntryPtr.GetSharedReferenceCount() == 1, "SyncEntry have not been removed from subsystem correctly");
		Test->AddInfo("WaitForThreeTags_Test1 Passed");
	}, 2.f));

	return true;
}


bool FTest_WaitForThreeTags::RunTest(const FString& Parameters)
{
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

	ADD_LATENT_AUTOMATION_COMMAND(FTest_WaitForThreeTags_Test1(this, World));

	return true;
}
