// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Player/BaseCharacter.h"
#include "TestCharacter.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ATestCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//UPROPERTY()


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
