// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <variant>

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Conditions/EasySyncBaseCondition.h"
#include "EasySyncUtils.h"
#include "Runtime/Engine/Public/Subsystems/WorldSubsystem.h"
#include "EasySyncSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FOwnedGameplayTag
{
	GENERATED_BODY()

public:
	FOwnedGameplayTag() = default;
	FOwnedGameplayTag(UObject* InOwner, const FGameplayTag& InTag)
		: Owner(InOwner)
		, Tag(InTag)
	{}

protected:
	TWeakObjectPtr<UObject> Owner;
	FGameplayTag Tag;

public:
	UObject* GetOwner() const { return Owner.Get(); }
	FGameplayTag GetTag() const { return Tag; }


public:
	friend uint32 GetTypeHash(const FOwnedGameplayTag& SyncKey)
	{
		uint32 Hash = GetTypeHash(SyncKey.GetOwner());
		Hash = HashCombine(Hash, GetTypeHash(SyncKey.GetTag()));
		return Hash;
	}
};


struct FEasySyncEntry;
class FNativeGameplayTag;

struct EASYSYNCHRONIZER_API FEasySyncKey
{
	friend FEasySyncEntry;

	using FValueType = std::variant<TSharedPtr<FEasySyncConditionHandler>, FGameplayTag, FOwnedGameplayTag>;

	FEasySyncKey(const FGameplayTag& Tag);
	FEasySyncKey(const FNativeGameplayTag& Tag);

	FEasySyncKey(const TSharedPtr<FEasySyncConditionHandler>& SyncCondition);
	FEasySyncKey(const FEasySyncConditionHandlerBPWrapper& SyncCondition);

	FEasySyncKey(const FOwnedGameplayTag& OwnedTag);
	FEasySyncKey(UObject* TagOwner, const FGameplayTag& Tag);

	FEasySyncKey(const FValueType& InValue);

protected:
	FValueType Value;

public:
	const TSharedPtr<FEasySyncConditionHandler>* GetValueAsCondition() const;
	const FGameplayTag* GetValueAsTag() const;
	const FOwnedGameplayTag* GetValueAsOwnedTag() const;


protected:
	bool bPassed = false;
	void ClearPass() { bPassed = false; }
	void MarkPassed() { bPassed = true; }

public:
	bool IsPassed() const { return bPassed; }


protected:
	TWeakPtr<FEasySyncEntry> SyncEntry;

public:
	void SetSyncEntry(const TWeakPtr<FEasySyncEntry>& InSyncEntry)
	{
		SyncEntry = InSyncEntry;
	}
	TSharedPtr<FEasySyncEntry> GetSyncEntry() const
	{
		return SyncEntry.Pin();
	}


public:
	friend uint32 GetTypeHash(const FEasySyncKey& SyncKey);
	friend uint32 GetTypeHash(const TSharedPtr<FEasySyncKey>& SyncEntry);
};

USTRUCT()
struct EASYSYNCHRONIZER_API FEasySyncEntry
{
	GENERATED_BODY()

	using FDelegateType = std::variant<FEasySyncDynamicDelegate, FEasySyncDelegate>;

public:
	static TSharedPtr<FEasySyncEntry> Create(TArray<FEasySyncKey>&& InKeys, FDelegateType&& InDelegate, bool bInOnlyOnce);

protected:
	TArray<TSharedPtr<FEasySyncKey>> Keys;
	void SetKeys(TArray<FEasySyncKey>&& InKeys);

public:
	TArray<TWeakPtr<FEasySyncKey>> GetKeys() const;
	bool IsAllKeyPassed() const;
	void ClearPasses();

	void MarkKeyAsPassed(const TSharedRef<FEasySyncKey>& InSyncKey);


protected:
	FDelegateType Delegate;
	void SetDelegate(FDelegateType&& InDelegate);

public:
	bool Execute();


protected:
	bool bOnlyOnce = false;
	void SetOnlyOnce(bool bInOnlyOnce) { bOnlyOnce = bInOnlyOnce; }

public:
	bool OnlyOnce() const { return bOnlyOnce; }


public:
	bool operator==(const FEasySyncEntry& Other) const;

	friend uint32 GetTypeHash(const FEasySyncEntry& SyncEntry);
	friend uint32 GetTypeHash(const TSharedPtr<FEasySyncEntry>& SyncEntry);
};


USTRUCT(Blueprintable, BlueprintType)
struct EASYSYNCHRONIZER_API FEasyBroadcastParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UObject* Sender = nullptr;
};


