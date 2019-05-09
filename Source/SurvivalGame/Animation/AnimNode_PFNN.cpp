// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "AnimNode_PFNN.h"
#include "PFNNAnimInstance.h"
#include "Gameplay/Movement/TrajectoryComponent.h"

#include "AnimInstanceProxy.h"
#include "MachineLearning/PhaseFunctionNeuralNetwork.h"
#include "PlatformFilemanager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <ThirdParty/glm/gtx/transform.inl>

UPhaseFunctionNeuralNetwork* FAnimNode_PFNN::PFNN = nullptr;

FAnimNode_PFNN::FAnimNode_PFNN(): Trajectory(nullptr), PFNNAnimInstance(nullptr), bIsPFNNLoaded(false)
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

void FAnimNode_PFNN::LoadPFNN()
{
	if((PFNN == nullptr || !bIsPFNNLoaded) && Trajectory)
	{
		PFNN = NewObject<UPhaseFunctionNeuralNetwork>();
		bIsPFNNLoaded = PFNN->LoadNetworkData(Trajectory->GetOwner());
	}
}

void FAnimNode_PFNN::ApplyPFNN()
{
	auto RootPosition = Trajectory->GetRootPosition();
	auto RootRotation = Trajectory->GetRootRotation();

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
	const glm::vec3 PreviousRootPosition = Trajectory->GetPreviousRootPosition();
	const glm::mat3 PreviousRootRotation = Trajectory->GetRootRotation();

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

	//Input heights for the trajectory
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		const int o = (((UTrajectoryComponent::LENGTH) / 10)*10) + JOINT_NUM * 3 * 2;
		const int w = UTrajectoryComponent::LENGTH / 10;

		const glm::vec3 PositionRight	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(Trajectory->Width, 0, 0));
		const glm::vec3 PositionLeft	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(-Trajectory->Width, 0, 0));

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, Trajectory->GetOwner());
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResultLeft(ForceInit);
		FHitResult HitResultRight(ForceInit);

		const FVector UPositionRight = FVector(PositionRight.x, PositionRight.y, PositionRight.z);
		const FVector UPositionLeft = FVector(PositionLeft.x, PositionLeft.y, PositionLeft.z);

		Trajectory->GetOwner()->GetWorld()->LineTraceSingleByChannel(HitResultRight, UPositionLeft, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);
		Trajectory->GetOwner()->GetWorld()->LineTraceSingleByChannel(HitResultLeft, UPositionRight, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);

		//TODO: Add height addition
		PFNN->Xp(o + (w * 0) + i / 10) = /*HitResultRight.Location.Z*/ 0 - RootPosition.z;
		PFNN->Xp(o + (w * 1) + i / 10) = Trajectory->Positions[i].z; /*- RootPosition.z*/
		PFNN->Xp(o + (w * 2) + i / 10) = /*HitResultLeft.Location.Z-*/ 0 - RootPosition.z;
	}

	//Preform regression
	PFNN->Predict(Phase);

	//Build local transformation for the joints
	for (int i = 0; i < JOINT_NUM; i++)
	{
		const int OPosition = 8 + (((UTrajectoryComponent::LENGTH / 2) / 10) * 4) + JOINT_NUM * 3 * 0;
		const int OVelocity = 8 + (((UTrajectoryComponent::LENGTH / 2) / 10) * 4) + JOINT_NUM * 3 * 1;
		const int ORoation = 8 + (((UTrajectoryComponent::LENGTH / 2) / 10) * 4) + JOINT_NUM * 3 * 2;

		//Flipped Y and Z
		const glm::vec3 Position = RootRotation * glm::vec3(PFNN->Yp(OPosition + i * 3 + 0), PFNN->Yp(OPosition + i * 3 + 2), PFNN->Yp(OPosition + i * 3 + 1)) + RootPosition;
		const glm::vec3 Velocity = RootRotation * glm::vec3(PFNN->Yp(OVelocity + i * 3 + 0), PFNN->Yp(OVelocity + i * 3 + 2), PFNN->Yp(OVelocity + i * 3 + 1));
		const glm::mat3 Rotation = RootRotation * glm::toMat3(QuaternionExpression(glm::vec3(PFNN->Yp(ORoation + i * 3 + 0), PFNN->Yp(ORoation + i * 3 + 2), PFNN->Yp(ORoation + i * 3 + 1))));

		//TODO: 0.8f should be replaced by ExtraJointSmooth
		JointPosition[i] = glm::mix(JointPosition[i] + Velocity, Position, 0.8f);
		JointVelocitys[i] = Velocity;
		JointRotations[i] = Rotation;

		JointGlobalAnimXform[i] = glm::transpose(glm::mat4(
			Rotation[0][0], Rotation[1][0], Rotation[2][0], Position[0],
			Rotation[0][1], Rotation[1][1], Rotation[2][1], Position[1],
			Rotation[0][2], Rotation[1][2], Rotation[2][2], Position[2],
			0, 0, 0, 1));
	}

	for (int i = 0; i < JOINT_NUM; i++)
	{
		if (i == 0)
		{
			JointAnimXform[i] = JointGlobalAnimXform[i];
		}
		else
		{
			JointAnimXform[i] = glm::inverse(JointGlobalAnimXform[JointParents[i]]) * JointGlobalAnimXform[i];
		}
	}

	//Forward kinematics
	for (int i = 0; i < JOINT_NUM; i++)
	{
		JointGlobalAnimXform[i] = JointAnimXform[i];
		JointGlobalRestXform[i] = JointRestXform[i];
		int j = JointParents[i];
		while (j != -1)
		{
			JointGlobalAnimXform[i] = JointAnimXform[j] * JointGlobalAnimXform[i];
			JointGlobalRestXform[i] = JointRestXform[j] * JointGlobalRestXform[i];
			j = JointParents[j];
		}
		JointMeshXform[i] = JointGlobalAnimXform[i] * glm::inverse(JointGlobalRestXform[i]);
	}

	//Update past trajectory
	for (int i = 0; i < UTrajectoryComponent::LENGTH / 2; i++)
	{
		Trajectory->Positions[i] = Trajectory->Positions[i + 1];
		Trajectory->Directions[i] = Trajectory->Directions[i + 1];
		Trajectory->Rotations[i] = Trajectory->Rotations[i + 1];
		Trajectory->Heights[i] = Trajectory->Heights[i + 1];
		Trajectory->GaitStand[i] = Trajectory->GaitStand[i + 1];
		Trajectory->GaitWalk[i] = Trajectory->GaitWalk[i + 1];
		Trajectory->GaitJog[i] = Trajectory->GaitJog[i + 1];
		Trajectory->GaitBump[i] = Trajectory->GaitBump[i + 1];
	}

	//Update current trajectory
	float StandAmount = powf(1.0f - Trajectory->GaitStand[UTrajectoryComponent::LENGTH / 2], 0.25f);

	const glm::vec3 TrajectoryUpdate = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] * glm::vec3(PFNN->Yp(0), PFNN->Yp(1), 0.0f); //TODEBUG: Rot
	Trajectory->Positions[UTrajectoryComponent::LENGTH / 2] = Trajectory->Positions[UTrajectoryComponent::LENGTH / 2] + StandAmount * TrajectoryUpdate;
	Trajectory->Directions[UTrajectoryComponent::LENGTH / 2] = glm::mat3(glm::rotate(StandAmount * -PFNN->Yp(2), glm::vec3(0, 0, 1))) * Trajectory->Directions[UTrajectoryComponent::LENGTH / 2]; //TODEBUG: Rot
	Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] = glm::mat3(glm::rotate(glm::atan(
		Trajectory->Directions[UTrajectoryComponent::LENGTH / 2].y,
		Trajectory->Directions[UTrajectoryComponent::LENGTH / 2].x), glm::vec3(0, 0, 1)));

	//TODO: Add wall logic

	//Update future trajectory
	for (int i = UTrajectoryComponent::LENGTH / 2 + 1; i < UTrajectoryComponent::LENGTH; i++)
	{
		//int i = UTrajectoryComponent::LENGTH - 1;
		const int W = (UTrajectoryComponent::LENGTH / 2) / 10;
		const float M = fmod((static_cast<float>(i) - (UTrajectoryComponent::LENGTH / 2)) / 10, 1.0f);
		//Trajectory->Positions[i].x = (glm::normalize(TrajectoryTargetDirectionNew).x);
		//Trajectory->Positions[i].y = (glm::normalize(TrajectoryTargetDirectionNew).y);
		//Trajectory->Directions[i].x = TrajectoryTargetDirectionNew.x;
		//Trajectory->Directions[i].y = TrajectoryTargetDirectionNew.y;

		Trajectory->Positions[i].x = (1 - M) * PFNN->Yp(8 + (W * 0) + (i / 10) - W) + M * PFNN->Yp(8 + (W * 0) + (i / 10) - W + 1); //TODEBUG: Rot
		Trajectory->Positions[i].y = (1 - M) * PFNN->Yp(8 + (W * 1) + (i / 10) - W) + M * PFNN->Yp(8 + (W * 1) + (i / 10) - W + 1); //TODEBUG: Rot
		Trajectory->Directions[i].x = (1 - M) * PFNN->Yp(8 + (W * 2) + (i / 10) - W) + M * PFNN->Yp(8 + (W * 2) + (i / 10) - W + 1); //TODEBUG: Rot
		Trajectory->Directions[i].y = (1 - M) * PFNN->Yp(8 + (W * 3) + (i / 10) - W) + M * PFNN->Yp(8 + (W * 3) + (i / 10) - W + 1); //TODEBUG: Rot

		Trajectory->Positions[i] = (Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] * Trajectory->Positions[i]) + Trajectory->Positions[UTrajectoryComponent::LENGTH / 2];
		Trajectory->Directions[i] = glm::normalize((Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2] * Trajectory->Directions[i]));
		Trajectory->Rotations[i] = glm::mat3(glm::rotate(atan2f(Trajectory->Directions[i].y, Trajectory->Directions[i].x), glm::vec3(0, 0, 1)));
	}


	FinalBoneLocations.Empty();
	FinalBoneRotations.Empty();

	FinalBoneLocations.SetNum(JOINT_NUM);
	FinalBoneRotations.SetNum(JOINT_NUM);
	const float FinalScale = 1.0f;

	for (int32 i = 0; i < JOINT_NUM; i++)
	{
		FinalBoneLocations[i] = FVector(JointPosition[i].x, JointPosition[i].y, JointPosition[i].z);

		FMatrix UMatrix;
		UMatrix.M[0][0] = JointMeshXform[i][0][0];	UMatrix.M[1][0] = JointMeshXform[i][1][0];	UMatrix.M[2][0] = JointMeshXform[i][2][0];	UMatrix.M[3][0] = JointMeshXform[i][3][0];
		UMatrix.M[0][1] = JointMeshXform[i][0][1];	UMatrix.M[1][1] = JointMeshXform[i][1][1];	UMatrix.M[2][1] = JointMeshXform[i][2][1];	UMatrix.M[3][1] = JointMeshXform[i][3][1];
		UMatrix.M[0][2] = JointMeshXform[i][0][2];	UMatrix.M[1][2] = JointMeshXform[i][1][2];	UMatrix.M[2][2] = JointMeshXform[i][2][2];	UMatrix.M[3][2] = JointMeshXform[i][3][2];
		UMatrix.M[0][3] = 0;						UMatrix.M[1][3] = 0;						UMatrix.M[2][3] = 0;						UMatrix.M[3][3] = 1;

		const glm::quat Rotation = JointRotations[i];
		FinalBoneRotations[i] = FQuat(Rotation.x, Rotation.z, Rotation.y, Rotation.w);
	}

	//Phase update
	Phase = fmod(Phase + (StandAmount * 0.9f + 0.1f) * 2.0f * PI * PFNN->Yp(3), 2.0f * PI);
}

