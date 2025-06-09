// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/CoreUObject/Public/UObject/Object.h"
#include "EasySynchronizer/EasySyncUtils.h"
#include "EasySyncBaseCondition.generated.h"

struct FEasySyncBaseConditionData;
class UEasySyncBaseCondition;
struct FEasySyncEntry;


template<class T>
struct TEasySyncConditionOptionsBase
{
	enum
	{
		bErrorOnNoHintWithNonZeroConditionData = true
	};
};


template<class T>
struct TEasySyncConditionOptions : TEasySyncConditionOptionsBase<T> {};

namespace EasySync
{
	inline struct FEasySyncNoConditionHint {} NoConditionHint;
}

/**
 * One instance of handler to one instance data
 * One instance of handler to Condition CDO
 */
struct EASYSYNCHRONIZER_API FEasySyncConditionHandler
{
	FEasySyncConditionHandler(const TSubclassOf<UEasySyncBaseCondition>& InConditionClass,
		                 const TSharedPtr<FEasySyncBaseConditionData>& InData);

	TSubclassOf<UEasySyncBaseCondition> ConditionClass;
	TSharedPtr<FEasySyncBaseConditionData> Data;

	EEasySyncConditionStatus ConditionStatus() const;

	static uint32 GetHashFrom(const TSubclassOf<UEasySyncBaseCondition>& Class, const FEasySyncBaseConditionData& Data);
	friend uint32 GetTypeHash(const FEasySyncConditionHandler& SyncKey);
};



USTRUCT(BlueprintType, Blueprintable)
struct EASYSYNCHRONIZER_API FEasySyncBaseConditionData
{
	GENERATED_BODY()

	virtual ~FEasySyncBaseConditionData() = default;

	virtual uint32 GetHash() const { return 0; };
	friend uint32 GetTypeHash(const FEasySyncBaseConditionData& Data) { return Data.GetHash(); }
};



/**
 * Wait example:
 * ->WaitFor({UAB_GameStateCondition::Use(FAB_GameStateConditionData{PC})}, ...);
 *
 * Broadcast example:
 * ->Broadcast(this, UAB_GameStateCondition::Use(FAB_GameStateConditionData{PC}));
 */
UCLASS(Blueprintable, BlueprintType)
class EASYSYNCHRONIZER_API UEasySyncBaseCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	EEasySyncConditionStatus ConditionStatus(const FEasySyncBaseConditionData& Data);

protected:
	virtual EEasySyncConditionStatus ConditionStatus_Implementation(const FEasySyncBaseConditionData& Data);

	template<class T>
	const T* CastConditionData(const FEasySyncBaseConditionData& Data)
	{
		return static_cast<const T*>(&Data);
	}
};
