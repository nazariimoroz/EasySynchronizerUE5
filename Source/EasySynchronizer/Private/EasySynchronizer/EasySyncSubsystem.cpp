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

FEasySyncKey::FEasySyncKey(const FValueType& InValue)
{
	Value = InValue;
}

uint32 GetTypeHash(const FEasySyncKey& SyncKey)
{
	if (const auto SyncCondition = std::get_if<TSharedPtr<FEasySyncConditionHandler>>(&SyncKey.Value))
	{
		return GetTypeHash(*SyncCondition);
	}
	if (const auto Tag = std::get_if<FGameplayTag>(&SyncKey.Value))
	{
		return GetTypeHash(Tag);
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
		if (!Key->bPassed) return false;
	}
	return true;
}

void FEasySyncEntry::ClearPasses()
{
	for (auto& Key : Keys)
	{
		Key->bPassed = false;
	}
}

void FEasySyncEntry::MarkKeyAsPassed(const TSharedRef<FEasySyncKey>& InSyncKey)
{
	for (auto& SyncKey : Keys)
	{
		if (SyncKey == InSyncKey)
		{
			SyncKey->bPassed = true;
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
		Hash = HashCombine(Hash, GetTypeHash(PureDelegate));
	}
	else if (const auto DynamicDelegate = std::get_if<FEasySyncDynamicDelegate>(&SyncEntry.Delegate))
	{
		Hash = HashCombine(Hash, GetTypeHash(DynamicDelegate));
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
	auto PrevParams = MoveTemp(LastParams);

	LastParams = Params;
	BroadcastTag(LastParams.Tag);
	LastParams = MoveTemp(PrevParams);
}

void UEasySyncSubsystem::WaitFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncDelegate&& SyncDelegate)
{
	auto Entry = FEasySyncEntry::Create(MoveTemp(SyncKeys), MoveTemp(SyncDelegate), true);
	RegisterEntry(MoveTemp(Entry));
}

void UEasySyncSubsystem::SubscribeFor(TArray<FEasySyncKey>&& SyncKeys, FEasySyncDelegate&& SyncDelegate)
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

	for (auto& WeakSyncKey : SyncEntry->GetKeys())
	{
		const auto SyncKey = WeakSyncKey.Pin();

		if (const auto& ConditionHandlerPtr = std::get_if<TSharedPtr<FEasySyncConditionHandler>>(&SyncKey->Value))
		{
			const auto ConditionHandler = ConditionHandlerPtr->Get();
			Hash_SyncConditions.Add(GetTypeHash(*ConditionHandler), WeakSyncKey);
			Class_SyncConditions.Add(ConditionHandler->ConditionClass, WeakSyncKey);

			switch (ConditionHandler->ConditionStatus())
			{
			case EEasySyncConditionStatus::Pass:
				SyncKey->bPassed = true;
				break;
			case EEasySyncConditionStatus::Wait:
				SyncKey->bPassed = false;
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("TODO"));
				return;
			}

		}
		else if (const auto& TagKeyPtr = std::get_if<FGameplayTag>(&SyncKey->Value))
		{
			SyncTags[TagKeyPtr->GetGameplayTagParents().Num()].Add(*TagKeyPtr, WeakSyncKey);
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

	for (auto& WeakSyncKey : SyncEntry->GetKeys())
	{
		const auto SyncKey = WeakSyncKey.Pin();

		if (const auto& ConditionHandlerPtr = std::get_if<TSharedPtr<FEasySyncConditionHandler>>(&SyncKey->Value))
		{
			const auto ConditionHandler = ConditionHandlerPtr->Get();

			Hash_SyncConditions.Remove(GetTypeHash(*ConditionHandler), WeakSyncKey);
			Class_SyncConditions.Remove(ConditionHandler->ConditionClass, WeakSyncKey);
		}
		else if (const auto& TagKeyPtr = std::get_if<FGameplayTag>(&SyncKey->Value))
		{
			SyncTags[TagKeyPtr->GetGameplayTagParents().Num()].RemoveSingle(*TagKeyPtr, WeakSyncKey);
		}
	}

	EntriesSet.Remove(SyncEntry);
}

void UEasySyncSubsystem::BroadcastTag(const FGameplayTag& Tag)
{
	const auto TagParents = Tag.GetGameplayTagParents();
	const int32 LevelNum = TagParents.Num();
	if (LevelNum < 1 || LevelNum > 4)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't broadcast Tag %s because it's on unknown level"), *Tag.ToString());
		return;
	}

	for (const auto& SubTag : TagParents)
	{
		const auto SynkKeys = GetSyncKeysByTag(SubTag);
		for (const auto WeakSyncKey : SynkKeys)
		{
			const auto TagSyncKey = WeakSyncKey.Pin();
			if (!TagSyncKey)
			{
				UE_LOG(LogTemp, Error, TEXT("SyncKey is not valid anymore. It is very uncommon situation"));
				continue;
			}
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

void UEasySyncSubsystem::BroadcastConditionByHint(uint32 Hash)
{
	TArray<TWeakPtr<FEasySyncKey>*> WeakConditionSyncKeys;
	Hash_SyncConditions.MultiFindPointer(Hash, WeakConditionSyncKeys);

	for (const auto& WeakConditionSyncKey : WeakConditionSyncKeys)
	{
		if (!WeakConditionSyncKey) continue;

		const auto ConditionSyncKey = WeakConditionSyncKey->Pin();
		if (!ConditionSyncKey)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncKey is not valid anymore. It is very uncommon situation"));
			continue;
		}
		auto SyncEntry = ConditionSyncKey->GetSyncEntry();
		if (!SyncEntry)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
			continue;
		}

		SyncEntry->MarkKeyAsPassed(ConditionSyncKey.ToSharedRef());

		if (TryExecuteEntry(SyncEntry))
		{
			RemoveEntry(MoveTemp(SyncEntry));
		}
	}
}

void UEasySyncSubsystem::BroadcastConditionNoHint(const TSubclassOf<UEasySyncBaseCondition>& ConditionClass)
{
	TArray<TWeakPtr<FEasySyncKey>*> WeakConditionSyncKeys;
	Class_SyncConditions.MultiFindPointer(ConditionClass, WeakConditionSyncKeys);

	for (const auto& WeakConditionSyncKey : WeakConditionSyncKeys)
	{
		if (!WeakConditionSyncKey) continue;

		const auto ConditionSyncKey = WeakConditionSyncKey->Pin();
		if (!ConditionSyncKey)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncKey is not valid anymore. It is very uncommon situation"));
			continue;
		}
		auto SyncEntry = ConditionSyncKey->GetSyncEntry();
		if (!SyncEntry)
		{
			UE_LOG(LogTemp, Error, TEXT("SyncEntry is not valid anymore. It is very uncommon situation"));
			continue;
		}

		SyncEntry->MarkKeyAsPassed(ConditionSyncKey.ToSharedRef());

		if (TryExecuteEntry(SyncEntry))
		{
			RemoveEntry(MoveTemp(SyncEntry));
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
