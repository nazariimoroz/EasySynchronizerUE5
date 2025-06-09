// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_WaitForConditions_Conditions.h"

#include "Test_WaitForConditions_Actors.h"
#include "GameFramework/PlayerState.h"

EEasySyncConditionStatus UTest_Property1Condition::ConditionStatusInternal(const TWeakObjectPtr<ATest_WaitForConditions_BroadcastActor>& WaiterActor)
{
	if (!WaiterActor.IsValid()) return EEasySyncConditionStatus::Error;

	UE_LOG(LogTemp, Warning, TEXT("WA: %lld | %i"), reinterpret_cast<intptr_t>(WaiterActor.Get()), WaiterActor->Property1)
	return WaiterActor->Property1 == 1 ? EEasySyncConditionStatus::Pass : EEasySyncConditionStatus::Wait;
}

EEasySyncConditionStatus UTest_GlobalProperty2Condition::ConditionStatusInternal()
{
	UE_LOG(LogTemp, Warning, TEXT("EasySyncTest::bSomeGlobalVariable | %i"), (int)EasySyncTest::bSomeGlobalVariable);
	return EasySyncTest::bSomeGlobalVariable ? EEasySyncConditionStatus::Pass : EEasySyncConditionStatus::Wait;
}