glm::quat FAnimNode_PFNN::QuaternionExpression(const glm::vec3 arg_Length)
{
	float W = glm::length(arg_Length);

	const glm::quat Quat = W < 0.01 ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f) : glm::quat(
		cosf(W),
		arg_Length.x * (sinf(W) / W),
		arg_Length.y * (sinf(W) / W),
		arg_Length.z * (sinf(W) / W));

	return Quat / sqrtf(Quat.w*Quat.w + Quat.x*Quat.x + Quat.y*Quat.y + Quat.z*Quat.z);
}

UPFNNAnimInstance * FAnimNode_PFNN::GetPFNNInstanceFromContext(const FAnimationInitializeContext& Context)
{
	FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy;
	if (AnimProxy)
	{
		return Cast<UPFNNAnimInstance>(AnimProxy->GetAnimInstanceObject());
	}
	return nullptr;
}

UPFNNAnimInstance * FAnimNode_PFNN::GetPFNNInstanceFromContext(const FAnimationUpdateContext & Context)
{
	FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy;
	if (AnimProxy)
	{
		return Cast<UPFNNAnimInstance>(AnimProxy->GetAnimInstanceObject());
	}
	return nullptr;
}

void FAnimNode_PFNN::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	
	EvaluateGraphExposedInputs.Execute(Context);

	PFNNAnimInstance = GetPFNNInstanceFromContext(Context);
	if (!PFNNAnimInstance) 
	{
		UE_LOG(LogTemp, Error, TEXT("PFNN Animation node should only be added to a PFNNAnimInstance child class!"));
	}
}

