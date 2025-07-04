// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySynchronizer/EasySyncSubsystem.h"

#include "NativeGameplayTags.h"

FEasySyncKey::FEasySyncKey(const FGameplayTag& Tag)
{
	Value = Tag;
}

FEasySyncKey::FEasySyncKey(const FNativeGameplayTag& Tag)
{
	Value = Tag.GetTag();
}

FEasySyncKey::FEasySyncKey(const TSharedPtr<FEasySyncConditionHandler>& SyncCondition)
{
	Value = SyncCondition;
}

FEasySyncKey::FEasySyncKey(const FEasySyncConditionHandlerBPWrapper& SyncCondition)
{
	Value = SyncCondition.Handler;
}

FEasySyncKey::FEasySyncKey(const FOwnedGameplayTag& OwnedTag)
{
	Value = OwnedTag;
}

FEasySyncKey::FEasySyncKey(UObject* TagOwner, const FGameplayTag& Tag)
{
	Value = FOwnedGameplayTag(TagOwner, Tag);
}

FEasySyncKey::FEasySyncKey(const FValueType& InValue)
{
	Value = InValue;
}

const TSharedPtr<FEasySyncConditionHandler>* FEasySyncKey::GetValueAsCondition() const
{
	if (const auto SyncCondition = std::get_if<TSharedPtr<FEasySyncConditionHandler>>(&Value))
	{
		return SyncCondition;
	}
	return nullptr;
}

const FGameplayTag* FEasySyncKey::GetValueAsTag() const
{
	if (const auto Tag = std::get_if<FGameplayTag>(&Value))
	{
		return Tag;
	}
	return nullptr;
}

const FOwnedGameplayTag* FEasySyncKey::GetValueAsOwnedTag() const
{
	if (const auto OwnedTag = std::get_if<FOwnedGameplayTag>(&Value))
	{
		return OwnedTag;
	}
	return nullptr;
}

uint32 GetTypeHash(const FEasySyncKey& SyncKey)
{
	if (const auto SyncCondition = std::get_if<TSharedPtr<FEasySyncConditionHandler>>(&SyncKey.Value))
	{
		return GetTypeHash(**SyncCondition);
	}
	if (const auto Tag = std::get_if<FGameplayTag>(&SyncKey.Value))
	{
		return GetTypeHash(*Tag);
	}
	if (const auto OwnedTag = std::get_if<FOwnedGameplayTag>(&SyncKey.Value))
	{
		return GetTypeHash(*OwnedTag);
	}
	return 0;
}

uint32 GetTypeHash(const TSharedPtr<FEasySyncKey>& SyncEntry)
{
	if (!SyncEntry.IsValid())
	{
		return 0;
	}
	return GetTypeHash(*SyncEntry);
}


TSharedPtr<FEasySyncEntry> FEasySyncEntry::Create(TArray<FEasySyncKey>&& InKeys, FDelegateType&& InDelegate,
                                                  bool bInOnlyOnce)
{
	auto SyncEntry = MakeShared<FEasySyncEntry>();
	SyncEntry->SetKeys(MoveTemp(InKeys));
	SyncEntry->SetDelegate(MoveTemp(InDelegate));
	SyncEntry->SetOnlyOnce(bInOnlyOnce);
	return SyncEntry;
}

void FEasySyncEntry::SetKeys(TArray<FEasySyncKey>&& InKeys)
{
	Keys.Reserve(InKeys.Num());
	for (FEasySyncKey& InKey : InKeys)
	{
		Keys.Add(MakeShared<FEasySyncKey>(MoveTemp(InKey)));
	}
}

TArray<TWeakPtr<FEasySyncKey>> FEasySyncEntry::GetKeys() const
{
	TArray<TWeakPtr<FEasySyncKey>> ToRet;
	ToRet.Reserve(Keys.Num());

	for (auto& Key : Keys)
	{
		ToRet.Add(Key);
	}

	return ToRet;
}

bool FEasySyncEntry::IsAllKeyPassed() const
{
	for (const auto& Key : Keys)
	{
		if (!Key->IsPassed()) return false;
	}
	return true;
}

