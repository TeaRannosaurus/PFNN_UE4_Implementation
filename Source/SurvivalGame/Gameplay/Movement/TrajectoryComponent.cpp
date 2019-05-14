// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "TrajectoryComponent.h"

#include "Gameplay/Player/BaseCharacter.h"

#include <ThirdParty/glm/gtx/transform.inl>
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include <ThirdParty/glm/ext/quaternion_trigonometric.inl>
#include <ThirdParty/glm/ext/quaternion_common.inl>
#include <ThirdParty/glm/detail/type_quat.hpp>

#include <fstream>

#if !UE_BUILD_SHIPPING //Debug functions are excluded from the shipping build
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#endif

UTrajectoryComponent::UTrajectoryComponent(): ExtraStrafeSmooth(0), ExtraGaitSmooth(0), ExtraJointSmooth(0)
{
	PrimaryComponentTick.bCanEverTick = true;

	Width = 25.0f;
	TargetDirection = glm::vec3(0,0,1);
	ExtraVelocitySmooth	= 0.9f;
	ExtraStrafeVelocity	= 0.9f;
	ExtraDirectionSmooth= 0.9f;
	ExtraStrafeSmooth	= 0.9f;
	ExtraGaitSmooth		= 0.1f;
	ExtraJointSmooth	= 0.5f;

	StrafeTarget= 0.0f;
	StrafeAmount= 0.0f;
	Responsive	= 0.0f;


	for (int i = 0; i < LENGTH; i++)
	{
		Positions[i] = glm::vec3(0, 0, 0);
		Rotations[i] = glm::mat4(1);
		Directions[i] = glm::vec3(0, 0, 1);
		Heights[i]	= 0.0f;
		GaitJog[i]	= 0.0f;
		GaitWalk[i] = 0.0f;
		GaitBump[i] = 0.0f;
		GaitJump[i] = 0.0f;
		GaitStand[i]= 0.0f;
	}

	CurrentFrameInput = glm::vec2(0);

	OwnerPawn = nullptr;

#if WITH_EDITORONLY_DATA
	bShowDebugInformation = true;
#endif
}

UTrajectoryComponent::~UTrajectoryComponent()
{
}


void UTrajectoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	IsValid(OwnerPawn);

	tickcounter = 0;
}

glm::vec3 UTrajectoryComponent::GetRootPosition() const
{
	return glm::vec3(
		GetOwner()->GetActorLocation().X * 0.01f,
		GetOwner()->GetActorLocation().Y * 0.01f,
		Heights[LENGTH / 2] * 0.01f
	);
}

glm::vec3 UTrajectoryComponent::GetPreviousRootPosition() const
{
	return glm::vec3(
		Positions[LENGTH / 2 - 1].x,
		Positions[LENGTH / 2 - 1].y,
		Heights[LENGTH / 2 - 1]
	);
}

glm::mat3 UTrajectoryComponent::GetRootRotation() const
{
	return Rotations[LENGTH / 2];
}

glm::mat3 UTrajectoryComponent::GetPreviousRootRotation() const
{
	return Rotations[LENGTH / 2 - 1];
}

void UTrajectoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITORONLY_DATA
	if (bShowDebugInformation)
		DrawDebugTrajectory();
#endif

}

