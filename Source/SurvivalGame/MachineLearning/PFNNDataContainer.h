// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "PhaseFunctionNeuralNetwork.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PFNNDataContainer.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNNDataContainer : public UObject
{
	GENERATED_BODY()

public:

	UPFNNDataContainer(const FObjectInitializer& arg_ObjectInitializer);

	void LoadData(EPFNNMode arg_Mode);

	bool IsDataLoaded();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	bool bIsDataLoaded;

};
