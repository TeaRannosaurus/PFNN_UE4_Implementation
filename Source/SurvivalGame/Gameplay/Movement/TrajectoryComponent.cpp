// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "TrajectoryComponent.h"

UTrajectoryComponent::UTrajectoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	Width = 25.0f;
	TargetDirection = glm::vec3(0);
	ExtraVelocitySmooth = 0.9f;
	ExtraStrafeVelocity = 0.9f;

	for (int i = 0; i < LENGTH; i++)
	{
		Positions[i] = glm::vec3(0, 0, 0);
		Rotations[i] = glm::mat4(0);
		Directions[i] = glm::vec3(0, 1, 0);
		Heights[i] = 0.0f;
		GaitJog[i] = 0.0f;
		GaitWalk[i] = 0.0f;
		GaitBump[i] = 0.0f;
		GaitJump[i] = 0.0f;
		GaitStand[i] = 0.0f;
	}
}

UTrajectoryComponent::~UTrajectoryComponent()
{
}


void UTrajectoryComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UTrajectoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

