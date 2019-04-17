// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "TrajectoryComponent.h"
#include <ThirdParty/glm/gtx/transform.inl>
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include <ThirdParty/glm/ext/quaternion_trigonometric.inl>
#include <ThirdParty/glm/ext/quaternion_common.inl>
#include <ThirdParty/glm/detail/type_quat.hpp>

#if !UE_BUILD_SHIPPING //Debug functions are excluded from the shipping build
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#endif

UTrajectoryComponent::UTrajectoryComponent(): ExtraStrafeSmooth(0), ExtraGaitSmooth(0), ExtraJointSmooth(0)
{
	PrimaryComponentTick.bCanEverTick = true;

	Width = 25.0f;
	TargetDirection = glm::vec3(0);
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
		Rotations[i] = glm::mat4(0);
		Directions[i] = glm::vec3(0, 1, 0);
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
	bShowDebugInformation = false;
#endif
}

UTrajectoryComponent::~UTrajectoryComponent()
{
}


void UTrajectoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = static_cast<APawn*>(GetOwner());
	IsValid(OwnerPawn);
}

void UTrajectoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (glm::abs(CurrentFrameInput.x) + glm::abs(CurrentFrameInput.y) < 0.305f)
	{
		CurrentFrameInput = glm::vec2(0);
	}

	glm::vec3 TrajectoryTargetDirectionNew = glm::normalize(glm::vec3(GetOwner()->GetActorForwardVector().X, GetOwner()->GetActorForwardVector().Y, 0.0f));
	const glm::mat3 TrajectoryTargetRotation = glm::mat3(glm::rotate(atan2f(
		TrajectoryTargetDirectionNew.y,
		TrajectoryTargetDirectionNew.x), glm::vec3(0, 0, 1)));
	
	float TargetVelocitySpeed = OwnerPawn->GetVelocity().SizeSquared() / (OwnerPawn->GetMovementComponent()->GetMaxSpeed() * OwnerPawn->GetMovementComponent()->GetMaxSpeed()) * 7.5f; //7.5 is current training walking speed
	const glm::vec3 TrajectoryTargetVelocityNew = TargetVelocitySpeed * (TrajectoryTargetRotation * glm::vec3(1, 0, 0));
	TargetVelocity = glm::mix(TargetVelocity, TrajectoryTargetVelocityNew, ExtraVelocitySmooth);

	const glm::vec3 TrajectoryTargetVelocityDirection = glm::length(TargetVelocity) < 1e-05 ? TargetDirection : glm::normalize(TargetVelocity);
	TrajectoryTargetDirectionNew = MixDirections(TrajectoryTargetVelocityDirection, TrajectoryTargetDirectionNew, StrafeAmount);
	TargetDirection = MixDirections(TargetDirection, TrajectoryTargetDirectionNew, ExtraDirectionSmooth);
	//TargetDirection = TrajectoryTargetVelocityDirection;

	TickGaits();
	PredictFutureTrajectory(DeltaTime);
	TickRotations();
	TickHeights();

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

