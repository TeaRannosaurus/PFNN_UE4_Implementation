// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdParty/glm/glm.hpp"
#include "TP_ThirdPersonCharacter.generated.h"

class UPFNN_SkeletalMeshComponent;
class UPFNNTrajectory;
struct FSkeletonData;

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

	UPROPERTY(VisibleAnywhere, Category = PFNN)
	class UPoseableMeshComponent* PosableMesh;

public:
	ATP_ThirdPersonCharacter();

	//Amount of joints
	enum
	{
		JOINT_NUM = 31
	};


	//Bone definitions
	enum
	{
		//Root
		JOINT_ROOT		= 0,

		//Left leg
		JOINT_ROOT_L	= 1,
		JOINT_HIP_L		= 2,
		JOINT_KNEE_L	= 3,
		JOINT_HEEL_L	= 4,
		JOINT_TOE_L		= 5,
		
		//Right leg
		JOINT_ROOT_R	= 6,
		JOINT_HIP_R		= 7,
		JOINT_KNEE_R	= 8,
		JOINT_HEEL_R	= 9,
		JOINT_TOE_R		= 10,

		//Back to head
		JOINT_BACK		= 11,
		JOINT_SPINE_1	= 12,
		JOINT_SPINE_2	= 13,
		JOINT_NECK		= 14,
		JOINT_NECK1		= 15,
		JOINT_HEAD		= 16,

		//Right arm
		JOINT_SHOULDER_L= 17,
		JOINT_ARM_L		= 18,
		JOINT_FOREARM_L	= 19,
		JOINT_HAND_L	= 20,
		JOINT_FINGER_L	= 21,
		JOINT_INDEX_L	= 22,
		JOINT_THUMB_L	= 23,

		//Left arm
		JOINT_SHOULDER_R= 24,
		JOINT_ARM_R		= 25,
		JOINT_FOREARM_R	= 26,
		JOINT_HAND_R	= 27,
		JOINT_FINGER_R	= 28,
		JOINT_INDEX_R	= 29,
		JOINT_THUMB_R	= 30,
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
	void PoseMesh(FSkeletonData arg_SkeletonData);
	
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

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

private:

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
	glm::quat QuaternionExpression(const glm::vec3 arg_Length);
	/*
	* @Description Scales the given value between the given min and max
	* @Param[in] arg_Unscaled
	* @Param[in] arg_Min
	* @Param[in] arg_Max
	* @Return Resulting calculation
	*/
	float ScaleBetween(const float arg_Unscaled, const float arg_Min, const float arg_Max);

#if !UE_BUILD_SHIPPING //Debug functions are excluded from the shipping build
	int BoneStep;
	void DrawDebugPoints();
	void DrawDebugTrajectory();
	//void DrawDebugUI();
	void DebugUp();
	void DebugDown();
#endif

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};