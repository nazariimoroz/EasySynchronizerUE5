// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySynchronizer/Conditions/EasySyncBaseCondition.h"


FEasySyncConditionHandler::FEasySyncConditionHandler(const TSubclassOf<UEasySyncBaseCondition>& InConditionClass,
                                                     const TSharedPtr<FEasySyncBaseConditionData>& InData)
{
	ConditionClass = InConditionClass;
	Data = InData;
}

EEasySyncConditionStatus FEasySyncConditionHandler::ConditionStatus() const
{
	return
		ConditionClass.Get()
		->GetDefaultObject<UEasySyncBaseCondition>()
		->ConditionStatus(*Data.Get());
}

uint32 FEasySyncConditionHandler::GetHashFrom(const TSubclassOf<UEasySyncBaseCondition>& Class, const FEasySyncBaseConditionData& Data)
{
	uint32 Hash = GetTypeHash(Class);
	Hash = HashCombine(Hash, GetTypeHash(Data));
	return Hash;
}

uint32 GetTypeHash(const FEasySyncConditionHandler& SyncKey)
{
	return FEasySyncConditionHandler::GetHashFrom(SyncKey.ConditionClass, *SyncKey.Data);
}


EEasySyncConditionStatus UEasySyncBaseCondition::ConditionStatus_Implementation(const FEasySyncBaseConditionData& Data)
{
	return EEasySyncConditionStatus::Error;
}
