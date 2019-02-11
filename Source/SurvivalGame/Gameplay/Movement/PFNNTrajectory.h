// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ThirdParty/glm/glm.hpp"
#include "PFNNTrajectory.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNNTrajectory : public UObject
{
	GENERATED_BODY()

public:
	UPFNNTrajectory();
	~UPFNNTrajectory();
	
	enum { LENGTH = 120};

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PFNNTrajectory)
	glm::vec3 TargetDirection;
	//UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	glm::vec3 TargetVelocity;

	//Extra smoothing values
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float Width;
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float ExtraVelocitySmooth;
	UPROPERTY(EditAnywhere, Category = PFNNTrajectory)
	float ExtraStrafeVelocity;

	//Enviorment data
	//UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	glm::vec3 Positions[LENGTH];
	//UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
	glm::vec3 Directions[LENGTH];
	//UPROPERTY(VisibleAnywhere, Category = PFNNTrajectory)
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
