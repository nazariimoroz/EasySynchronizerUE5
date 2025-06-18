// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasySyncUtils.generated.h"

/** Delegates */

using FEasySyncDelegate = TDelegate<void(), FNotThreadSafeNotCheckedDelegateUserPolicy>;
uint32 GetTypeHash(const FEasySyncDelegate& Key);

DECLARE_DYNAMIC_DELEGATE(FEasySyncDynamicDelegate);


UENUM(BlueprintType)
enum class EEasySyncConditionStatus : uint8
{
	Pass,
	Wait,
	Error
};

namespace EasySync
{
	template<class T>
	constexpr int TTemplateFalse = false;
}

UCLASS(Abstract)
class UTK_Base_TODELATE : public UObject {GENERATED_BODY() };
