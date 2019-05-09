// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once
#define GLM_ENABLE_EXPERIMENTAL


#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include <ThirdParty/glm/vec3.hpp>
#include <ThirdParty/glm/mat4x4.hpp>
#include <ThirdParty/glm/gtx/quaternion.hpp>

#include "AnimNode_PFNN.generated.h"


class UTrajectoryComponent;
class UPhaseFunctionNeuralNetwork;

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct SURVIVALGAME_API FAnimNode_PFNN : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	FAnimNode_PFNN();

	void LoadData();
	void LoadXForms();
	void LoadPFNN();

	void ApplyPFNN();
	glm::quat QuaternionExpression(const glm::vec3 arg_Length);

	class UPFNNAnimInstance* GetPFNNInstanceFromContext(const FAnimationInitializeContext& Context);
	class UPFNNAnimInstance* GetPFNNInstanceFromContext(const FAnimationUpdateContext& Context);

	// FAnimNode_Base interface
	/**
	 * Default AnimNode function to initialize bones
	 * @param[in] Context, Bone context
	 */
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) 	override;
	/**
	 * Default AnimNode function to cache bones
	 * @param[in] Context, Bone context
	 */
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext & arg_Context)	override {}
	/**
	* Default AnimNode function to update bones
	* @param[in] Context, Animation context
	*/
	virtual void Update_AnyThread(const FAnimationUpdateContext & arg_Context) 			override;
	/**
	* Default AnimNode function to evaluate bones
	* @param[in] Context, Bone context
	*/
	virtual void Evaluate_AnyThread(FPoseContext& arg_Output) 							override;
	// End of FAnimNode_Base interface

	void LogNetworkData(int arg_FrameCounter);

	//Amount of joints
	enum
	{
		JOINT_NUM = 31
	};

	class UPFNNAnimInstance* PFNNAnimInstance;

	UTrajectoryComponent* Trajectory = nullptr;

	//LOG THESE VARIABLES

	glm::vec3 JointPosition[JOINT_NUM];
	glm::vec3 JointVelocitys[JOINT_NUM];
	glm::mat4 JointRotations[JOINT_NUM];

	glm::mat4 JointAnimXform[JOINT_NUM];
	glm::mat4 JointRestXform[JOINT_NUM];
	glm::mat4 JointMeshXform[JOINT_NUM];
	glm::mat4 JointGlobalRestXform[JOINT_NUM];
	glm::mat4 JointGlobalAnimXform[JOINT_NUM];
	int JointParents[JOINT_NUM];

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PFNN)
	float Phase;

	TArray<FVector> FinalBoneLocations;
	TArray<FQuat>	FinalBoneRotations;

	//END LOG THESE VARIABLES

	static UPhaseFunctionNeuralNetwork* PFNN;

	int FrameCounter;

	bool bIsPFNNLoaded;

};
