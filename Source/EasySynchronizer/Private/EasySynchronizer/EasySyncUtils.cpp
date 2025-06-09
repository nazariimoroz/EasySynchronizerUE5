// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySynchronizer/EasySyncUtils.h"

bool operator==(const FEasySyncDelegate& Left, const FEasySyncDelegate& Right)
{
	const auto LObject = Left.GetUObject();
	const auto LMethodName = Left.TryGetBoundFunctionName();
	if (!LObject || LMethodName.IsNone()) return false;

	const auto RObject = Right.GetUObject();
	const auto RMethodName = Right.TryGetBoundFunctionName();
	if (!RObject || RMethodName.IsNone()) return false;

	return LObject == RObject && LMethodName == RMethodName;
}
