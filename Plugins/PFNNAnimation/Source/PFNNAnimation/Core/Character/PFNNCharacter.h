// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PFNNCharacter.generated.h"

UCLASS()
class PFNNANIMATION_API APFNNCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	// Sets default values for this character's properties
	APFNNCharacter();

	class UTrajectoryComponent* GetTrajectoryComponent();

	bool HasDebuggingEnabled();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	class UTrajectoryComponent* TrajectoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PFNNDebugging ,meta = (AllowPrivateAccess = "True"))
	bool bIsDebuggingEnabled;

};