/**
* TkSync Module
* Goal: Unify synchronization methods over the code
* Warning: NOT THREAD SAFE
*
* Rules:
* - For WaitFor, WaitForInPlace, and SubscribeFor, only a unique pair of (tag, delegate) or (USync_Smt, delegate) can be added.
* - BroadcastTag("Test1.Test2.Test3") for WaitFor/SubscribeFor("Test1") will broadcast only once with GetLastTag() == "Test1.Test2.Test3"
* - If FTK_SyncDelegate was used, then is can be truly compared only of it was created with ::CreateUObject or ::CreateUFUnction, unless compare operator will always return false
*
* Usage example(PlayerComponent):
* UFUNCTION()
* void SomeMethod(<must be empty>)
* {
*     if(!UEasySyncSubsystem::Get(this)->WaitForInPlace({
*             USync_GameState::Create(),
*             USync_PlayerState::Create(Owner),
*             FGameplayTag::RequestGameplayTag("Test1.Event")
*        }, FEasySyncDelegate::CreateUObject(this, &ThisClass::SomeMethod))) return;
*
*	  // GameState and PlayerState will be available here
*	  // and Test1.Event will be done
* }
*
*
*
* Usage example 2(GameMode):
* void StartPlay()
* {
*     UEasySyncSubsystem::Get(this)->SubscribeFor(FGameplayTag::RequestGameplayTag("ProjectName.Events"),
*                                                FEasySyncDelegate::CreateUObject(this, &UEasySyncSubsystem::Sync_OnEventBroadcast));
*
*     UEasySyncSubsystem::Get(this)->WaitFor({FGameplayTag::RequestGameplayTag("ProjectName.Events"), FGameplayTag::RequestGameplayTag("ProjectName.Events")},
*                                                FEasySyncDelegate::CreateUObject(this, &UEasySyncSubsystem::Sync_OnEventBroadcast));
* }
*
* UPROPERTY()
* void Sync_OnEventBroadcast()
* {
*     // Broadcast params:
*     // UEasySyncSubsystem::Get(this)->GetLastSender();
*     // UEasySyncSubsystem::Get(this)->GetLastTag();
*     // UEasySyncSubsystem::Get(this)->GetLastData();
* }
*
* // Broadcast example:
* void OnRep_SomeVar()
* {
*     // UEasySyncSubsystem::Get(this)->Broadcast(FGameplayTag::RequestGameplayTag("ProjectName.Events.TestEvent"));
*
*     // with Sender and Data
*     // FAB_MyData Params; // struct FAB_MyData : FTK_BroadcastData
*     // UEasySyncSubsystem::Get(this)->Broadcast(this, FGameplayTag::RequestGameplayTag("ProjectName.Events.TestEvent"),
*     //                                         { .Data = &Params });
* }
*
*
*
* Usage example 3(GameMode):
* void StartPlay()
* {
*
* }
*
*/
UCLASS()
class EASYSYNCHRONIZER_API UEasySyncSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	friend class FTest_EasySyncSubsystemWrapper;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual TStatId GetStatId() const override;

	static UEasySyncSubsystem* Get(const UObject* ContextObject);

public:
	void Broadcast(UObject* Sender, const FGameplayTag& Tag, FEasyBroadcastParams Params = {});

	template<class TCondition>
	void Broadcast(UObject* Sender,
		           typename TCondition::FConditionDataType&& Hint,
		           FEasyBroadcastParams Params = {})
	{
		auto PrevParams = std::exchange(LastParams, Params);
		LastParams.Sender = Sender;
		BroadcastConditionByHint(FEasySyncConditionHandler::GetHashFrom(TCondition::StaticClass(), Hint));
		LastParams = MoveTemp(PrevParams);
	}

	template<class TCondition>
	void Broadcast(UObject* Sender,
				   const EasySync::FEasySyncNoConditionHint&,
				   FEasyBroadcastParams Params = {})
	{
		if constexpr (TEasySyncConditionOptions<TCondition>::bErrorOnNoHintWithNonZeroConditionData &&
			          TCondition::FConditionDataType::DataPropertiesNum != 0)
		{
			static_assert(
				EasySync::TTemplateFalse<TCondition>,
				"Broadcasting with no hint but non-zero condition data properties is not allowed and can lead to UB."
				"Disable TEasySyncConditionOptions<>::bErrorOnNoHintWithNonZeroConditionData flag to get rid of this error"
			);
		}

		auto PrevParams = std::exchange(LastParams, Params);
		LastParams.Sender = Sender;
		BroadcastConditionNoHint(TCondition::StaticClass());
		LastParams = MoveTemp(PrevParams);
	}


	void WaitFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncEntry::FDelegateType&& SyncDelegate);
	void SubscribeFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncEntry::FDelegateType&& SyncDelegate);

protected:
	void RegisterEntry(TSharedPtr<FEasySyncEntry> SyncEntry);
	void RemoveEntry(TSharedPtr<FEasySyncEntry>&& SyncEntry);
	void BroadcastTag(const FGameplayTag& Tag);
	void BroadcastTagWithOwner(UObject* TagOwner, const FGameplayTag& Tag);
	void BroadcastConditionByHint(uint32 Hash);
	void BroadcastConditionNoHint(const TSubclassOf<UEasySyncBaseCondition>& ConditionClass);

	TArray<TWeakPtr<FEasySyncKey>> GetSyncKeysByTag(const FGameplayTag& Tag);

	bool/** bRemove */ TryExecuteEntry(const TSharedPtr<FEasySyncEntry>& Entry);

protected:
	/** Entry must be set in EntriesSet and in one EntriesMap_* */
	TSet<TSharedPtr<FEasySyncEntry>> EntriesSet;

	TMap<int32/** Level */, TMultiMap<FGameplayTag, TWeakPtr<FEasySyncKey>>> SyncTags;

	TMultiMap<uint32/* Hash */, TWeakPtr<FEasySyncKey>> Hash_SyncConditions;
	TMultiMap<TSubclassOf<UEasySyncBaseCondition>, TWeakPtr<FEasySyncKey>> Class_SyncConditions;

	TMultiMap<uint32/** Hash */, TWeakPtr<FEasySyncKey>> Hash_OwnedSyncTags;

private:
	FEasyBroadcastParams LastParams;

};
