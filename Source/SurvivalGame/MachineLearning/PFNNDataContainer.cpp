// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNDataContainer.h"

UPFNNDataContainer::UPFNNDataContainer(const FObjectInitializer& arg_ObjectInitializer) : Super(arg_ObjectInitializer), bIsDataLoaded(false) 
{

}

void UPFNNDataContainer::LoadData(EPFNNMode arg_Mode) 
{

}

bool UPFNNDataContainer::IsDataLoaded()
{
	return bIsDataLoaded;
}
