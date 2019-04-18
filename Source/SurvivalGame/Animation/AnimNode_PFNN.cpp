// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "AnimNode_PFNN.h"
#include "PFNN_Trajectory.h"
#include "AnimInstanceProxy.h"
#include "Gameplay/Movement/TrajectoryComponent.h"
#include "MachineLearning/PhaseFunctionNeuralNetwork.h"
#include <ThirdParty/glm/gtx/transform.inl>
#include "PlatformFilemanager.h"

UPhaseFunctionNeuralNetwork* FAnimNode_PFNN::PFNN = nullptr;

FAnimNode_PFNN::FAnimNode_PFNN(): GaitStand(0), GaitWalk(0), GaitJog(0), GaitJump(0), GaitBump(0), ExtraGaitSmooth(0),
                                  PFNNMode(2)
{
}

void FAnimNode_PFNN::LoadData()
{
	LoadXForms();
	LoadPFNN();
}

void FAnimNode_PFNN::LoadXForms()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const FString RelativePath = FPaths::ProjectDir();
	const FString FullPathParents = RelativePath + FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/character_parents.bin"));
	const FString FullPathXforms = RelativePath + FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/character_xforms.bin"));

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPathParents);
	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load charater parents"));
		return;
	}
	float JointParentsFloat[JOINT_NUM];
	FileHandle->Read(reinterpret_cast<uint8*>(JointParentsFloat), sizeof(JointParentsFloat));

	for (int i = 0; i < JOINT_NUM; i++)
	{
		JointParents[i] = static_cast<int>(JointParentsFloat[i]);
	}


	FileHandle = PlatformFile.OpenRead(*FullPathXforms);
	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load character xforms"));
		return;
	}
	FileHandle->Read(reinterpret_cast<uint8*>(JointRestXform), sizeof(JointRestXform));

	for (int i = 0; i < JOINT_NUM; i++)
	{
		JointRestXform[i] = glm::transpose(JointRestXform[i]);
	}

	delete FileHandle;
}

void FAnimNode_PFNN::LoadPFNN() const
{
	if(PFNN == nullptr || PFNN->mode != PFNNMode)
	{
		PFNN = NewObject<UPhaseFunctionNeuralNetwork>();
		PFNN->mode = PFNNMode;
		PFNN->LoadNetworkData();
	}
}

void FAnimNode_PFNN::ApplyPFNN(FPoseContext& arg_LocalPoseContext)
{
	const FBoneContainer& RequiredBones = arg_LocalPoseContext.Pose.GetBoneContainer();

	FCSPose<FCompactPose> GlobalPose;
	GlobalPose.InitPose(arg_LocalPoseContext.Pose);
}

void FAnimNode_PFNN::PredictFutureTrajectory(UTrajectoryComponent* arg_Trajectory)
{
	//const float StandAmount = powf(1.0f - Trajectory->GaitStand[UTrajectoryComponent::LENGTH / 2], 0.25f);

	//const glm::vec3 TrajectoryUpdate = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] * glm::vec3(PFNN_SkeletalMesh->PFNN->Yp(0), PFNN_SkeletalMesh->PFNN->Yp(1), 0.0f); //TODEBUG: Rot
	//Trajectory->Positions[UTrajectoryComponent::LENGTH / 2] = Trajectory->Positions[UTrajectoryComponent::LENGTH / 2] + StandAmount * TrajectoryUpdate;
	//Trajectory->Directions[UTrajectoryComponent::LENGTH / 2] = glm::mat3(glm::rotate(StandAmount * -PFNN_SkeletalMesh->PFNN->Yp(2), glm::vec3(0, 0, 1))) * Trajectory->Directions[UTrajectoryComponent::LENGTH / 2]; //TODEBUG: Rot
	//Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] = glm::mat3(glm::rotate(glm::atan(
	//	Trajectory->Directions[UTrajectoryComponent::LENGTH / 2].y,
	//	Trajectory->Directions[UTrajectoryComponent::LENGTH / 2].x), glm::vec3(0, 0, 1)));
}

void FAnimNode_PFNN::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	if(Trajectory == nullptr)
		UE_LOG(LogTemp, Error, TEXT("Trajectory is not set in a PFNN animation node."));
	

	BasePose.Initialize(Context);
	LoadData();
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
	BasePose.Evaluate(Output);

	const FTransform& CharacterTransform = Output.AnimInstanceProxy->GetActorTransform();

	for (int32 i = 0; i < Output.Pose.GetNumBones(); i++)
	{
		const FCompactPoseBoneIndex RootBoneIndex(i);

		Output.Pose[RootBoneIndex].SetLocation(Output.Pose[RootBoneIndex].GetLocation());
		Output.Pose[RootBoneIndex].SetRotation(Output.Pose[RootBoneIndex].GetRotation());

		Output.AnimInstanceProxy->AnimDrawDebugSphere(Output.Pose[RootBoneIndex].GetLocation() + CharacterTransform.GetLocation(), 2.5f, 12, FColor::Green, false, -1.0f);
	}
	Output.Pose.NormalizeRotations();
}
