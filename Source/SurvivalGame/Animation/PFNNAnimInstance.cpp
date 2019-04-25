// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNAnimInstance.h"

#include "Gameplay/Player/TestCharacter.h"

#include "DrawDebugHelpers.h"

void UPFNNAnimInstance::NativeInitializeAnimation() 
{
	Super::NativeInitializeAnimation();
	ATestCharacter* OwningCharacter = Cast<ATestCharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		OwningTrajectoryComponent = OwningCharacter->GetTrajectoryComponent();
	}

}

void UPFNNAnimInstance::NativeUpdateAnimation(float arg_DeltaTimeX)
{
	Super::NativeUpdateAnimation(arg_DeltaTimeX);

	ATestCharacter* OwningCharacter = Cast<ATestCharacter>(TryGetPawnOwner());
	if (OwningCharacter) 
	{
		OwningTrajectoryComponent = OwningCharacter->GetTrajectoryComponent();
	}
}

UTrajectoryComponent * UPFNNAnimInstance::GetOwningTrajectoryComponent()
{
	return OwningTrajectoryComponent;
}
