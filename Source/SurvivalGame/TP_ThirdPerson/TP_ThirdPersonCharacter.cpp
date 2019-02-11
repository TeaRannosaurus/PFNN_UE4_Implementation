// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gameplay/Player/PFNN_SkeletalMeshComponent.h"
#include "Gameplay/Player/PFNN_PosableMesh.h"
#include "PlatformFilemanager.h"
#include "Gameplay/Movement/AnimationController/PhaseFunctionNeuralNetwork.h"
#include "Gameplay/Movement/PFNNTrajectory.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "ThirdParty/glm/glm.hpp"
#include "ThirdParty/glm/gtx/transform.hpp"
#include "ThirdParty/glm/gtx/quaternion.hpp"
#include "ThirdParty/glm/gtc/quaternion.hpp"
#include "ThirdParty/glm/gtx/rotate_vector.hpp"


//////////////////////////////////////////////////////////////////////////
// ATP_ThirdPersonCharacter

#define GLM_ENABLE_EXPERIMENTAL

ATP_ThirdPersonCharacter::ATP_ThirdPersonCharacter() : Phase(0.0f), ExtraDirectionSmooth(0.9f), StrafeAmount(0.0f)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Default smoothing values
	ExtraStrafeSmooth = 0.9f;
	ExtraGaitSmooth = 0.1f;
	ExtraJointSmooth = 0.5f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	PFNN_SkeletalMesh = CreateDefaultSubobject<UPFNN_SkeletalMeshComponent>(TEXT("PFNN controller"));
	//skeletalmesh
	//PFNN_SkeletalMesh->MeshObject ;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATP_ThirdPersonCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATP_ThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATP_ThirdPersonCharacter::MoveRight);
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATP_ThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATP_ThirdPersonCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATP_ThirdPersonCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATP_ThirdPersonCharacter::TouchStopped);
}

FVector ATP_ThirdPersonCharacter::MixVector(const FVector arg_XVector, FVector const arg_YVector, const float arg_Scalar)
{
	return arg_XVector * (1.0f - arg_Scalar) + arg_YVector * arg_Scalar;
}

float ATP_ThirdPersonCharacter::MixFloat(const float arg_XFloat, const float arg_YFloat, const float arg_Scalar)
{
	return arg_XFloat * (1.0f - arg_Scalar) + arg_YFloat * arg_Scalar;
}

glm::vec3 ATP_ThirdPersonCharacter::MixDirections(const glm::vec3 arg_XDirection, const glm::vec3 arg_YDirection,	const float arg_Scalar)
{
	//const glm::quat XQuat = glm::angleAxis(atan2f(arg_XDirection.y, arg_XDirection.x), glm::vec3(1, 0, 0));
	//const glm::quat YQuat = glm::angleAxis(atan2f(arg_YDirection.y, arg_YDirection.x), glm::vec3(1, 0, 0));
	//const glm::quat ZQuat = glm::slerp(XQuat, YQuat, arg_Scalar); 
	//return ZQuat * glm::vec3(0, 0, 1);
	return glm::vec3(0);
}

FQuat ATP_ThirdPersonCharacter::CreateQuaternionFromAngleAxis(const float arg_Angle, const FVector arg_Axis)
{
	auto s = sin(arg_Angle * 0.5f);
	return FQuat(arg_Axis.X, arg_Axis.Z, arg_Axis.Y, cos(arg_Angle * 0.5f));
}

FQuat ATP_ThirdPersonCharacter::QuaternionExpresion(const FVector arg_Length)
{
	const float w = arg_Length.Size();
	FQuat q;
	if (w < 0.01f)
	{
		q = FQuat(1, 0, 0, 0);
	}
	else
	{
		q = FQuat(FMath::Cos(w), arg_Length.X * (FMath::Sin(w) / w), arg_Length.Y * (FMath::Sin(w) / w), arg_Length.Z * (FMath::Sin(w) / w));
	}

	return q / FMath::Sqrt(q.W * q.W + q.X * q.X + q.Y * q.Y + q.Z * q.Z);
}

glm::quat ATP_ThirdPersonCharacter::QuaternionExpression(const glm::vec3 arg_Length)
{
	float W = glm::length(arg_Length);

	const glm::quat Quat = W < 0.01 ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f) : glm::quat(
		cosf(W), 
		arg_Length.x * (sinf(W) / W), 
		arg_Length.y * (sinf(W) / W), 
		arg_Length.z * (sinf(W) / W));

	return Quat / sqrtf(Quat.w*Quat.w + Quat.x*Quat.x + Quat.y*Quat.y + Quat.z*Quat.z);
}

