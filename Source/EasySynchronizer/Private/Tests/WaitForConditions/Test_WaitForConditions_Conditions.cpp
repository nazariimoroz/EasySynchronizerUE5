// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_WaitForConditions_Conditions.h"
#include "GameFramework/PlayerState.h"

EEasySyncConditionStatus UTest_Property1Condition::ConditionStatusInternal(const TWeakObjectPtr<ATest_WaitForConditions_WaiterActor>& WaiterActor)
{
	if (!WaiterActor.IsValid()) return EEasySyncConditionStatus::Error;
	return EEasySyncConditionStatus::Pass;
}

EEasySyncConditionStatus UTest_GlobalProperty2Condition::ConditionStatusInternal()
{
	return GetWorld()->GetGameState() ? EEasySyncConditionStatus::Pass : EEasySyncConditionStatus::Wait;
}

