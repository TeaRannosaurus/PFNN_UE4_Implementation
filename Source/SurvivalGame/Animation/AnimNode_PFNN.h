// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include <ThirdParty/glm/vec3.hpp>
#include <ThirdParty/glm/mat4x4.hpp>
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

	//Amount of joints
	enum
	{
		JOINT_NUM = 31
	};

	/** Base Pose*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink BasePose;

	/* Trajectory pin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trajectory, meta = (PinShownByDefault))
	UTrajectoryComponent* Trajectory = nullptr;

	/* Gait pins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float GaitStand;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float GaitWalk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float GaitJog;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float GaitJump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float GaitBump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gaits, meta = (PinShownByDefault))
	float ExtraGaitSmooth;

	glm::vec3 JointPosition[JOINT_NUM];
	glm::vec3 JointVelocitys[JOINT_NUM];
	glm::mat4 JointRotations[JOINT_NUM];

	glm::mat4 JointAnimXform[JOINT_NUM];
	glm::mat4 JointRestXform[JOINT_NUM];
	glm::mat4 JointMeshXform[JOINT_NUM];
	glm::mat4 JointGlobalRestXform[JOINT_NUM];
	glm::mat4 JointGlobalAnimXform[JOINT_NUM];
	int JointParents[JOINT_NUM];

	UPROPERTY(EditAnywhere, Category = PFNN, meta = (ClampMin = 0, ClampMax = 2, UIMin = 0, UIMax = 2))
	int PFNNMode;
	static UPhaseFunctionNeuralNetwork* PFNN;

	void LoadData();
	void LoadXForms();
	void LoadPFNN() const;


	void ApplyPFNN(FPoseContext& arg_LocalPoseContext);

	void PredictFutureTrajectory(UTrajectoryComponent* arg_Trajectory);

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
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)	override {}
	/**
	* Default AnimNode function to update bones
	* @param[in] Context, Animation context
	*/
	virtual void Update_AnyThread(const FAnimationUpdateContext & Context) 			override;
	/**
	* Default AnimNode function to evaluate bones
	* @param[in] Context, Bone context
	*/
	virtual void Evaluate_AnyThread(FPoseContext& Output) 							override;
	// End of FAnimNode_Base interface
};
