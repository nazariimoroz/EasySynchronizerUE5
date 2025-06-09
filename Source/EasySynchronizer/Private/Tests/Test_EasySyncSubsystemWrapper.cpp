// Fill out your copyright notice in the Description page of Project Settings.


#include "Test_EasySyncSubsystemWrapper.h"


FTest_EasySyncSubsystemWrapper::FTest_EasySyncSubsystemWrapper(UEasySyncSubsystem* InSyncSubsystem)
{
	SyncSubsystem = InSyncSubsystem;
}

TSet<TSharedPtr<FEasySyncEntry>>& FTest_EasySyncSubsystemWrapper::GetEntriesSet() const
{
	return SyncSubsystem->EntriesSet;
}

TMap<int32, TMultiMap<FGameplayTag, TWeakPtr<FEasySyncKey>>>& FTest_EasySyncSubsystemWrapper::GetSyncTags() const
{
	return SyncSubsystem->SyncTags;
}

TMultiMap<TSubclassOf<UEasySyncBaseCondition>, TWeakPtr<FEasySyncKey>>& FTest_EasySyncSubsystemWrapper::GetSyncConditions() const
{
	return SyncSubsystem->Class_SyncConditions;
}
