// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Test_WaitForConditions_Actors.generated.h"

class ATest_WaitForConditions_BroadcastActor;

UCLASS()
class ATest_WaitForConditions_WaiterActor : public AActor
{
	GENERATED_BODY()

public:
	void StartWaiting(ATest_WaitForConditions_BroadcastActor* Property1Handler, ATest_WaitForConditions_BroadcastActor* Property2Handler);

	DECLARE_DELEGATE(FOnDoneDelegate);
	FOnDoneDelegate OnDoneDelegate;
};

UCLASS()
class ATest_WaitForConditions_BroadcastActor : public AActor
{
	GENERATED_BODY()
public:
	void Broadcast();

	int Property1 = 0;
};

