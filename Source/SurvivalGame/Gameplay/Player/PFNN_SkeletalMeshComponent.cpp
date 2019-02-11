// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PFNN_SkeletalMeshComponent.h"
#include "Gameplay/Movement/AnimationController/PhaseFunctionNeuralNetwork.h"
#include "Gameplay/Movement/PFNNTrajectory.h"
#include "Gameplay/Player/PFNN_PosableMesh.h"

UPFNN_SkeletalMeshComponent::UPFNN_SkeletalMeshComponent(): Trajectory(nullptr), PFNN(nullptr)
{
	PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_CONSTANT);
	///Other load modes for the PFNN
	//PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_CUBIC);
	//PFNN = new PhaseFunctionNeuralNetwork(PhaseFunctionNeuralNetwork::MODE_LINEAR);

	Trajectory = CreateDefaultSubobject<UPFNNTrajectory>(TEXT("Trajectory"));
	PosableMesh = CreateDefaultSubobject<UPFNN_PosableMesh>(TEXT("PosableMesh"));
}

UPFNN_SkeletalMeshComponent::~UPFNN_SkeletalMeshComponent()
{
	//PosableMesh->BeginDestroy();
	//Trajectory->BeginDestroy();
	//delete PFNN;
}

void UPFNN_SkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	PosableMesh->SetSkeletalMeshObject(SkeletalMesh);
	PFNN->load();


	//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
}
