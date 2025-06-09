// Fill out your copyright notice in the Description page of Project Settings.


#include "Test_WaitForConditions_Actors.h"

#include "Test_WaitForConditions_Conditions.h"
#include "EasySynchronizer/EasySyncSubsystem.h"


void ATest_WaitForConditions_WaiterActor::StartWaiting(ATest_WaitForConditions_BroadcastActor* Property1Handler, ATest_WaitForConditions_BroadcastActor* Property2Handler)
{
	UEasySyncSubsystem::Get(this)->WaitFor(
	{
		UTest_Property1Condition::CheckWith(Property1Handler),
		UTest_Property1Condition::CheckWith(Property2Handler),
		UTest_GlobalProperty2Condition::CheckWith()
	}, FEasySyncDelegate::CreateWeakLambda(this, [this]{OnDoneDelegate.Execute();}));
}

void ATest_WaitForConditions_BroadcastActor::Broadcast()
{
	Property1 = 1;
	UEasySyncSubsystem::Get(this)->Broadcast<UTest_Property1Condition>(this, {this});
}

