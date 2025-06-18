// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Test_WaitForThreeTagsActor.generated.h"

UCLASS()
class ATest_WaitForThreeTagsActor : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	USceneComponent* BaseComponent = nullptr;

public:
	ATest_WaitForThreeTagsActor();

	void Execute();

	UFUNCTION()
	void Passed();

	int PassCount = 0;
};
