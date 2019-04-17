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

	const glm::vec3 RootPosition = glm::vec3(
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2].x + Trajectory->GetOwner()->GetActorLocation().X,
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2].y + Trajectory->GetOwner()->GetActorLocation().Y,
		/*Trajectory->Heights[LENGTH / Half]*/0.0f);

	//const glm::vec3 RootPosition = glm::vec3(
	//	 GetActorLocation().X,
	//	 GetActorLocation().Y,
	//	/*Trajectory->Heights[LENGTH / Half]*/0.0f);


	const glm::mat3 RootRotation = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2];

	//Input trajectiory positions and directions
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		int w = UTrajectoryComponent::LENGTH / 10;
		const glm::vec3 Position = glm::inverse((RootRotation)) * (Trajectory->Positions[i] - RootPosition);
		const glm::vec3 Direction = glm::inverse((RootRotation)) * Trajectory->Directions[i];
		PFNN->Xp((w * 0) + i / 10) = Position.x;
		PFNN->Xp((w * 1) + i / 10) = Position.y;
		PFNN->Xp((w * 2) + i / 10) = Direction.x;
		PFNN->Xp((w * 3) + i / 10) = Direction.y;
	}

	// Input trajectory gaits
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		const int w = UTrajectoryComponent::LENGTH / 10;
		PFNN->Xp((w * 4) + i / 10) = Trajectory->GaitStand[i];
		PFNN->Xp((w * 5) + i / 10) = Trajectory->GaitWalk[i];
		PFNN->Xp((w * 6) + i / 10) = Trajectory->GaitJog[i];
		PFNN->Xp((w * 7) + i / 10) = 0; //Unused input for crouch?;
		PFNN->Xp((w * 8) + i / 10) = Trajectory->GaitJump[i];
		PFNN->Xp((w * 9) + i / 10) = 0; //Unused input
	}

	//Input previous join position / velocity / rotations
	const glm::vec3 PreviousRootPosition = glm::vec3(
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2 - 1].x,
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2 - 1].y,
		/*Trajectory->Heights[UTrajectoryComponent::LENGTH / 2 - 1]*/0.0f);
	const glm::mat3 PreviousRootRotation = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2 - 1];
	for (int i = 0; i < JOINT_NUM; i++)
	{
		const int o = (((UTrajectoryComponent::LENGTH) / 10)*10);
		const glm::vec3 Position = glm::inverse(PreviousRootRotation) * (JointPosition[i] - PreviousRootPosition);
		const glm::vec3 Previous = glm::inverse(PreviousRootRotation) * JointVelocitys[i];
		//Magical numbers are indexes for the PFNN
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 0) = Position.x;
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 1) = Position.z;
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 2) = Position.y;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 0) = Previous.x;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 1) = Previous.z;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 2) = Previous.y;
	}
}

void FAnimNode_PFNN::PredictFutureTrajectory(UTrajectoryComponent* arg_Trajectory)
{
	//Predicting future trajectory
	//TO VALIDATE: Calculations might be incorrect
	glm::vec3 TrajectoryPositionsBlend[UTrajectoryComponent::LENGTH];
	TrajectoryPositionsBlend[UTrajectoryComponent::LENGTH / 2] = Trajectory->Positions[UTrajectoryComponent::LENGTH / 2];
	for (int i = UTrajectoryComponent::LENGTH / 2 + 1; i < UTrajectoryComponent::LENGTH; i++)
	{
		const float BiasPosition = Trajectory->Responsive ? glm::mix(2.0f, 2.0f, Trajectory->StrafeAmount) : glm::mix(0.5f, 1.0f, Trajectory->StrafeAmount);
		const float BiasDirection = Trajectory->Responsive ? glm::mix(5.0f, 3.0f, Trajectory->StrafeAmount) : glm::mix(2.0f, 0.5f, Trajectory->StrafeAmount);

		const float ScalePosition = (1.0f - powf(1.0f - (static_cast<float>(i - UTrajectoryComponent::LENGTH / 2) / (UTrajectoryComponent::LENGTH / 2)), BiasPosition));
		const float ScaleDirection = (1.0f - powf(1.0f - (static_cast<float>(i - UTrajectoryComponent::LENGTH / 2) / (UTrajectoryComponent::LENGTH / 2)), BiasDirection));

		//TrajectoryPositionsBlend[i] = glm::mix(Trajectory->Positions[i] - Trajectory->Positions[i - 1], Trajectory->TargetVelocity * DeltaSeconds, ScalePosition);
		TrajectoryPositionsBlend[i] = glm::mix(Trajectory->Positions[i] - Trajectory->Positions[i - 1], Trajectory->TargetVelocity, ScalePosition); //Delta seconds missing

		//TODO: Add wall colision for future trajectory - 1519

		Trajectory->Directions[i] = glm::mix(Trajectory->Directions[i], Trajectory->TargetDirection, ScaleDirection);

		//Trajectory->Heights[i]	= Trajectory->Heights[UTrajectoryComponent::LENGTH / 2];
		Trajectory->Heights[i] = 0; //Debug can be removed

		Trajectory->GaitStand[i] = Trajectory->GaitStand[UTrajectoryComponent::LENGTH / 2];
		Trajectory->GaitWalk[i] = Trajectory->GaitWalk[UTrajectoryComponent::LENGTH / 2];
		Trajectory->GaitJog[i] = Trajectory->GaitJog[UTrajectoryComponent::LENGTH / 2];
		Trajectory->GaitJump[i] = Trajectory->GaitJump[UTrajectoryComponent::LENGTH / 2];
		Trajectory->GaitBump[i] = Trajectory->GaitBump[UTrajectoryComponent::LENGTH / 2];
	}
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


	for(int32 i = 0; i < JOINT_NUM; i++)
	{
		const FCompactPoseBoneIndex index(i);

		UE_LOG(LogTemp, Error, TEXT("Evaluate"));

		const FCompactPoseBoneIndex RootBoneIndex(i);
		Output.Pose[RootBoneIndex].SetRotation(Output.Pose[RootBoneIndex].GetRotation());
		Output.Pose[RootBoneIndex].NormalizeRotation();
	}





	BasePose.Evaluate(Output);
}