void FEasySyncEntry::ClearPasses()
{
	for (auto& Key : Keys)
	{
		Key->ClearPass();
	}
}

void FEasySyncEntry::MarkKeyAsPassed(const TSharedRef<FEasySyncKey>& InSyncKey)
{
	for (auto& SyncKey : Keys)
	{
		if (SyncKey == InSyncKey)
		{
			SyncKey->MarkPassed();
		}
	}
}

void FEasySyncEntry::SetDelegate(FDelegateType&& InDelegate)
{
	Delegate = MoveTemp(InDelegate);
}

bool FEasySyncEntry::Execute()
{
	if (const auto PureDelegate = std::get_if<FEasySyncDelegate>(&Delegate))
	{
		return PureDelegate->ExecuteIfBound();
	}
	else if (const auto DynamicDelegate = std::get_if<FEasySyncDynamicDelegate>(&Delegate))
	{
		return DynamicDelegate->ExecuteIfBound();
	}
	return false;
}

bool FEasySyncEntry::operator==(const FEasySyncEntry& SyncKey) const
{
	return GetTypeHash(this) == GetTypeHash(SyncKey);
}

uint32 GetTypeHash(const FEasySyncEntry& SyncEntry)
{
	uint32 Hash = GetTypeHash(SyncEntry.Keys);

	if (const auto PureDelegate = std::get_if<FEasySyncDelegate>(&SyncEntry.Delegate))
	{
		Hash = HashCombine(Hash, GetTypeHash(*PureDelegate));
	}
	else if (const auto DynamicDelegate = std::get_if<FEasySyncDynamicDelegate>(&SyncEntry.Delegate))
	{
		Hash = HashCombine(Hash, GetTypeHash(*DynamicDelegate));
	}

	return Hash;
}

uint32 GetTypeHash(const TSharedPtr<FEasySyncEntry>& SyncEntry)
{
	if (!SyncEntry.IsValid())
	{
		return 0;
	}
	return GetTypeHash(*SyncEntry);
}


void UEasySyncSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	for (int i = 1; i < 10; ++i)
	{
		SyncTags.Add(i, TMultiMap<FGameplayTag, TWeakPtr<FEasySyncKey>>());
	}
}

TStatId UEasySyncSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTK_SyncSubsystem, STATGROUP_Tickables);
}

UEasySyncSubsystem* UEasySyncSubsystem::Get(const UObject* const ContextObject)
{
	verify(GEngine);
	verifyf(IsInGameThread(), TEXT("UEasySyncSubsystem can bu used only in GameThread"));

	const auto World = GEngine->GetWorldFromContextObject(ContextObject, EGetWorldErrorMode::Assert);

	return World->GetSubsystem<UEasySyncSubsystem>();
}

void UEasySyncSubsystem::Broadcast(UObject* Sender, const FGameplayTag& Tag, FEasyBroadcastParams Params)
{
	auto PrevParams = std::exchange(LastParams, Params);
	LastParams.Sender = Sender;
	BroadcastTag(Tag);
	BroadcastTagWithOwner(Sender, Tag);
	LastParams = MoveTemp(PrevParams);
}

void UEasySyncSubsystem::WaitFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncEntry::FDelegateType&& SyncDelegate)
{
	auto Entry = FEasySyncEntry::Create(MoveTemp(SyncKeys), MoveTemp(SyncDelegate), true);
	RegisterEntry(MoveTemp(Entry));
}

void UEasySyncSubsystem::SubscribeFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncEntry::FDelegateType&& SyncDelegate)
{
	auto Entry = FEasySyncEntry::Create(MoveTemp(SyncKeys), MoveTemp(SyncDelegate), false);
	RegisterEntry(MoveTemp(Entry));
}

