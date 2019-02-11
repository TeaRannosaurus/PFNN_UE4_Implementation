// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNNTrajectory.h"

UPFNNTrajectory::UPFNNTrajectory(): Width(25)
{
	TargetDirection = glm::vec3(0);
	ExtraVelocitySmooth = 0.9f;
	ExtraStrafeVelocity = 0.9f;

	for(int i = 0; i < LENGTH; i++)
	{
		Positions[i] = glm::vec3(0, 0, 0);
		Rotations[i] = glm::mat4(0);
		Directions[i]= glm::vec3(0, 1, 0);
		Heights[i]	 = 0.0f;
		GaitJog[i]	 = 0.0f;
		GaitWalk[i]	 = 0.0f;
		GaitBump[i]	 = 0.0f;
		GaitJump[i]	 = 0.0f;
		GaitStand[i] = 0.0f;
	}
}

UPFNNTrajectory::~UPFNNTrajectory()
{
}
