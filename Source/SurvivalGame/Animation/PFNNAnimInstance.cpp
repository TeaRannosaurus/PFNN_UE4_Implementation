// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNAnimInstance.h"

void UPFNNAnimInstance::NativeInitializeAnimation() 
{
	Super::NativeInitializeAnimation();

	SkeletalMesh = GetOwningComponent();
}

void UPFNNAnimInstance::NativeUpdateAnimation(float arg_DeltaTimeX)
{
	Super::NativeUpdateAnimation(arg_DeltaTimeX);

	if (!SkeletalMesh) 
	{
		UE_LOG(LogTemp, Error, TEXT("PFNNAnimationInstance does not have SkeletalMesh!"));
		return;
	}

	const FBoneContainer& Bones = GetRequiredBones();

}