void FAnimNode_PFNN::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	FAnimNode_Base::Update_AnyThread(Context);

	if (!bIsPFNNLoaded) 
	{
		LoadData();
	}

	EvaluateGraphExposedInputs.Execute(Context);

	if (PFNNAnimInstance) 
	{
		Trajectory = PFNNAnimInstance->GetOwningTrajectoryComponent();
	}
	
	if(Trajectory != nullptr && bIsPFNNLoaded)
		ApplyPFNN();
}

void FAnimNode_PFNN::Evaluate_AnyThread(FPoseContext& Output)
{
	const FTransform& CharacterTransform = Output.AnimInstanceProxy->GetActorTransform();
	if (FinalBoneLocations.Num() >= JOINT_NUM && FinalBoneRotations.Num() >= JOINT_NUM) 
	{
		for (int32 i = 0; i < JOINT_NUM; i++)
		{
			const FCompactPoseBoneIndex RootBoneIndex(i);

			Output.Pose[RootBoneIndex].SetLocation(FinalBoneLocations[i]);
			Output.Pose[RootBoneIndex].SetRotation(FinalBoneRotations[i]);

			Output.AnimInstanceProxy->AnimDrawDebugSphere(Output.Pose[RootBoneIndex].GetLocation() + CharacterTransform.GetLocation(), 2.5f, 12, FColor::Green, false, -1.0f);
		}
		Output.Pose.NormalizeRotations();
	}
	else 
	{
		UE_LOG(PFNN_Logging, Error, TEXT("PFNN results were not properly applied!"));
	}
}
