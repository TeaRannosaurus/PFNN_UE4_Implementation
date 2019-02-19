// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNN_PosableMesh.h"
#include "Components/SkeletalMeshComponent.h"

FSkeletonData::FSkeletonData() : Scale(1)
{

}

bool FSkeletonData::HasEmptyValues() const
{
	if(BonePositions.Num() <= 0 || BonePositions.Num() <= 0 || Scale == 0)
	{
		return true;
	}

	return false;
}

UPFNN_PosableMesh::UPFNN_PosableMesh()
{
	//MeshObject = CreateDefaultSubobject<FSkeletalMeshObject>(TEXT("MeshObject"));
}

UPFNN_PosableMesh::UPFNN_PosableMesh(FSkeletalMeshObject* arg_SkeletalMesh)
{
	MeshObject = arg_SkeletalMesh;
}

UPFNN_PosableMesh::~UPFNN_PosableMesh()
{
	
}

void UPFNN_PosableMesh::SetSkeletalMeshObject(USkeletalMeshComponent* NewMesh)
{
	//SkeletonData.BonePositions = TArray<FVector>();
	//SkeletonData.BoneRotations = TArray<FQuat>();
	//SkeletonData.Scale = 1;
	//SkeletalMesh = NewMesh->SkeletalMesh;
	//AllocateTransformData();
}

void UPFNN_PosableMesh::SetSkeletonData(TArray<FVector> arg_Location, TArray<FQuat> arg_Rotation, float arg_Scale)
{
	SkeletonData.BonePositions = arg_Location;
	SkeletonData.BoneRotations = arg_Rotation;
	SkeletonData.Scale		   = arg_Scale;
}


void UPFNN_PosableMesh::TickPose(float DeltaTime, bool bNeedsValidRootMotion)
{	
	Super::TickPose(DeltaTime, bNeedsValidRootMotion);

	if (SkeletonData.HasEmptyValues())
		return;

	const int32 NumBones = 31;

	for(int32 i = 0; i < NumBones; i++)
	{
		const FName BoneName = GetBoneName(i);

		const FTransform BoneTransform(SkeletonData.BoneRotations[i], SkeletonData.BonePositions[i], FVector(SkeletonData.Scale));
		SetBoneTransformByName(BoneName, BoneTransform, EBoneSpaces::WorldSpace);
	}
}
