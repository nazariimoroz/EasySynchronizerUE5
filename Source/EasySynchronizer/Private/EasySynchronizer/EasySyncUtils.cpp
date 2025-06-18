// Fill out your copyright notice in the Description page of Project Settings.

#include "EasySynchronizer/EasySyncUtils.h"

uint32 GetTypeHash(const FEasySyncDelegate& Key)
{
	return FCrc::MemCrc_DEPRECATED(&Key,sizeof(Key));
}
