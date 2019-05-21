// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNCharacter.h"

#include "Animation/AnimComponents/TrajectoryComponent.h"

// Sets default values
APFNNCharacter::APFNNCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TrajectoryComponent = CreateDefaultSubobject<UTrajectoryComponent>(TEXT("TrajectoryComponent"));

	bIsDebuggingEnabled = false;

}

UTrajectoryComponent* APFNNCharacter::GetTrajectoryComponent()
{
	return TrajectoryComponent;
}

bool APFNNCharacter::HasDebuggingEnabled()
{
	return bIsDebuggingEnabled;
}
