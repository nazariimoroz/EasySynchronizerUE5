// Fill out your copyright notice in the Description page of Project Settings.

#include "Test_WaitForThreeTagsActor.h"

#include "Runtime/GameplayTags/Classes/GameplayTagContainer.h"
#include "EasySynchronizer/EasySyncSubsystem.h"

ATest_WaitForThreeTagsActor::ATest_WaitForThreeTagsActor()
{
	BaseComponent = CreateDefaultSubobject<USceneComponent>("Base Component");
	SetRootComponent(BaseComponent);
}

void ATest_WaitForThreeTagsActor::Execute()
{
	UEasySyncSubsystem::Get(this)->WaitFor({
		FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test1"),
		FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test2"),
		FGameplayTag::RequestGameplayTag("EasySynchronizer.Test.Test3"),
	}, FEasySyncDelegate::CreateUObject(this, &ThisClass::Passed));
}

void ATest_WaitForThreeTagsActor::Passed()
{
	PassCount += 1;
}