/*FMatrix ATP_ThirdPersonCharacter::QuaternionToMatrix(const FQuat arg_Quad)
{
	FMatrix FinalMatrix;

	const auto Sqw = arg_Quad.W * arg_Quad.W;
	const auto Sqx = arg_Quad.X * arg_Quad.X;
	const auto Sqy = arg_Quad.Y * arg_Quad.Y;
	const auto Sqz = arg_Quad.Z * arg_Quad.Z;

	// Inverse square length is only required if quaternion is not already normalised
	const auto Inverse = 1 / (Sqx + Sqy + Sqz + Sqz);

	FinalMatrix.M[0][0] = ( Sqx - Sqy - Sqz + Sqw) * Inverse;
	FinalMatrix.M[1][1] = (-Sqx + Sqy - Sqz + Sqw) * Inverse;
	FinalMatrix.M[1][1] = (-Sqx - Sqy + Sqz + Sqw) * Inverse;
	
	auto Temp1 = arg_Quad.X * arg_Quad.Y;
	auto Temp2 = arg_Quad.Z * arg_Quad.W;
	FinalMatrix.M[1][0] = 2.0f * (Temp1 + Temp2) * Inverse;
	FinalMatrix.M[0][1] = 2.0f * (Temp1 - Temp2) * Inverse;

	Temp1 = arg_Quad.X * arg_Quad.Z;
	Temp2 = arg_Quad.Y * arg_Quad.W;
	FinalMatrix.M[2][0] = 2.0f * (Temp1 - Temp2) * Inverse;
	FinalMatrix.M[0][2] = 2.0f * (Temp1 + Temp2) * Inverse;

	Temp1 = arg_Quad.Y * arg_Quad.Z;
	Temp2 = arg_Quad.X * arg_Quad.W;
	FinalMatrix.M[2][1] = 2.0f * (Temp1 + Temp2) * Inverse;
	FinalMatrix.M[1][2] = 2.0f * (Temp1 - Temp2) * Inverse;

	return FinalMatrix;
}*/

FVector ATP_ThirdPersonCharacter::MatrixVectorMultiplication(const FMatrix arg_InMatrix, const FVector arg_InVector)
{
	FVector ResultVector;

	ResultVector[0] = arg_InMatrix.M[0][0] * arg_InVector.X + arg_InMatrix.M[1][0] * arg_InVector.Y + arg_InMatrix.M[2][0] * arg_InVector.Z;
	ResultVector[1] = arg_InMatrix.M[0][1] * arg_InVector.X + arg_InMatrix.M[1][1] * arg_InVector.Y + arg_InMatrix.M[2][1] * arg_InVector.Z;
	ResultVector[2] = arg_InMatrix.M[0][2] * arg_InVector.X + arg_InMatrix.M[1][2] * arg_InVector.Y + arg_InMatrix.M[2][2] * arg_InVector.Z;

	return ResultVector;
}

FMatrix ATP_ThirdPersonCharacter::VectorToMatrix(const FVector arg_InVector)
{
	FMatrix ResultMatrix = FMatrix::Identity;

	ResultMatrix.M[0][0] = arg_InVector.X;
	ResultMatrix.M[0][1] = arg_InVector.Z;
	ResultMatrix.M[0][2] = arg_InVector.Y;
	//ResultMatrix.M[0][3] = arg_InVector.X;
	ResultMatrix.M[1][0] = arg_InVector.X;
	ResultMatrix.M[1][1] = arg_InVector.Z;
	ResultMatrix.M[1][2] = arg_InVector.Y;
	//ResultMatrix.M[1][3] = ;
	ResultMatrix.M[2][0] = arg_InVector.X;
	ResultMatrix.M[2][1] = arg_InVector.Z;
	ResultMatrix.M[2][2] = arg_InVector.Y;
	//ResultMatrix.M[2][3] = 
	ResultMatrix.M[3][0] = 0;
	ResultMatrix.M[3][1] = 0;
	ResultMatrix.M[3][2] = 0;
	ResultMatrix.M[3][3] = 1;

	return ResultMatrix;
}

FMatrix ATP_ThirdPersonCharacter::RotateVectorOverAngle(const float arg_Angle, FVector arg_Vector)
{
	const float A = arg_Angle;
	const float C = FMath::Cos(A);
	const float S = FMath::Sin(C);

	arg_Vector.Normalize();

	const FVector Axis = arg_Vector;
	const FVector Temp = C * Axis;

	// ReSharper disable once CppLocalVariableMayBeConst
	FMatrix Rotate;

	Rotate.M[0][0] = C + Temp[0] * Axis[2];

	return Rotate;
}

float ATP_ThirdPersonCharacter::ScaleBetween(const float arg_Unscaled, const float arg_Min, const float arg_Max)
{
	return (arg_Unscaled - arg_Min) / (arg_Max - arg_Min);
}

void ATP_ThirdPersonCharacter::DrawDebugPoints()
{
	for(int i = 0; i < JOINT_NUM; i++)
	{
		DrawDebugPoint(GetWorld(), FVector(JointPosition[i].x, JointPosition[i].y, JointPosition[i].z), 10, FColor::Red, false, 0.03f);
	}

	DrawDebugTrajectory();
	//DrawDebugUI();
}

