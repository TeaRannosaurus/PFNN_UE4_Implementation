// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ThirdParty/glm/glm.hpp"
#include "PFNN_Trajectory.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNN_Trajectory : public UObject
{
	GENERATED_BODY()
	
public:
	UPFNN_Trajectory();
	~UPFNN_Trajectory();

	enum { LENGTH = 120 };	//Length defined in header so it can be used before compilation

	glm::vec3 TargetDirection;
	glm::vec3 TargetVelocity;

	//Extra smoothing values
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float Width;
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float ExtraVelocitySmooth;
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float ExtraStrafeVelocity;

	//Enviorment data
	glm::vec3 Positions[LENGTH];
	glm::vec3 Directions[LENGTH];
	glm::mat3 Rotations[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float Heights[LENGTH];

	//Gaits
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float GaitStand[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float GaitWalk[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float GaitJog[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float GaitJump[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	float GaitBump[LENGTH];
};
