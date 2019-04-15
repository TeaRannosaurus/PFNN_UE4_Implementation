// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThirdParty/glm/glm.hpp"
#include "TrajectoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALGAME_API UTrajectoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrajectoryComponent();
	~UTrajectoryComponent();

	enum { LENGTH = 120 };

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

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

		
};