void ATP_ThirdPersonCharacter::DrawDebugTrajectory()
{
	const float MajorPointSize = 15.0f;
	const float MinorPointSize = 5.0f;
	const float PointOffset = 2.0f;

	const auto StartingPoint	= Trajectory->Positions[0];							//Get the leading point of the trajectory
	const auto MidPoint			= Trajectory->Positions[UPFNNTrajectory::LENGTH/2]; //Get the mid point/player point of the trajectory
	const auto EndingPoint		= Trajectory->Positions[UPFNNTrajectory::LENGTH-1];	//Get the ending point of the trajectory


	///*Red		End		-	Start*/	DrawDebugLine(GetWorld(), FVector(StartingPoint.x, StartingPoint.y, StartingPoint.z), FVector(EndingPoint.x, EndingPoint.y, EndingPoint.z), FColor::Red, false, -1, 0, 3);
	///*Green		Pos		-	End*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(StartingPoint.x, StartingPoint.y, StartingPoint.z), FColor::Blue, false, -1, 0, 3);
	///*Yellow	Pos		-	Mid*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(MidPoint.x, MidPoint.y, MidPoint.z), FColor::Yellow, false, -1, 0, 3);
	///*Blue		Pos		-	Start*/	DrawDebugLine(GetWorld(), GetActorLocation(), FVector(EndingPoint.x, EndingPoint.y, EndingPoint.z), FColor::Green, false, -1, 0, 3);

	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green,	FString::Printf(TEXT("Starting point = %f %f %f"), StartingPoint.x, StartingPoint.y, StartingPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow,	FString::Printf(TEXT("Middle point = %f %f %f"), MidPoint.x, MidPoint.y, MidPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue,	FString::Printf(TEXT("Ending point = %f %f %f"), EndingPoint.x, EndingPoint.y, EndingPoint.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red,	FString::Printf(TEXT("Player point = %f %f %f"), GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z));

	for(int i = 0; i < UPFNNTrajectory::LENGTH-10; i++)	//Show minor points
	{
		const auto PointPosition = Trajectory->Positions[i];
		const auto PointColor = FColor(Trajectory->GaitStand[i] * 255, Trajectory->GaitJog[i] * 255, Trajectory->GaitWalk[i] * 255);

		//const auto UPointPoisiton = FVector(ScaleBetween(PointPosition.x, 0.0f, 7.5f) * 600.0f, ScaleBetween(PointPosition.y, 0.0f, 7.5f) * 600.0f + PointOffset, ScaleBetween(PointPosition.z, 0.0f, 7.5f) * 600.0f);
		const auto UPointPoisiton = FVector(PointPosition.x, PointPosition.y, PointOffset);

		DrawDebugPoint(GetWorld(), UPointPoisiton, MinorPointSize, PointColor, false, 0.03f);
	}

	for(int i = 0; i < UPFNNTrajectory::LENGTH; i+=10) //Show major points
	{
		const auto PointPosition = Trajectory->Positions[i];
		const auto PointColor = FColor(Trajectory->GaitStand[i] * 255 , Trajectory->GaitJog[i] * 255, Trajectory->GaitWalk[i] * 255);

		//const auto UPointPoisiton = FVector(ScaleBetween(PointPosition.x, 0.0f, 7.5f) * 600.0f, ScaleBetween(PointPosition.y, 0.0f, 7.5f) * 600.0f + PointOffset, ScaleBetween(PointPosition.z, 0.0f, 7.5f) * 600.0f);
		const auto UPointPoisiton = FVector(PointPosition.x, PointPosition.y, PointOffset);

		DrawDebugPoint(GetWorld(), UPointPoisiton, MajorPointSize, PointColor, false, 0.03f);
	}

	for(int i = 0; i < UPFNNTrajectory::LENGTH; i+=10) //Show major hieghts
	{
		const auto PointPositionRight = Trajectory->Positions[i] + Trajectory->Rotations[i] * glm::vec3( Trajectory->Width, 0, 0);
		const auto PointPositionLeft = Trajectory->Positions[i] + Trajectory->Rotations[i] * glm::vec3(-Trajectory->Width, 0, 0);
		const auto PointColor = FColor(Trajectory->GaitStand[i] * 255, Trajectory->GaitJog[i] * 255, Trajectory->GaitWalk[i] * 255);


		DrawDebugPoint(GetWorld(), FVector(PointPositionRight.x,	PointPositionRight.y,	PointPositionRight.z + PointOffset),	MinorPointSize, PointColor, false, 0.03f);
		DrawDebugPoint(GetWorld(), FVector(PointPositionLeft.x,		PointPositionLeft.y,	PointPositionLeft.z + PointOffset),		MinorPointSize, PointColor, false, 0.03f);
	}

	for(int i = 0; i < UPFNNTrajectory::LENGTH; i+=10) //Draw arrows
	{
		const float ArrowHeadLenth = 4.0f;

		auto Base	= Trajectory->Positions[i] + glm::vec3(0.0f, PointOffset, 0.0f);
		auto Side	= glm::normalize(glm::cross(Trajectory->Directions[i], glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 Forward = Base + 15.0f * Trajectory->Directions[i];
		const auto PointColor = FColor(Trajectory->GaitStand[i] * 255, Trajectory->GaitJog[i] * 255, Trajectory->GaitWalk[i] * 255);

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResult(ForceInit);

		GetWorld()->LineTraceSingleByChannel(HitResult, FVector(Forward.x, Forward.y + PointOffset, Forward.z), -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);
		Forward.z = HitResult.Location.Z;
		Forward.z = 0; //DEBUG: Can be removed

		const glm::vec3 Arrow0 = Forward + ArrowHeadLenth * Side + ArrowHeadLenth * -Trajectory->Directions[i];
		const glm::vec3 Arrow1 = Forward - ArrowHeadLenth * Side + ArrowHeadLenth * -Trajectory->Directions[i];

		//const auto UArrow0 = FVector(Arrow0.x, Arrow0.y, Arrow0.z);
		const auto UArrow0 = FVector(Arrow0.x, Arrow0.y, 0);
		//const auto UArrow1 = FVector(Arrow1.x, Arrow1.y, Arrow1.z);
		const auto UArrow1 = FVector(Arrow1.x, Arrow1.y, 0);

		DrawDebugLine(GetWorld(), FVector(Forward.x, Forward.y, Forward.z), UArrow0, PointColor, false, -1, 0, 1);
		DrawDebugLine(GetWorld(), FVector(Forward.x, Forward.y, Forward.z), UArrow1, PointColor, false, -1, 0, 1);
		DrawDebugLine(GetWorld(), FVector(Base.x, Base.y, Base.z), FVector(Forward.x, Forward.y, Forward.z), PointColor, false, -1, 0, 1);


	}
}

void ATP_ThirdPersonCharacter::DrawDebugUI()
{
	//Phase circle
	//const float CircleStepSize = 0.1f;
	//for(int i = 0; i < 2*PI; i+= CircleStepSize)
	//{
	//	const auto LineStart	= FVector(static_cast<float>(GEngine->GameViewport->Viewport->GetSizeXY().X) - 125 + 50*FMath::Cos(i				 ), 100.0f+50.0f*FMath::Sin(i				  ), 0.0f);
	//	const auto LineEnd		= FVector(static_cast<float>(GEngine->GameViewport->Viewport->GetSizeXY().X) - 125 + 50*FMath::Cos(i + CircleStepSize), 100.0f+50.0f*FMath::Sin(i + CircleStepSize), 0.0f);
	//	DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Black, false, -1, 0, 1);	
	//}
	

	/*DrawDebugPoint(GetWorld(), FVector(static_cast<float>(GEngine->GameViewport->Viewport->GetSizeXY().X) - 125 + 50 * FMath::Cos(Phase), 100 + 50 * FMath::Sin(Phase), 0), 1.0f, FColor::Red, false, 0.03f);
	
	const auto Pindex1 = static_cast<int>(Phase / (2 * PI) * 50);
	Eigen::MatrixXf W0p = PFNN_SkeletalMesh->PFNN->W0[Pindex1];

	for(int x = 0; x < W0p.rows(); x++)
	{
		for(int y = 0; y < W0p.cols(); y++)
		{
			const float value = (W0p(x, y) + 0.5f) / 2.0f;
			const glm::vec3 color = value > 0.5f ? glm::mix(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), value - 0.5f) : glm::mix(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), value*2.0f);
			DrawDebugPoint(GetWorld(), FVector(static_cast<float>( W0p.cols() + y - 25.0f), x, 0.0f), 2.0f, FColor(color.r, color.g, color.b), false, 0.03f);
		}
	}*/
	
}

void ATP_ThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	Load();
	Trajectory = PFNN_SkeletalMesh->Trajectory;
}



void ATP_ThirdPersonCharacter::Tick(float DeltaSeconds)
{
	//TODO: Change devisions to multiplications since these are faster. -Feedback from dyon.

	Super::Tick(DeltaSeconds);
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, FString::Printf(TEXT("Detla seconds = %f"), DeltaSeconds));
	DrawDebugPoints();

	if (Trajectory == nullptr)
	{
		//UE_LOG(TrajectoryFatal, Fatal, TEXT("Fatal error: No trajectory found."));
		return;
	}

	//CurrentFrameInput.x = CurrentFrameInput.x * -1.0f;
	//CurrentFrameInput.y = CurrentFrameInput.y * -1.0f;
	if (glm::abs(CurrentFrameInput.x) + glm::abs(CurrentFrameInput.y) < 0.305f)
	{
		CurrentFrameInput = glm::vec2(0);
	}

	glm::vec3 TrajectoryTargetDirectionNew = glm::normalize(glm::vec3(GetActorForwardVector().X, GetActorForwardVector().Y, 0.0f));
	const glm::mat3 TrajectoryTargetRotation = glm::mat3(glm::rotate(atan2f(
			TrajectoryTargetDirectionNew.y,
			TrajectoryTargetDirectionNew.x), glm::vec3(0, 0, 1)));

	//const float TargetVelocitySpeed = ScaleBetween(GetMovementComponent()->GetMaxSpeed(), 0.0f, 600.0f) * 7.5f; //TODO: Check multiplication
	const float TargetVelocitySpeed = GetVelocity().SizeSquared() / (GetMovementComponent()->GetMaxSpeed() * GetMovementComponent()->GetMaxSpeed()) * 7.5f; //TODO: Check multiplication

	const glm::vec3 TrajectoryTargetVelocityNew = TargetVelocitySpeed * (TrajectoryTargetRotation * glm::vec3(1, 0, 0.0f));
	const glm::vec3 TTVNTEMP = glm::normalize(TrajectoryTargetVelocityNew);
	Trajectory->TargetVelocity = Trajectory->TargetVelocity;
	//Trajectory->TargetVelocity = glm::mix(Trajectory->TargetVelocity, TrajectoryTargetVelocityNew, Trajectory->ExtraVelocitySmooth);

	//TODO: Add strafing data
	StrafeAmount = glm::mix(StrafeAmount, StrafeTarget, ExtraStrafeSmooth);
	const glm::vec3 TrajectoryTargetVelocityDirection = glm::length(Trajectory->TargetVelocity) < 1e-05 ? Trajectory->TargetDirection : glm::normalize(Trajectory->TargetVelocity);
	//TrajectoryTargetDirectionNew			= MixDirections(TrajectoryTargetVelocityDirection, TrajectoryTargetDirectionNew, StrafeAmount);
	Trajectory->TargetDirection = TrajectoryTargetDirectionNew;//MixDirections(Trajectory->TargetDirection, TrajectoryTargetDirectionNew, ExtraDirectionSmooth);
	//Trajectory->TargetDirection				= MixDirections(Trajectory->TargetDirection, TrajectoryTargetDirectionNew, ExtraDirectionSmooth);
	
	//TODO: Add crouching

	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Magenta, FString::Printf(TEXT("Player forward: X:%f Y:%f Z:%f"), GetActorForwardVector().X, GetActorForwardVector().Y, GetActorForwardVector().Z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Magenta, FString::Printf(TEXT("PFNN forward: X:%f Y:%f Z:%f"),TrajectoryTargetDirectionNew.x, TrajectoryTargetDirectionNew.y, TrajectoryTargetDirectionNew.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::Printf(TEXT("Current frame input = %f %f"), CurrentFrameInput.x, CurrentFrameInput.y));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::Printf(TEXT("Target direction new = %f %f %f"), TrajectoryTargetDirectionNew.x, TrajectoryTargetDirectionNew.y, TrajectoryTargetDirectionNew.z));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Reduced speed = %f"), TargetVelocitySpeed));
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(TTVNTEMP.x, TTVNTEMP.y, TTVNTEMP.z) * 100, FColor::Red, false, -1, 1, 2);

	//Updating of the gaits
	const float MovementCutOff = 0.01f;
	const float JogCuttoff = 0.5f;
	const int Half = 2;
	const auto TrajectoryLength = glm::length(Trajectory->TargetVelocity);
	if (TrajectoryLength < MovementCutOff) //Standing
	{
		const float StandingClampMin = 0.0f;
		const float StandingClampMax = 1.0f;
		const float StandingAmount = 1.0f - glm::clamp(glm::length(Trajectory->TargetVelocity) / 0.1f, StandingClampMin, StandingClampMax);
		Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half],	StandingAmount, ExtraGaitSmooth);
		Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half]		= glm::mix(Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half],		0.0f, ExtraGaitSmooth);
		Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);

	}
	else if (glm::abs(CurrentFrameInput.x) > JogCuttoff || glm::abs(CurrentFrameInput.y) > JogCuttoff) //Jog
	{

		Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half]		= glm::mix(Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half],		1.0f, ExtraGaitSmooth);
		Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
	}
	else //Walking
	{
		Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half],	1.0f, ExtraGaitSmooth);
		Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half]		= glm::mix(Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half],		0.0f, ExtraGaitSmooth);
		Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
		Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half]	= glm::mix(Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half],	0.0f, ExtraGaitSmooth);
	}


	//Predicting future trajectory
	glm::vec3 TrajectoryPositionsBlend[UPFNNTrajectory::LENGTH];
	TrajectoryPositionsBlend[UPFNNTrajectory::LENGTH / Half] = Trajectory->Positions[UPFNNTrajectory::LENGTH / Half];
