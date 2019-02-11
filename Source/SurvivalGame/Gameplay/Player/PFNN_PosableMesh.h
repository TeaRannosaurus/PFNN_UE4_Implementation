// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PoseableMeshComponent.h"
#include "PFNN_PosableMesh.generated.h"

USTRUCT()
struct FSkeletonData
{
	GENERATED_USTRUCT_BODY()

	FSkeletonData();

	//Location of bones
	//UPROPERTY(VisableAnywere)
	TArray<FVector> BonePositions;

	//Rotation of bones
	//UPROPERTY(VisableAnywere)
	TArray<FQuat> BoneRotations;

	//Scale of the skeletal mesh
	//UPROPERTY(VisableAnywere)
	float Scale;

	bool HasEmptyValues() const;
};

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNN_PosableMesh : public UPoseableMeshComponent
{
public:
	GENERATED_BODY()
	UPFNN_PosableMesh();
	UPFNN_PosableMesh(FSkeletalMeshObject* arg_SkeletalMesh);
	~UPFNN_PosableMesh();

	void SetSkeletalMeshObject(USkeletalMeshComponent* NewMesh);
	void SetSkeletonData(TArray<FVector> arg_Location, TArray<FQuat> arg_Rotation, float arg_Scale);
	void TickPose(float DeltaTime, bool bNeedsValidRootMotion) override;
	
private:
	FSkeletonData SkeletonData;

};
