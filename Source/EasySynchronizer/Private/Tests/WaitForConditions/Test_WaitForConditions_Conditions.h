// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasySynchronizer/EasySyncMacros.h"
#include "EasySynchronizer/Conditions/EasySyncBaseCondition.h"

#include "Test_WaitForConditions_Conditions.generated.h"

class ATest_WaitForConditions_BroadcastActor;

UCLASS()
class UTest_Property1Condition : public UEasySyncBaseCondition
{
	GENERATED_BODY()
	EASYSYNC_GENERATED_CONDITION_OneParam(
		TWeakObjectPtr<ATest_WaitForConditions_BroadcastActor>, InWaiterActor
	);
};


namespace EasySyncTest
{
	inline bool bSomeGlobalVariable = false;
}

UCLASS()
class UTest_GlobalProperty2Condition : public UEasySyncBaseCondition
{
	GENERATED_BODY()
	EASYSYNC_GENERATED_CONDITION();
};