#if 0
	for (int i = UPFNNTrajectory::LENGTH / Half + 1; i < UPFNNTrajectory::LENGTH; i++)
	{
		const float BiasPosition	= Responsive ? glm::mix(2.0f, 2.0f, StrafeAmount) : glm::mix(0.5f, 1.0f, StrafeAmount);
		const float BiasDirection	= Responsive ? glm::mix(5.0f, 3.0f, StrafeAmount) : glm::mix(2.0f, 0.5f, StrafeAmount);

		const float ScalePosition	= (1.0f - powf(1.0f - (static_cast<float>(i - UPFNNTrajectory::LENGTH / Half) / (UPFNNTrajectory::LENGTH / Half)), BiasPosition));
		const float ScaleDirection	= (1.0f - powf(1.0f - (static_cast<float>(i - UPFNNTrajectory::LENGTH / Half) / (UPFNNTrajectory::LENGTH / Half)), BiasDirection));

		TrajectoryPositionsBlend[i] = TrajectoryPositionsBlend[i - 1] + Trajectory->TargetVelocity * DeltaSeconds; //glm::mix(Trajectory->Positions[i] - Trajectory->Positions[i - 1], Trajectory->TargetVelocity * DeltaTime, ScalePosition);

		//TODO: Add wall colision for future trajectory - 1519

		Trajectory->Directions[i]	= glm::mix(Trajectory->Directions[i], Trajectory->TargetDirection, ScaleDirection);

		//Trajectory->Heights[i]	= Trajectory->Heights[UPFNNTrajectory::LENGTH / Half];
		Trajectory->Heights[i]	= 0; //Debug can be removed

		Trajectory->GaitStand[i] = Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half];
		Trajectory->GaitWalk[i] = Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half];
		Trajectory->GaitJog[i] = Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half];
		Trajectory->GaitJump[i] = Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half];
		Trajectory->GaitBump[i] = Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half];
	}
