// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "AnimNode_PFNN.h"
#include "PFNN_Trajectory.h"
#include "AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "Engine.h"

FAnimNode_PFNN::FAnimNode_PFNN(): GaitStand(0), GaitWalk(0), GaitJog(0), GaitJump(0), GaitBump(0), ExtraGaitSmooth(0)
{

}

void FAnimNode_PFNN::ApplyPFNN(FPoseContext& arg_LocalPoseContext)
{
	const FBoneContainer& RequiredBones = arg_LocalPoseContext.Pose.GetBoneContainer();

	FCSPose<FCompactPose> GlobalPose;
	GlobalPose.InitPose(arg_LocalPoseContext.Pose);

}

void FAnimNode_PFNN::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	if(Trajectory == nullptr)
		UE_LOG(LogTemp, Error, TEXT("Trajectory is not set in a PFNN animation node."));
	

	BasePose.Initialize(Context);
	//UE_LOG(LogTemp, Warning, TEXT("Initalize PFNN node"));
}

void FAnimNode_PFNN::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	//UE_LOG(LogTemp, Warning, TEXT("Update PFNN node"));
	FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy;
	FTransform transform = AnimProxy->GetSkelMeshCompOwnerTransform();

	BasePose.Update(Context);
}

void FAnimNode_PFNN::Evaluate_AnyThread(FPoseContext& Output)
{

	for (int32 i = 0; i < JOINT_NUM; i++)
	{
		const FCompactPoseBoneIndex RootBoneIndex(i);
		Output.Pose[RootBoneIndex].SetRotation(Output.Pose[RootBoneIndex].GetRotation() + FQuat(FQuat::MakeFromEuler(FVector(5.0f))));
		Output.Pose[RootBoneIndex].NormalizeRotation();

	}

	BasePose.Evaluate(Output);
}
