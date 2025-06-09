// Fill out your copyright notice in the Description page of Project Settings.


#include "Test_WaitForConditions_Actors.h"

#include "Test_WaitForConditions_Conditions.h"
#include "EasySynchronizer/EasySyncSubsystem.h"


ATest_WaitForConditions_WaiterActor::ATest_WaitForConditions_WaiterActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATest_WaitForConditions_WaiterActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATest_WaitForConditions_WaiterActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