#endif
	for (int i = UPFNNTrajectory::LENGTH / Half + 1; i < UPFNNTrajectory::LENGTH; i++)
	{
		//Trajectory->Positions[i] += Trajectory->TargetVelocity * DeltaSeconds;// TrajectoryPositionsBlend[i];
	}
	//TODO: Add jumping here 
	//for(int i = UPFNNTrajectory::LENGTH / Half; i < UPFNNTrajectory::LENGTH; i++)
	//Trajectory->GaitJump[i] = Distance from ground

	//Trajectory rotation
	for (int i = 0; i < UPFNNTrajectory::LENGTH; i++)
	{
		Trajectory->Rotations[i] = glm::mat3(glm::rotate(atan2f(Trajectory->Directions[i].x, Trajectory->Directions[i].y), glm::vec3(0, 0, 1)));
	}


	//Trajectory height
	for (int i = UPFNNTrajectory::LENGTH / Half; i < UPFNNTrajectory::LENGTH; i++)
	{
		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResult(ForceInit);
		GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);


		Trajectory->Positions[i].z = HitResult.Location.Z;
		Trajectory->Positions[i].z = 0; //DEBUG: removable
	}

	const int TrajectoryPointStepSize = 10;
	Trajectory->Heights[UPFNNTrajectory::LENGTH / Half] = 0.0f;
	for (int i = 0; i < UPFNNTrajectory::LENGTH; i += TrajectoryPointStepSize)
	{
		Trajectory->Heights[UPFNNTrajectory::LENGTH / Half] += Trajectory->Positions[i].z / (UPFNNTrajectory::LENGTH / TrajectoryPointStepSize);
	}

	/*const glm::vec3 RootPosition = glm::vec3(
		Trajectory->Positions[UPFNNTrajectory::LENGTH / Half].x + GetActorLocation().X,
		Trajectory->Positions[UPFNNTrajectory::LENGTH / Half].y + GetActorLocation().Y,
		///*Trajectory->Heights[UPFNNTrajectory::LENGTH / Half]0.0f);*/

	const glm::vec3 RootPosition = glm::vec3(
		 GetActorLocation().X,
		 GetActorLocation().Y,
		/*Trajectory->Heights[UPFNNTrajectory::LENGTH / Half]*/0.0f);


	const glm::mat3 RootRotation = Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half];

	//Input trajectiory positions and directions
	for (int i = 0; i < UPFNNTrajectory::LENGTH; i += TrajectoryPointStepSize)
	{
		int w = UPFNNTrajectory::LENGTH / TrajectoryPointStepSize;
		const glm::vec3 Position	= glm::inverse((RootRotation)) * (Trajectory->Positions[i] - RootPosition);
		const glm::vec3 Direction	= glm::inverse((RootRotation)) * Trajectory->Directions[i];
		PFNN_SkeletalMesh->PFNN->Xp((w * 0) + i / TrajectoryPointStepSize) = Position.x;
		PFNN_SkeletalMesh->PFNN->Xp((w * 1) + i / TrajectoryPointStepSize) = Position.y;
		PFNN_SkeletalMesh->PFNN->Xp((w * 2) + i / TrajectoryPointStepSize) = Direction.x;
		PFNN_SkeletalMesh->PFNN->Xp((w * 3) + i / TrajectoryPointStepSize) = Direction.y;
	}

	// Input trajectory gaits
	for (int i = 0; i < UPFNNTrajectory::LENGTH; i += TrajectoryPointStepSize)
	{
		const int w = UPFNNTrajectory::LENGTH / TrajectoryPointStepSize;
		PFNN_SkeletalMesh->PFNN->Xp((w * 4) + i / TrajectoryPointStepSize) = Trajectory->GaitStand[i];
		PFNN_SkeletalMesh->PFNN->Xp((w * 5) + i / TrajectoryPointStepSize) = Trajectory->GaitWalk[i];
		PFNN_SkeletalMesh->PFNN->Xp((w * 6) + i / TrajectoryPointStepSize) = Trajectory->GaitJog[i];
		PFNN_SkeletalMesh->PFNN->Xp((w * 7) + i / TrajectoryPointStepSize) = 0; //Unused input for crouch?;
		PFNN_SkeletalMesh->PFNN->Xp((w * 8) + i / TrajectoryPointStepSize) = Trajectory->GaitJump[i];
		PFNN_SkeletalMesh->PFNN->Xp((w * 9) + i / TrajectoryPointStepSize) = 0; //Unused input
	}

	//Input previous join position / velocity / rotations
	const glm::vec3 PreviousRootPosition = glm::vec3(
		Trajectory->Positions[UPFNNTrajectory::LENGTH / Half - 1].x,
		Trajectory->Positions[UPFNNTrajectory::LENGTH / Half - 1].y,
		/*Trajectory->Heights[UPFNNTrajectory::LENGTH / Half - 1]*/0.0f);

	const glm::mat3 PreviousRootRotation = Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half - 1];

	for (int i = 0; i < JOINT_NUM; i++)
	{
		const int o = (((UPFNNTrajectory::LENGTH) / TrajectoryPointStepSize)*TrajectoryPointStepSize);
		const glm::vec3 Position = glm::inverse(PreviousRootRotation) * (JointPosition[i] - PreviousRootPosition);
		const glm::vec3 Previous = glm::inverse(PreviousRootRotation) * JointVelocitys[i];
		//Magical numbers are indexes for the PFNN
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 0) = Position.x;
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 1) = Position.z;
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 2) = Position.y;
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 0) = Previous.x;
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 1) = Previous.z;
		PFNN_SkeletalMesh->PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 2) = Previous.y;
	}

	//Input heights for the trajectory
	for (int i = 0; i < UPFNNTrajectory::LENGTH; i += TrajectoryPointStepSize)
	{
		const int o = (((UPFNNTrajectory::LENGTH) / TrajectoryPointStepSize)*TrajectoryPointStepSize) + JOINT_NUM * 3 * 2;
		const int w = UPFNNTrajectory::LENGTH / TrajectoryPointStepSize;

		const glm::vec3 PositionRight	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(Trajectory->Width, 0, 0));
		const glm::vec3 PositionLeft	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(-Trajectory->Width, 0, 0));

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, this);
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResultLeft(ForceInit);
		FHitResult HitResultRight(ForceInit);

		const FVector UPositionRight = FVector(PositionRight.x, PositionRight.y, PositionRight.z);
		const FVector UPositionLeft = FVector(PositionLeft.x, PositionLeft.y, PositionLeft.z);

		GetWorld()->LineTraceSingleByChannel(HitResultRight, UPositionLeft, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);
		GetWorld()->LineTraceSingleByChannel(HitResultLeft, UPositionRight, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);

		//TODO: Add height addition
		PFNN_SkeletalMesh->PFNN->Xp(o + (w * 0) + i / TrajectoryPointStepSize) = /*HitResultRight.Location.Z*/ 0 - RootPosition.z;
		PFNN_SkeletalMesh->PFNN->Xp(o + (w * 1) + i / TrajectoryPointStepSize) = Trajectory->Positions[i].z; /*- RootPosition.z*/
		PFNN_SkeletalMesh->PFNN->Xp(o + (w * 2) + i / TrajectoryPointStepSize) = /*HitResultLeft.Location.Z-*/ 0 - RootPosition.z;
	}

	//Preform regression
	PFNN_SkeletalMesh->PFNN->Predict(Phase);

	const int InputLayerSkip = 8;
	//Build local transformation for the joints
	for (int i = 0; i < JOINT_NUM; i++)
	{
		const int OPosition = InputLayerSkip + (((UPFNNTrajectory::LENGTH / Half) / TrajectoryPointStepSize) * 4) + JOINT_NUM * 3 * 0;
		const int OVelocity = InputLayerSkip + (((UPFNNTrajectory::LENGTH / Half) / TrajectoryPointStepSize) * 4) + JOINT_NUM * 3 * 1;
		const int ORoation	= InputLayerSkip + (((UPFNNTrajectory::LENGTH / Half) / TrajectoryPointStepSize) * 4) + JOINT_NUM * 3 * 2;

		//Flipped Y and Z
		const glm::vec3 Position = RootRotation *									glm::vec3(PFNN_SkeletalMesh->PFNN->Yp(OPosition + i * 3 + 0), PFNN_SkeletalMesh->PFNN->Yp(OPosition + i * 3 + 2), PFNN_SkeletalMesh->PFNN->Yp(OPosition + i * 3 + 1)) + RootPosition;
		const glm::vec3 Velocity = RootRotation *									glm::vec3(PFNN_SkeletalMesh->PFNN->Yp(OVelocity + i * 3 + 0), PFNN_SkeletalMesh->PFNN->Yp(OVelocity + i * 3 + 2), PFNN_SkeletalMesh->PFNN->Yp(OVelocity + i * 3 + 1));
		const glm::mat3 Rotation = RootRotation * glm::toMat3(QuaternionExpression(	glm::vec3(PFNN_SkeletalMesh->PFNN->Yp(ORoation	+ i * 3 + 0), PFNN_SkeletalMesh->PFNN->Yp(ORoation	+ i * 3 + 2), PFNN_SkeletalMesh->PFNN->Yp(ORoation	+ i * 3 + 1))));

		JointPosition[i] = glm::mix(JointPosition[i] + Velocity, Position, ExtraJointSmooth);
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
	for (int i = 0; i < UPFNNTrajectory::LENGTH / Half; i++)
	{
		Trajectory->Positions[i]	= Trajectory->Positions	[i + 1];
		Trajectory->Directions[i]	= Trajectory->Directions[i + 1];
		Trajectory->Rotations[i]	= Trajectory->Rotations	[i + 1];
		Trajectory->Heights[i]		= Trajectory->Heights	[i + 1];
		Trajectory->GaitStand[i]	= Trajectory->GaitStand	[i + 1];
		Trajectory->GaitWalk[i]		= Trajectory->GaitWalk	[i + 1];
		Trajectory->GaitJog[i]		= Trajectory->GaitJog	[i + 1];
		Trajectory->GaitBump[i]		= Trajectory->GaitBump	[i + 1];
	}

	//Update current trajectory
	const float StandAmount = powf(1.0f - Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half], 0.25f);

	const glm::vec3 TrajectoryUpdate = Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half] * glm::vec3(PFNN_SkeletalMesh->PFNN->Yp(0), PFNN_SkeletalMesh->PFNN->Yp(1), 0.0f); //TODEBUG: Rot
	Trajectory->Positions[UPFNNTrajectory::LENGTH / Half]	= Trajectory->Positions[UPFNNTrajectory::LENGTH / Half] + StandAmount * TrajectoryUpdate;
	Trajectory->Directions[UPFNNTrajectory::LENGTH / Half]	= glm::mat3(glm::rotate(StandAmount * -PFNN_SkeletalMesh->PFNN->Yp(2), glm::vec3(0, 0, 1))) * Trajectory->Directions[UPFNNTrajectory::LENGTH / Half]; //TODEBUG: Rot
	Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half]	= glm::mat3(glm::rotate(glm::atan(
		Trajectory->Directions[UPFNNTrajectory::LENGTH / Half].x,
		Trajectory->Directions[UPFNNTrajectory::LENGTH / Half].y), glm::vec3(0, 0, 1)));

	//TODO: Add wall logic

	//Update future trajectory
	for (int i = UPFNNTrajectory::LENGTH / Half + 1; i < UPFNNTrajectory::LENGTH; i++)
	{
	//int i = UPFNNTrajectory::LENGTH - 1;
		const int W = (UPFNNTrajectory::LENGTH / Half) / TrajectoryPointStepSize;
		const float M = fmod((static_cast<float>(i) - (UPFNNTrajectory::LENGTH / Half)) / TrajectoryPointStepSize, 1.0f);
		Trajectory->Positions[i].x = (glm::normalize(TrajectoryTargetDirectionNew).x * 5);
		Trajectory->Positions[i].y = (glm::normalize(TrajectoryTargetDirectionNew).y * 5);
		Trajectory->Directions[i].x = TrajectoryTargetDirectionNew.x;
		Trajectory->Directions[i].y = TrajectoryTargetDirectionNew.y;

		//Trajectory->Positions[i].x	= (1 - M) * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 0) + (i / TrajectoryPointStepSize) - W) + M * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 0) + (i / TrajectoryPointStepSize) - W + 1); //TODEBUG: Rot
		//Trajectory->Positions[i].y	= (1 - M) * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 1) + (i / TrajectoryPointStepSize) - W) + M * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 1) + (i / TrajectoryPointStepSize) - W + 1); //TODEBUG: Rot
		//Trajectory->Directions[i].x = (1 - M) * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 2) + (i / TrajectoryPointStepSize) - W) + M * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 2) + (i / TrajectoryPointStepSize) - W + 1); //TODEBUG: Rot
		//Trajectory->Directions[i].y = (1 - M) * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 3) + (i / TrajectoryPointStepSize) - W) + M * PFNN_SkeletalMesh->PFNN->Yp(InputLayerSkip + (W * 3) + (i / TrajectoryPointStepSize) - W + 1); //TODEBUG: Rot
		//
		Trajectory->Positions[i]	= (Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half] * Trajectory->Positions[i]) + Trajectory->Positions[UPFNNTrajectory::LENGTH / Half];
		Trajectory->Directions[i]	= glm::normalize((Trajectory->Rotations[UPFNNTrajectory::LENGTH / Half] * Trajectory->Directions[i]));
		Trajectory->Rotations[i]	= glm::mat3(glm::rotate(atan2f(Trajectory->Directions[i].x, Trajectory->Directions[i].y), glm::vec3(0, 0, 1)));
	}


	//Update the phase
	Phase = fmod(Phase + (StandAmount * 0.9f + 0.1f) * 2.0f * PI * PFNN_SkeletalMesh->PFNN->Yp(3), 2.0f * PI);

	//for (int i = 0; i < JOINT_NUM; i++)
	//{
	//	JointMeshXform[i] = glm::rotate(JointMeshXform[i], 90.0f, glm::vec3(1, 0, 0)); //Rotating the final mesh bones 90 degrees over the X axis
	//	JointGlobalAnimXform[i] = glm::rotate(JointMeshXform[i], 90.0f, glm::vec3(1, 0, 0)); //Rotating the final mesh bones 90 degrees over the X axis
	//	JointGlobalRestXform[i] = glm::rotate(JointMeshXform[i], 90.0f, glm::vec3(1, 0, 0)); //Rotating the final mesh bones 90 degrees over the X axis
	//}

	TArray<FVector> FinalBoneLocations;
	TArray<FQuat>	FinalBoneRotations;
	FinalBoneLocations.SetNum(JOINT_NUM);
	FinalBoneRotations.SetNum(JOINT_NUM);
	const float FinalScale = 1.0f;

	for(int32 i = 0; i < JOINT_NUM; i++)
	{
		FinalBoneLocations[i] = FVector(JointPosition[i].x, JointPosition[i].y, JointPosition[i].z);

		FMatrix UMatrix;
		UMatrix.M[0][0] = JointMeshXform[i][0][0];	UMatrix.M[1][0] = JointMeshXform[i][1][0];	UMatrix.M[2][0] = JointMeshXform[i][2][0];	UMatrix.M[3][0] = JointMeshXform[i][3][0];
		UMatrix.M[0][1] = JointMeshXform[i][0][1];	UMatrix.M[1][1] = JointMeshXform[i][1][1];	UMatrix.M[2][1] = JointMeshXform[i][2][1];	UMatrix.M[3][1] = JointMeshXform[i][3][1];
		UMatrix.M[0][2] = JointMeshXform[i][0][2];	UMatrix.M[1][2] = JointMeshXform[i][1][2];	UMatrix.M[2][2] = JointMeshXform[i][2][2];	UMatrix.M[3][2] = JointMeshXform[i][3][2];
		UMatrix.M[0][3] = 0;						UMatrix.M[1][3] = 0;						UMatrix.M[2][3] = 0;						UMatrix.M[3][3] = 1;

		FinalBoneRotations[i] = FQuat(1, 0, 0, 0);
	}

	PFNN_SkeletalMesh->PosableMesh->SetSkeletonData(FinalBoneLocations, FinalBoneRotations, FinalScale);

	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Trajectory->GaitStand half = %f"), Trajectory->GaitStand[UPFNNTrajectory::LENGTH / Half]));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Trajectory->GaitWalk half = %f"), Trajectory->GaitWalk[UPFNNTrajectory::LENGTH / Half]));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Trajectory->GaitJog half = %f"), Trajectory->GaitJog[UPFNNTrajectory::LENGTH / Half]));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Trajectory->GaitJump half = %f"), Trajectory->GaitJump[UPFNNTrajectory::LENGTH / Half]));
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("Trajectory->GaitBump half = %f"), Trajectory->GaitBump[UPFNNTrajectory::LENGTH / Half]));

}

void ATP_ThirdPersonCharacter::Load()
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

	for(int i = 0; i < JOINT_NUM; i++)
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

	for(int i = 0; i < JOINT_NUM; i++)
	{
		JointRestXform[i] = JointRestXform[i];
	}

	delete FileHandle;
}


void ATP_ThirdPersonCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ATP_ThirdPersonCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ATP_ThirdPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATP_ThirdPersonCharacter::MoveForward(float Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Move forward value: %f"), Value));
	CurrentFrameInput = glm::vec2(CurrentFrameInput.x, Value);

	if ((Controller != NULL) && (Value != 0.0f))
	{
		const FRotator TrajectoryTargetDirectionNew = Controller->GetControlRotation();
		const FRotator YawRotation(0, TrajectoryTargetDirectionNew.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		XMovementDirection = Direction;

		AddMovementInput(Direction, Value);
	}
}

void ATP_ThirdPersonCharacter::MoveRight(float Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("Move right value: %f"), Value));
	CurrentFrameInput = glm::vec2(Value, CurrentFrameInput.y);

	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		YMovementDirection = Direction;

		AddMovementInput(Direction, Value);
	}
}