void UEasySyncSubsystem::RegisterEntry(TSharedPtr<FEasySyncEntry> SyncEntry)
{
	if (!SyncEntry) return;

	if (EntriesSet.Contains(SyncEntry))
	{
		UE_LOG(LogTemp, Warning, TEXT("Entry already registered in Subsystem"));
		return;
	}

	EntriesSet.Add(SyncEntry);

	for (const auto& WeakSyncKey : SyncEntry->GetKeys())
	{
		const auto SyncKey = WeakSyncKey.Pin();
		if (!SyncKey) continue;

		SyncKey->SetSyncEntry(SyncEntry);

		if (const auto& ConditionHandlerPtr = SyncKey->GetValueAsCondition())
		{
			const auto ConditionHandler = ConditionHandlerPtr->Get();
			Hash_SyncConditions.Add(GetTypeHash(*ConditionHandler), WeakSyncKey);
			Class_SyncConditions.Add(ConditionHandler->ConditionClass, WeakSyncKey);

			switch (ConditionHandler->ConditionStatus())
			{
			case EEasySyncConditionStatus::Pass:
				SyncEntry->MarkKeyAsPassed(SyncKey.ToSharedRef());
				break;
			case EEasySyncConditionStatus::Wait:
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("TODO"));
				return;
			}
		}
		else if (const auto& TagKeyPtr = SyncKey->GetValueAsTag())
		{
			SyncTags[TagKeyPtr->GetGameplayTagParents().Num()].Add(*TagKeyPtr, WeakSyncKey);
		}
		else if (const auto& OwnedTagKeyPtr = SyncKey->GetValueAsOwnedTag())
		{
			Hash_OwnedSyncTags.Add(GetTypeHash(*OwnedTagKeyPtr), WeakSyncKey);
		}
	}

	if (/** bRemove */ TryExecuteEntry(SyncEntry))
	{
		RemoveEntry(MoveTemp(SyncEntry));
	}
}

void UEasySyncSubsystem::RemoveEntry(TSharedPtr<FEasySyncEntry>&& SyncEntry)
{
	if (!SyncEntry) return;

	if (!EntriesSet.Contains(SyncEntry))
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't remove Entry because it's not added in Subsystem"));
		return;
	}

	for (const auto& WeakSyncKey : SyncEntry->GetKeys())
	{
		const auto SyncKey = WeakSyncKey.Pin();

		if (const auto& ConditionHandlerPtr = SyncKey->GetValueAsCondition())
		{
			const auto ConditionHandler = ConditionHandlerPtr->Get();

			Hash_SyncConditions.RemoveSingle(GetTypeHash(*ConditionHandler), WeakSyncKey);
			Class_SyncConditions.RemoveSingle(ConditionHandler->ConditionClass, WeakSyncKey);
		}
		else if (const auto& TagKeyPtr = SyncKey->GetValueAsTag())
		{
			SyncTags[TagKeyPtr->GetGameplayTagParents().Num()].RemoveSingle(*TagKeyPtr, WeakSyncKey);
		}
		else if (const auto& OwnedTagKeyPtr = SyncKey->GetValueAsOwnedTag())
		{
			Hash_OwnedSyncTags.Remove(GetTypeHash(&OwnedTagKeyPtr), WeakSyncKey);
		}
	}

	EntriesSet.Remove(SyncEntry);
}

void UEasySyncSubsystem::BroadcastTag(const FGameplayTag& Tag)
{
	const auto TagParents = Tag.GetGameplayTagParents();
	const int32 LevelNum = TagParents.Num();

	for (const auto& SubTag : TagParents)
	{
		const auto SyncKeys = GetSyncKeysByTag(SubTag);
		for (const auto& WeakSyncKey : SyncKeys)
		{
			const auto TagSyncKey = WeakSyncKey.Pin();
			if (!TagSyncKey || TagSyncKey->IsPassed()) continue;

			auto SyncEntry = TagSyncKey->GetSyncEntry();
			if (!SyncEntry)
			{
				UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
				continue;
			}

			SyncEntry->MarkKeyAsPassed(TagSyncKey.ToSharedRef());

			if (TryExecuteEntry(SyncEntry))
			{
				RemoveEntry(MoveTemp(SyncEntry));
			}
		}
	}
}

