// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNN_SkeletalMeshComponent.h"
#include "Gameplay/Movement/PFNNTrajectory.h"
#include "Gameplay/Player/PFNN_PosableMesh.h"

UPFNN_SkeletalMeshComponent::UPFNN_SkeletalMeshComponent(): Trajectory(nullptr), PFNN(nullptr)
{
	///Other load modes for the PFNN
	//PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_CONSTANT);
	PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_CUBIC);
	//PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_LINEAR);

	Trajectory = CreateDefaultSubobject<UPFNNTrajectory>(TEXT("Trajectory"));
	//PosableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PosableMeshComponent"));
	//PFNNPosableMesh = CreateDefaultSubobject<UPFNN_PosableMesh>(TEXT("PFNNPosableMesh"));
}

UPFNN_SkeletalMeshComponent::~UPFNN_SkeletalMeshComponent()
{
	//PFNNPosableMesh->BeginDestroy();
	//Trajectory->BeginDestroy();
	//delete PFNN;
}

void UPFNN_SkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	PFNNPosableMesh->SetSkeletalMeshObject(SkeletalMesh);
	//PosableMesh->AllocateTransformData();
	PFNN->LoadNetworkData();


	//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
}