void UTrajectoryComponent::TickGaits()
{
	//Updating of the gaits
	const float MovementCutOff = 0.01f;
	const float JogCuttoff = 0.5f;
	const int Half = 2;
	const auto TrajectoryLength = glm::length(TargetVelocity);
	if (TrajectoryLength < MovementCutOff) //Standing
	{
		const float StandingClampMin = 0.0f;
		const float StandingClampMax = 1.0f;
		const float StandingAmount = 1.0f - glm::clamp(glm::length(TargetVelocity) / 0.1f, StandingClampMin, StandingClampMax);
		GaitStand[LENGTH / 2] = glm::mix(GaitStand[LENGTH / 2], StandingAmount, ExtraGaitSmooth);
		GaitWalk[LENGTH / 2] = glm::mix(GaitWalk[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitJog	[LENGTH / 2] = glm::mix(GaitJog	[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitJump[LENGTH / 2] = glm::mix(GaitJump[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitBump[LENGTH / 2] = glm::mix(GaitBump[LENGTH / 2], 0.0f, ExtraGaitSmooth);

	}
	else if (glm::abs(CurrentFrameInput.x) > JogCuttoff || glm::abs(CurrentFrameInput.y) > JogCuttoff) //Jog
	{

		GaitStand[LENGTH / 2] = glm::mix(GaitStand[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitWalk[LENGTH / 2] = glm::mix(GaitWalk[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitJog	[LENGTH / 2] = glm::mix(GaitJog	[LENGTH / 2], 1.0f, ExtraGaitSmooth);
		GaitJump[LENGTH / 2] = glm::mix(GaitJump[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitBump[LENGTH / 2] = glm::mix(GaitBump[LENGTH / 2], 0.0f, ExtraGaitSmooth);
	}
	else //Walking
	{
		GaitStand[LENGTH / 2] = glm::mix(GaitStand[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitWalk[LENGTH / 2] = glm::mix(GaitWalk[LENGTH / 2], 1.0f, ExtraGaitSmooth);
		GaitJog	[LENGTH / 2] = glm::mix(GaitJog	[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitJump[LENGTH / 2] = glm::mix(GaitJump[LENGTH / 2], 0.0f, ExtraGaitSmooth);
		GaitBump[LENGTH / 2] = glm::mix(GaitBump[LENGTH / 2], 0.0f, ExtraGaitSmooth);
	}
}

void UTrajectoryComponent::PredictFutureTrajectory()
{
	//Predicting future trajectory
	glm::vec3 TrajectoryPositionsBlend[LENGTH] = { glm::vec3(0) };
	TrajectoryPositionsBlend[LENGTH / 2] = Positions[LENGTH / 2];

	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{
		const float BiasPosition = Responsive ? glm::mix(2.0f, 2.0f, StrafeAmount) : glm::mix(0.5f, 1.0f, StrafeAmount);
		const float BiasDirection = Responsive ? glm::mix(5.0f, 3.0f, StrafeAmount) : glm::mix(2.0f, 0.5f, StrafeAmount);

		const float ScalePosition = (1.0f - powf(1.0f - (static_cast<float>(i - LENGTH / 2) / (LENGTH / 2)), BiasPosition));
		const float ScaleDirection = (1.0f - powf(1.0f - (static_cast<float>(i - LENGTH / 2) / (LENGTH / 2)), BiasDirection));

		TrajectoryPositionsBlend[i] = glm::mix(Positions[i] - Positions[i - 1], TargetVelocity, ScalePosition);

		//TODO: Add wall colision for future trajectory - 1519

		Directions[i] = MixDirections(Directions[i], TargetDirection, ScaleDirection);

		//Heights[i]	= Heights[LENGTH / 2];
		Heights[i] = 0; //Debug can be removed

		GaitStand[i]= GaitStand[LENGTH / 2];
		GaitWalk[i] = GaitWalk[LENGTH / 2];
		GaitJog[i]	= GaitJog [LENGTH / 2];
		GaitJump[i] = GaitJump[LENGTH / 2];
		GaitBump[i] = GaitBump[LENGTH / 2];
	}

	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{
		Positions[i] += TargetVelocity * TrajectoryPositionsBlend[i];
	}


}

void UTrajectoryComponent::TickRotations()
{
	for (int i = 0; i < LENGTH; i++)
	{
		Rotations[i] = glm::mat3(glm::rotate(atan2f(Directions[i].x, Directions[i].z), glm::vec3(0, 1, 0)));
	}
}

void UTrajectoryComponent::TickHeights()
{
	//Trajectory height
	for (int i = LENGTH / 2; i < LENGTH; i++)
	{
		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, GetOwner());
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResult(ForceInit);
		GetWorld()->LineTraceSingleByChannel(HitResult, GetOwner()->GetActorLocation(), -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);


		Positions[i].z = HitResult.Location.Z;
		Positions[i].z = 0; //DEBUG: removable
	}

	const int TrajectoryPointStepSize = 10;
	Heights[LENGTH / 2] = 0.0f;
	for (int i = 0; i < LENGTH; i += TrajectoryPointStepSize)
	{
		Heights[LENGTH / 2] += Positions[i].z / (LENGTH / TrajectoryPointStepSize);
	}
}

void UTrajectoryComponent::UpdatePastTrajectory()
{
	for (int i = 0; i < LENGTH / 2; i++)
	{
		Positions[i]	= Positions [i + 1];
		Directions[i]	= Directions[i + 1];
		Rotations[i]	= Rotations [i + 1];
		Heights[i]		= Heights	[i + 1];
		GaitStand[i]	= GaitStand [i + 1];
		GaitWalk[i]		= GaitWalk  [i + 1];
		GaitJog[i]		= GaitJog	[i + 1];
		GaitBump[i]		= GaitBump	[i + 1];
	}
}

glm::vec3 UTrajectoryComponent::MixDirections(const glm::vec3 arg_XDirection, const glm::vec3 arg_YDirection,
	const float arg_Scalar)
{
	const glm::quat XQuat = glm::angleAxis(atan2f(arg_XDirection.x, arg_XDirection.y), glm::vec3(0, 0, 1));
	const glm::quat YQuat = glm::angleAxis(atan2f(arg_YDirection.x, arg_YDirection.y), glm::vec3(0, 0, 1));
	const glm::quat ZQuat = glm::slerp(XQuat, YQuat, arg_Scalar);
	return ZQuat * glm::vec3(0, 1, 0);
}


//USES NATIVE CPP TO ENSURE IT CAN BE USED IN REFERENCE PROJECT
void UTrajectoryComponent::LogTrajectoryData(int arg_FrameCount)
{
	try 
	{
		std::ofstream fs;
		fs.open("UE4_Trajectory.log", std::ios::out);

		if (fs.is_open()) 
		{
			fs << "UE4_Implementation" << std::endl;
			fs << "TrajectoryLog Frame[" << arg_FrameCount << "]" << std::endl << std::endl;

			fs << "#Basic Variables" << std::endl;
			fs << "TargetDirection: " << TargetDirection.x << "X, " << TargetDirection.y << "Y, " << TargetDirection.z << "Z" << std::endl;
			fs << "TargetVelocity:  " << TargetVelocity.x << "X, " << TargetVelocity.y << "Y, " << TargetVelocity.z << "Z" << std::endl;
			fs << "Width:           " << Width << std::endl;
			fs << "StrafeAmount:    " << StrafeAmount << std::endl;
			fs << "StrafeTarget:    " << StrafeTarget << std::endl;
			fs << "Responsive:      " << Responsive << std::endl;
			fs << "#End Basic Variables" << std::endl << std::endl;

			fs << "#Extra Smoothing values" << std::endl;
			fs << "	ExtraVelocitySmooth:	" << ExtraVelocitySmooth << std::endl;
			fs << "	ExtraDirectionSmooth:   " << ExtraDirectionSmooth << std::endl;
			fs << "	ExtraStrafeVelocity:	" << ExtraStrafeVelocity << std::endl;
			fs << "	ExtraStrafeSmooth:      " << ExtraStrafeSmooth << std::endl;
			fs << "	ExtraGaitSmooth:        " << ExtraGaitSmooth << std::endl;
			fs << "	ExtraJointSmooth:       " << ExtraJointSmooth << std::endl;
			fs << "#End Extra Smoothing values" << std::endl << std::endl;

			fs << "#Positional Data" << std::endl;
			for (size_t i = 0; i < LENGTH; i++)
			{
				fs << "TrajectoryNode[" << i << "]" << std::endl;
				fs << " Position:  " << Positions[i].x << "X, " << Positions[i].y << "Y, " << Positions[i].z << "Z" << std::endl;
				fs << "	Direction: " << Directions[i].x << "X, " << Directions[i].y << "Y, " << Directions[i].z << "Z" << std::endl;
				for (size_t x = 0; x < 3; x++)
				{
				fs << "	Rotation:  " << Rotations[i][x].x << "X, " << Rotations[i][x].y << ", " << Rotations[i][x].z << std::endl;
				}
			}
			fs << "#End Positional Data" << std::endl << std::endl;

			fs << "#Gaits" << std::endl;
			for (size_t i = 0; i < LENGTH; i++)
			{
				fs << "Gait[" << i << "]" << std::endl;
				fs << "	GaitStand: " << GaitStand[i] << std::endl;
				fs << "	GaitWalk:  " << GaitWalk[i] << std::endl;
				fs << "	GaitJog:   " << GaitJog[i] << std::endl;
				fs << "	GaitJump:  " << GaitJump[i] << std::endl;
				fs << "	GaitBump:  " << GaitBump[i] << std::endl;
			}
			fs << "#End Gaits" << std::endl << std::endl;
		}
		else 
		{
			throw std::exception();
		}
		fs.close();
		return;
	}
	catch (std::exception e) 
	{
#ifdef WITH_EDITOR
		UE_LOG(LogTemp, Error, TEXT("Failed to Log Trajectory output data"));
#endif
	}
}

void UTrajectoryComponent::TickTrajectory()
{
	auto BaseCharacter = Cast<ABaseCharacter>(GetOwner());
	if (BaseCharacter) 
	{
		auto MovementComponent = BaseCharacter->GetMovementComponent();
		CurrentFrameInput = glm::vec2(MovementComponent->Velocity.X*0.01f, MovementComponent->Velocity.Y*0.01f);
	}

	if (glm::abs(CurrentFrameInput.x) + glm::abs(CurrentFrameInput.y) < 0.305f)
	{
		CurrentFrameInput = glm::vec2(0);
	}

	CurrentFrameInput = glm::normalize(CurrentFrameInput);

	glm::vec3 TrajectoryTargetDirectionNew = glm::normalize(glm::vec3(GetOwner()->GetActorForwardVector().X, 0.0f, GetOwner()->GetActorForwardVector().Y));
	const glm::mat3 TrajectoryTargetRotation = glm::mat3(glm::rotate(atan2f(
		TrajectoryTargetDirectionNew.x,
		TrajectoryTargetDirectionNew.z), glm::vec3(0, 1, 0)));

	float TargetVelocitySpeed = OwnerPawn->GetVelocity().SizeSquared() / (OwnerPawn->GetMovementComponent()->GetMaxSpeed() * OwnerPawn->GetMovementComponent()->GetMaxSpeed()) * 7.5f; //7.5 is current training walking speed

	const glm::vec3 TrajectoryTargetVelocityNew = TargetVelocitySpeed * (TrajectoryTargetRotation * glm::vec3(CurrentFrameInput.x, 0, CurrentFrameInput.y));
	TargetVelocity = glm::mix(TargetVelocity, TrajectoryTargetVelocityNew, ExtraVelocitySmooth);

	StrafeAmount = glm::mix(StrafeAmount, StrafeTarget, ExtraStrafeSmooth);
	const glm::vec3 TrajectoryTargetVelocityDirection = glm::length(TargetVelocity) < 1e-05 ? TargetDirection : glm::normalize(TargetVelocity);
	TrajectoryTargetDirectionNew = MixDirections(TrajectoryTargetVelocityDirection, TrajectoryTargetDirectionNew, StrafeAmount);
	TargetDirection = MixDirections(TargetDirection, TrajectoryTargetDirectionNew, ExtraDirectionSmooth);

	TickGaits();
	PredictFutureTrajectory();
	TickRotations();
	TickHeights();
}

#if !UE_BUILD_SHIPPING //Debug functions are excluded from the shipping build
void UTrajectoryComponent::DrawDebugTrajectory()
{
	const float MajorPointSize = 15.0f;
	const float MinorPointSize = 5.0f;
	const float PointOffset = 2.0f;

	const auto StartingPoint = Positions[0];		//Get the leading point of the trajectory
	const auto MidPoint = Positions[LENGTH / 2];	//Get the mid point/player point of the trajectory
	const auto EndingPoint = Positions[LENGTH - 1];	//Get the ending point of the trajectory

	for (size_t i = 0; i < LENGTH; i++)
	{
			FVector DebugLocation = FVector(Positions[i].x, Positions[i].z, Positions[i].y) + GetOwner()->GetActorLocation();
			DrawDebugPoint(GetWorld(), DebugLocation, 4.0f,FColor::Red);

			FVector Ue4Direction = FVector(Directions[i].x, Directions[i].z, Directions[i].y) * 10.0f;
			FVector DirectionLocation = DebugLocation + Ue4Direction;
			DrawDebugLine(GetWorld(), DebugLocation, DirectionLocation, FColor::Black, false, -1.0f, 0, 0.2f);
	}

	FVector DebugStartingPoint = FVector(StartingPoint.x, StartingPoint.z, StartingPoint.y) + GetOwner()->GetActorLocation();
	FVector DebugMidPoint = FVector(MidPoint.x, MidPoint.z, MidPoint.y) + GetOwner()->GetActorLocation();
	FVector DebugEndPoint = FVector(EndingPoint.x, EndingPoint.z, EndingPoint.y) + GetOwner()->GetActorLocation();

	DrawDebugPoint(GetWorld(), DebugStartingPoint, 10.0f, FColor::Red);
	DrawDebugPoint(GetWorld(), DebugMidPoint, 10.0f, FColor::Red);
	DrawDebugPoint(GetWorld(), DebugEndPoint, 10.0f, FColor::Red);

	DrawDebugString(GetWorld(), DebugStartingPoint, TEXT("Past"), nullptr, FColor::Blue, 0.001f);
	DrawDebugString(GetWorld(), DebugMidPoint, TEXT("Current"), nullptr, FColor::Blue, 0.001f);
	DrawDebugString(GetWorld(), DebugEndPoint, TEXT("Future"), nullptr, FColor::Blue, 0.001f);

}
#endif
