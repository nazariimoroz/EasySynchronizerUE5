// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_WaitForConditions_Conditions.h"

#include "Test_WaitForConditions_Actors.h"
#include "GameFramework/PlayerState.h"

EEasySyncConditionStatus UTest_Property1Condition::ConditionStatusInternal(const TWeakObjectPtr<ATest_WaitForConditions_BroadcastActor>& WaiterActor)
{
	if (!WaiterActor.IsValid()) return EEasySyncConditionStatus::Error;

	return WaiterActor->Property1 == 1 ? EEasySyncConditionStatus::Pass : EEasySyncConditionStatus::Wait;
}

uint32 UTest_Property1Condition::FConditionDataType::GetHash() const
{
	return GetTypeHash(WaiterActor.Get());
}


EEasySyncConditionStatus UTest_GlobalProperty2Condition::ConditionStatusInternal()
{
	return EasySyncTest::bSomeGlobalVariable ? EEasySyncConditionStatus::Pass : EEasySyncConditionStatus::Wait;
}

