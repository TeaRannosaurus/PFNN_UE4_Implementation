// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Animation/PhaseFunctionNeuralNetwork.h"
#include "PFNN_SkeletalMeshComponent.generated.h"

//class PhaseFunctionNeuralNetwork;
class UPFNNTrajectory;
class UPFNN_PosableMesh;


/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNN_SkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:	
	UPFNN_SkeletalMeshComponent();
	~UPFNN_SkeletalMeshComponent();

	void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PFNN)
	UPFNNTrajectory* Trajectory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PFNN)
	UPFNN_PosableMesh* PFNNPosableMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PFNN)
	UPoseableMeshComponent* PosableMesh; 

	PhaseFunctionNeuralNetwork* PFNN;
};
