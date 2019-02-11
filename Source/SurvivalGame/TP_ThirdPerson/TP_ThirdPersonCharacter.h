// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdParty/glm/glm.hpp"
#include "TP_ThirdPersonCharacter.generated.h"

class UPFNN_SkeletalMeshComponent;
class UPFNNTrajectory;

UCLASS(config=Game)
class ATP_ThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ATP_ThirdPersonCharacter();

	//Amount of joints
	enum
	{
		JOINT_NUM = 31
	};

	//Numbers for the bones
	enum
	{
		JOINT_ROOT_L	= 1,
		JOINT_HIP_L		= 2,
		JOINT_KNEE_L	= 3,
		JOINT_HEEL_L	= 4,
		JOINT_TOE_L		= 5,
		
		JOINT_ROOT_R	= 6,
		JOINT_HIP_R		= 7,
		JOINT_KNEE_R	= 8,
		JOINT_HEEL_R	= 9,
		JOINT_TOE_R		= 10,

	};

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PFNN)
	float TargetVelocitySpeed;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	glm::vec2 CurrentFrameInput;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float Phase;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float StrafeAmount; 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float StrafeTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float ExtraDirectionSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float ExtraStrafeSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float ExtraGaitSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float ExtraJointSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float Responsive;

	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::vec3 JointPosition[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::vec3 JointVelocitys[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointRotations[JOINT_NUM];

	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointAnimXform[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointRestXform[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointMeshXform[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointGlobalRestXform[JOINT_NUM];
	//UPROPERTY(VisibleAnywhere, Category = PFNN)
	glm::mat4 JointGlobalAnimXform[JOINT_NUM];

	UPROPERTY(VisibleAnywhere, Category = PFNN)
	int JointParents[JOINT_NUM];
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	UPFNNTrajectory* Trajectory;

	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	

	void Load();


	UPROPERTY(EditAnywhere, Category = PFNN)
	class UPFNN_SkeletalMeshComponent* PFNN_SkeletalMesh;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:

	FVector XMovementDirection; 
	FVector YMovementDirection;

	/*
	 * @Description Returnes a liniar bled between the X and Y vector by using the floating point scalar
	 * @Param[in] X vector
	 * @Param[in] Y vector
	 * @Param[in] Scalar
	 * @Return The result of liniar blending X, Y and the scalar
	 */
	FVector MixVector(const FVector arg_XVector, const FVector arg_YVector, const float arg_Scalar);
	/*
	* @Description Returnes a liniar bled between the X and Y float by using the floating point scalar
	* @Param[in] X float
	* @Param[in] Y float
	* @Param[in] Scalar
	* @Return The result of liniar blending X, Y and the scalar
	*/
	float MixFloat(const float arg_XFloat, const float arg_YFloat, const float arg_Scalar);
	/*
	* @Description Returnes a liniar bled between the X and Y vector direction by using the floating point scalar
	* @Param[in] X vector
	* @Param[in] Y vector
	* @Param[in] Scalar
	* @Return The result of liniar blending X, Y and the scalar
	*/
	glm::vec3 MixDirections(const glm::vec3 arg_XDirection, const glm::vec3 arg_YDirection, const float arg_Scalar);
	/*
	* @Description Creates a quaternion based on a angle and axis
	* @Param[in] Angle
	* @Param[in] Axis
	* @Return Resulting calculation
	*/
	FQuat CreateQuaternionFromAngleAxis(const float arg_Angle, const FVector arg_Axis);
	FQuat QuaternionExpresion(const FVector arg_Length);
	FMatrix QuaternionToMatrix(const FQuat arg_Quat);
	glm::quat QuaternionExpression(const glm::vec3 arg_Length);
	FVector MatrixVectorMultiplication(const FMatrix arg_InMatrix, const FVector arg_InVector);
	FMatrix VectorToMatrix(const FVector arg_InVector);
	FMatrix RotateVectorOverAngle(const float arg_Angle, FVector arg_Vector);
	float ScaleBetween(const float arg_Unscaled, const float arg_Min, const float arg_Max);

	void DrawDebugPoints();
	void DrawDebugTrajectory();
	void DrawDebugUI();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