void UTrajectoryComponent::PredictFutureTrajectory(const float DeltaSeconds)
{
	//Predicting future trajectory
	glm::vec3 TrajectoryPositionsBlend[LENGTH];
	TrajectoryPositionsBlend[LENGTH / 2] = Positions[LENGTH / 2];

	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{
		const float BiasPosition = Responsive ? glm::mix(2.0f, 2.0f, StrafeAmount) : glm::mix(0.5f, 1.0f, StrafeAmount);
		const float BiasDirection = Responsive ? glm::mix(5.0f, 3.0f, StrafeAmount) : glm::mix(2.0f, 0.5f, StrafeAmount);

		const float ScalePosition = (1.0f - powf(1.0f - (static_cast<float>(i - LENGTH / 2) / (LENGTH / 2)), BiasPosition));
		const float ScaleDirection = (1.0f - powf(1.0f - (static_cast<float>(i - LENGTH / 2) / (LENGTH / 2)), BiasDirection));

		TrajectoryPositionsBlend[i] = glm::mix(Positions[i] - Positions[i - 1], TargetVelocity * DeltaSeconds, ScalePosition);

		//TODO: Add wall colision for future trajectory - 1519

		Directions[i] = glm::mix(Directions[i], TargetDirection, ScaleDirection);

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
		Rotations[i] = glm::mat3(glm::rotate(atan2f(Directions[i].y, Directions[i].x), glm::vec3(0, 0, 1)));
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
	const glm::quat XQuat = glm::angleAxis(atan2f(arg_XDirection.y, arg_XDirection.x), glm::vec3(0, 0, 1));
	const glm::quat YQuat = glm::angleAxis(atan2f(arg_YDirection.y, arg_YDirection.x), glm::vec3(0, 0, 1));
	const glm::quat ZQuat = glm::slerp(XQuat, YQuat, arg_Scalar);
	return ZQuat * glm::vec3(0, 1, 0);
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


	///*Red		End		-	Start*/	DrawDebugLine(GetWorld(), FVector(StartingPoint.x, StartingPoint.y, StartingPoint.z), FVector(EndingPoint.x, EndingPoint.y, EndingPoint.z), FColor::Red, false, -1, 0, 3);
	///*Green		Pos		-	End*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(StartingPoint.x, StartingPoint.y, StartingPoint.z), FColor::Blue, false, -1, 0, 3);
	///*Yellow	Pos		-	Mid*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(MidPoint.x, MidPoint.y, MidPoint.z), FColor::Yellow, false, -1, 0, 3);
	///*Blue		Pos		-	Start*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(EndingPoint.x, EndingPoint.y, EndingPoint.z), FColor::Green, false, -1, 0, 3);

	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Starting point = %f %f %f"), StartingPoint.x, StartingPoint.y, StartingPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Middle point = %f %f %f"), MidPoint.x, MidPoint.y, MidPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::Printf(TEXT("Ending point = %f %f %f"), EndingPoint.x, EndingPoint.y, EndingPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("Player point = %f %f %f"), GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z));

	for (int i = 0; i < LENGTH - 10; i++)	//Show minor points
	{
		const auto PointPosition = Positions[i];
		const auto PointColor = FColor(GaitStand[i] * 255, GaitJog[i] * 255, GaitWalk[i] * 255);

		//const auto UPointPoisiton = FVector(ScaleBetween(PointPosition.x, 0.0f, 7.5f) * 600.0f, ScaleBetween(PointPosition.y, 0.0f, 7.5f) * 600.0f + PointOffset, ScaleBetween(PointPosition.z, 0.0f, 7.5f) * 600.0f);
		const auto UPointPoisiton = FVector(PointPosition.x, PointPosition.y, PointOffset);

		DrawDebugPoint(GetWorld(), UPointPoisiton, MinorPointSize, PointColor, false, 0.03f);
	}

	for (int i = 0; i < LENGTH; i += 10) //Show major points
	{
		const auto PointPosition = Positions[i];
		const auto PointColor = FColor(GaitStand[i] * 255, GaitJog[i] * 255, GaitWalk[i] * 255);

		//const auto UPointPoisiton = FVector(ScaleBetween(PointPosition.x, 0.0f, 7.5f) * 600.0f, ScaleBetween(PointPosition.y, 0.0f, 7.5f) * 600.0f + PointOffset, ScaleBetween(PointPosition.z, 0.0f, 7.5f) * 600.0f);
		const auto UPointPoisiton = FVector(PointPosition.x, PointPosition.y, PointOffset);

		DrawDebugPoint(GetWorld(), UPointPoisiton, MajorPointSize, PointColor, false, 0.03f);
	}

	for (int i = 0; i < LENGTH; i += 10) //Show major hieghts
	{
		const auto PointPositionRight = Positions[i] + Rotations[i] * glm::vec3(Width, 0, 0);
		const auto PointPositionLeft = Positions[i] + Rotations[i] * glm::vec3(-Width, 0, 0);
		const auto PointColor = FColor(GaitStand[i] * 255, GaitJog[i] * 255, GaitWalk[i] * 255);


		DrawDebugPoint(GetWorld(), FVector(PointPositionRight.x, PointPositionRight.y, PointPositionRight.z + PointOffset), MinorPointSize, PointColor, false, 0.03f);
		DrawDebugPoint(GetWorld(), FVector(PointPositionLeft.x, PointPositionLeft.y, PointPositionLeft.z + PointOffset), MinorPointSize, PointColor, false, 0.03f);
	}

	for (int i = 0; i < LENGTH; i += 10) //Draw arrows
	{
		const float ArrowHeadLenth = 4.0f;

		auto Base = Positions[i] + glm::vec3(0.0f, PointOffset, 0.0f);
		auto Side = glm::normalize(glm::cross(Directions[i], glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 Forward = Base + 15.0f * Directions[i];
		const auto PointColor = FColor(GaitStand[i] * 255, GaitJog[i] * 255, GaitWalk[i] * 255);

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, GetOwner());
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResult(ForceInit);

		GetWorld()->LineTraceSingleByChannel(HitResult, FVector(Forward.x, Forward.y + PointOffset, Forward.z), -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);
		Forward.z = HitResult.Location.Z;
		Forward.z = 0; //DEBUG: Can be removed

		const glm::vec3 Arrow0 = Forward + ArrowHeadLenth * Side + ArrowHeadLenth * -Directions[i];
		const glm::vec3 Arrow1 = Forward - ArrowHeadLenth * Side + ArrowHeadLenth * -Directions[i];

		//const auto UArrow0 = FVector(Arrow0.x, Arrow0.y, Arrow0.z);
		const auto UArrow0 = FVector(Arrow0.x, Arrow0.y, 0);
		//const auto UArrow1 = FVector(Arrow1.x, Arrow1.y, Arrow1.z);
		const auto UArrow1 = FVector(Arrow1.x, Arrow1.y, 0);

		DrawDebugLine(GetWorld(), FVector(Forward.x, Forward.y, Forward.z), UArrow0, PointColor, false, -1, 0, 1);
		DrawDebugLine(GetWorld(), FVector(Forward.x, Forward.y, Forward.z), UArrow1, PointColor, false, -1, 0, 1);
		DrawDebugLine(GetWorld(), FVector(Base.x, Base.y, Base.z), FVector(Forward.x, Forward.y, Forward.z), PointColor, false, -1, 0, 1);


	}
}
#endif
