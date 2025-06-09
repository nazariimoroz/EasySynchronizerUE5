// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasySynchronizer/EasySyncSubsystem.h"

class UEasySyncBaseCondition;
struct FGameplayTag;
struct FEasySyncEntry;

class FTest_EasySyncSubsystemWrapper
{
public:
	FTest_EasySyncSubsystemWrapper(UEasySyncSubsystem* InSyncSubsystem);

	TSet<TSharedPtr<FEasySyncEntry>>& GetEntriesSet() const;

	TMap<int32, TMultiMap<FGameplayTag, TWeakPtr<FEasySyncKey>>>& GetSyncTags() const;
	TMultiMap<TSubclassOf<UEasySyncBaseCondition>, TWeakPtr<FEasySyncKey>>& GetSyncConditions() const;

	UEasySyncSubsystem* SyncSubsystem;
};