void UEasySyncSubsystem::BroadcastTagWithOwner(UObject* TagOwner, const FGameplayTag& Tag)
{
	TArray<TWeakPtr<FEasySyncKey>> WeakConditionOwnedSyncKeys;
	Hash_OwnedSyncTags.MultiFind(GetTypeHash(FOwnedGameplayTag(TagOwner, Tag)), WeakConditionOwnedSyncKeys);

	for (const auto& WeakSyncKey : WeakConditionOwnedSyncKeys)
	{
		const auto OwnedTagSyncKey = WeakSyncKey.Pin();
		if (!OwnedTagSyncKey) continue;

		if (!OwnedTagSyncKey || OwnedTagSyncKey->IsPassed()) continue;

		auto SyncEntry = OwnedTagSyncKey->GetSyncEntry();
		if (!SyncEntry)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
			continue;
		}

		SyncEntry->MarkKeyAsPassed(OwnedTagSyncKey.ToSharedRef());

		if (TryExecuteEntry(SyncEntry))
		{
			RemoveEntry(MoveTemp(SyncEntry));
		}
	}
}

void UEasySyncSubsystem::BroadcastConditionByHint(uint32 Hash)
{
	TArray<TWeakPtr<FEasySyncKey>> WeakConditionSyncKeys;
	Hash_SyncConditions.MultiFind(Hash, WeakConditionSyncKeys);

	for (const auto& WeakConditionSyncKey : WeakConditionSyncKeys)
	{
		const auto ConditionSyncKey = WeakConditionSyncKey.Pin();
		if (!ConditionSyncKey || ConditionSyncKey->IsPassed()) continue;

		auto SyncEntry = ConditionSyncKey->GetSyncEntry();
		if (!SyncEntry)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
			continue;
		}

		const auto Condition = ConditionSyncKey->GetValueAsCondition();
		if (!Condition) continue;

		const auto ConditionStatus = (*Condition)->ConditionStatus();
		if (ConditionStatus == EEasySyncConditionStatus::Pass)
		{
			SyncEntry->MarkKeyAsPassed(ConditionSyncKey.ToSharedRef());

			if (TryExecuteEntry(SyncEntry))
			{
				RemoveEntry(MoveTemp(SyncEntry));
			}
		}
	}
}

void UEasySyncSubsystem::BroadcastConditionNoHint(const TSubclassOf<UEasySyncBaseCondition>& ConditionClass)
{
	TArray<TWeakPtr<FEasySyncKey>*> WeakConditionSyncKeys;
	Class_SyncConditions.MultiFindPointer(ConditionClass, WeakConditionSyncKeys);

	for (const auto& WeakConditionSyncKey : WeakConditionSyncKeys)
	{
		const auto ConditionSyncKey = WeakConditionSyncKey->Pin();
		if (!ConditionSyncKey || ConditionSyncKey->IsPassed()) continue;

		auto SyncEntry = ConditionSyncKey->GetSyncEntry();
		if (!SyncEntry)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
			continue;
		}

		const auto Condition = ConditionSyncKey->GetValueAsCondition();
		if (!Condition) continue;

		const auto ConditionStatus = (*Condition)->ConditionStatus();
		if (ConditionStatus == EEasySyncConditionStatus::Pass)
		{
			SyncEntry->MarkKeyAsPassed(ConditionSyncKey.ToSharedRef());

			if (TryExecuteEntry(SyncEntry))
			{
				RemoveEntry(MoveTemp(SyncEntry));
			}
		}
	}
}

TArray<TWeakPtr<FEasySyncKey>> UEasySyncSubsystem::GetSyncKeysByTag(const FGameplayTag& Tag)
{
	TArray<TWeakPtr<FEasySyncKey>> ToRet;
	const auto TagLevelNum = Tag.GetGameplayTagParents().Num();

	SyncTags[TagLevelNum].MultiFind(Tag, ToRet);

	return ToRet;
}

bool/** bRemove */ UEasySyncSubsystem::TryExecuteEntry(const TSharedPtr<FEasySyncEntry>& Entry)
{
	if (Entry->IsAllKeyPassed())
	{
		if (!Entry->Execute()) return true;
		if (Entry->OnlyOnce()) return true;
	}
	return false;
}
