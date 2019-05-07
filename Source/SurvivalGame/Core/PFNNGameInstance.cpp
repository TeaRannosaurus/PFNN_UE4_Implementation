// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNGameInstance.h"

#include "MachineLearning/PFNNDataContainer.h"

UPFNNGameInstance::UPFNNGameInstance(const FObjectInitializer& arg_ObjectInitializer) : Super(arg_ObjectInitializer) 
{
	PFNNDataContainer = NewObject<UPFNNDataContainer>(this, UPFNNDataContainer::StaticClass(), TEXT("PFNN DataContainer"));
}