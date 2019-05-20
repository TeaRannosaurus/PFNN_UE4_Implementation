// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PFNNAnimation/Core/Character/PFNNCharacter.h"
#include "TestCharacter.generated.h"

USTRUCT(BlueprintType)
struct SURVIVALGAME_API FWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MakeEditWidget = "True"))
	FVector Location;
};

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ATestCharacter : public APFNNCharacter
{
	GENERATED_BODY()
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	ATestCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void MoveToWaypoint();

private:

	void MoveBetweenTargets();

	void IncrementCurrentIndex();

	void DebugWaypoints();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	int CurrentIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	float RequiredDistanceToWaypoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	TArray<FWaypoint> Waypoints;

};
