// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Test_WaitForConditions_Actors.generated.h"

UCLASS()
class ATest_WaitForConditions_WaiterActor : public AActor
{
	GENERATED_BODY()

public:
	ATest_WaitForConditions_WaiterActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
