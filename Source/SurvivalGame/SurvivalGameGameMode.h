// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalGameGameMode.generated.h"

UCLASS(minimalapi)
class ASurvivalGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivalGameGameMode();

    void BeginPlay();

    void Tick(float deltaSeconds);

};



