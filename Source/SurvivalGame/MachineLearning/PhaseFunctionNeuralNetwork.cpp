// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PhaseFunctionNeuralNetwork.h"

#include "GenericPlatformFile.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "PlatformFilemanager.h"

UPhaseFunctionNeuralNetwork::UPhaseFunctionNeuralNetwork()
{
}

UPhaseFunctionNeuralNetwork::UPhaseFunctionNeuralNetwork(const int arg_PFNNMode) : mode(arg_PFNNMode)
{
	Xp = Eigen::ArrayXf(static_cast<int>(XDIM));
	Yp = Eigen::ArrayXf(static_cast<int>(YDIM));

	H0 = Eigen::ArrayXf(static_cast<int>(HDIM));
	H1 = Eigen::ArrayXf(static_cast<int>(HDIM));

	W0p = Eigen::ArrayXXf(static_cast<int>(HDIM), static_cast<int>(XDIM));
	W1p = Eigen::ArrayXXf(static_cast<int>(HDIM), static_cast<int>(HDIM));
	W2p = Eigen::ArrayXXf(static_cast<int>(YDIM), static_cast<int>(HDIM));

	b0p = Eigen::ArrayXf(static_cast<int>(HDIM));
	b1p = Eigen::ArrayXf(static_cast<int>(HDIM));
	b2p = Eigen::ArrayXf(static_cast<int>(YDIM));
}

UPhaseFunctionNeuralNetwork::~UPhaseFunctionNeuralNetwork()
{
}

void UPhaseFunctionNeuralNetwork::LoadNetworkData()
{
	//TODO: Look into Asynchronous Asset Loading -Elwin
	LoadWeights(Xmean, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Xmean.bin")));
	LoadWeights(Xstd, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Xstd.bin")));
	LoadWeights(Ymean, YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Ymean.bin")));
	LoadWeights(Ystd, YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Ystd.bin")));

	switch (mode)
	{

	case MODE_CONSTANT:

		W0.SetNum(50); W1.SetNum(50); W2.SetNum(50);
		b0.SetNum(50); b1.SetNum(50); b2.SetNum(50);

		for (int i = 0; i < 50; i++)
		{
			LoadWeights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03d.bin"), i));
			LoadWeights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03d.bin"), i));
			LoadWeights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03d.bin"), i));
			LoadWeights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03d.bin"), i));
			LoadWeights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03d.bin"), i));
			LoadWeights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03d.bin"), i));
		}

		break;

	case MODE_LINEAR:

		W0.SetNum(10); W1.SetNum(10); W2.SetNum(10);
		b0.SetNum(10); b1.SetNum(10); b2.SetNum(10);

		for (int i = 0; i < 10; i++)
		{
			LoadWeights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03d.bin"), i * 5));
			LoadWeights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03d.bin"), i * 5));
			LoadWeights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03d.bin"), i * 5));
			LoadWeights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03d.bin"), i * 5));
			LoadWeights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03d.bin"), i * 5));
			LoadWeights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03d.bin"), i * 5));
		}

		break;

	case MODE_CUBIC:

		W0.SetNum(4); W1.SetNum(4); W2.SetNum(4);
		b0.SetNum(4); b1.SetNum(4); b2.SetNum(4);

		for (int i = 0; i < 4; i++)
		{
			LoadWeights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03i.bin"), static_cast<int>(i * 12.5)));
			LoadWeights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03i.bin"), static_cast<int>(i * 12.5)));
			LoadWeights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03i.bin"), static_cast<int>(i * 12.5)));
			LoadWeights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03i.bin"), static_cast<int>(i * 12.5)));
			LoadWeights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03i.bin"), static_cast<int>(i * 12.5)));
			LoadWeights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03i.bin"), static_cast<int>(i * 12.5)));
		}

		break;
	}
}
void UPhaseFunctionNeuralNetwork::LoadWeights(Eigen::ArrayXXf& arg_A, const int arg_Rows, const int arg_Cols, const FString arg_FileName, ...)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString RelativePath = FPaths::ProjectDir();
	const FString FullPath = RelativePath += arg_FileName;

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPath);

	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
		return;
	}

	arg_A = Eigen::ArrayXXf(arg_Rows, arg_Cols);
	for (int x = 0; x < arg_Rows; x++)
	{
		for (int y = 0; y < arg_Cols; y++)
		{
			FFloat32 item;
			uint8* ByteBuffer = reinterpret_cast<uint8*>(&item);

			FileHandle->Read(ByteBuffer, sizeof(FFloat32));
			arg_A(x, y) = item.FloatValue;
		}
	}

	delete FileHandle;
}
void UPhaseFunctionNeuralNetwork::LoadWeights(Eigen::ArrayXf& arg_V, const int arg_Items, const FString arg_FileName, ...)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString RelativePath = FPaths::ProjectDir();
	const FString FullPath = RelativePath += arg_FileName;

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPath);

	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
		return;
	}

	arg_V = Eigen::ArrayXf(arg_Items);

	for (int i = 0; i < arg_Items; i++)
	{
		FFloat32 item;
		uint8* ByteBuffer = reinterpret_cast<uint8*>(&item);

		FileHandle->Read(ByteBuffer, sizeof(FFloat32));
		arg_V(i) = item.FloatValue;
	}

	delete FileHandle;
}

void UPhaseFunctionNeuralNetwork::ELU(Eigen::ArrayXf& arg_X)
{
	arg_X = arg_X.max(0) + arg_X.min(0).exp() - 1;
}

void UPhaseFunctionNeuralNetwork::Linear(Eigen::ArrayXf& arg_Out, const Eigen::ArrayXf& arg_Y0, const Eigen::ArrayXf& arg_Y1, float arg_MU)
{
	arg_Out = (1.0f - arg_MU) * arg_Y0 + (arg_MU)* arg_Y1;
}
void UPhaseFunctionNeuralNetwork::Linear(Eigen::ArrayXXf& arg_Out, const Eigen::ArrayXXf& arg_Y0, const Eigen::ArrayXXf& arg_Y1, float arg_MU)
{
	arg_Out = (1.0f - arg_MU) * arg_Y0 + (arg_MU)* arg_Y1;
}

void UPhaseFunctionNeuralNetwork::Cubic(Eigen::ArrayXf& arg_Out, const Eigen::ArrayXf& arg_Y0, const Eigen::ArrayXf& arg_Y1, const Eigen::ArrayXf& arg_Y2, const Eigen::ArrayXf& arg_Y3, float arg_MU)
{
	arg_Out = (
		(-0.5 * arg_Y0 + 1.5 * arg_Y1 - 1.5 * arg_Y2 + 0.5 * arg_Y3) * arg_MU * arg_MU * arg_MU +
		(arg_Y0 - 2.5 * arg_Y1 + 2.0 * arg_Y2 - 0.5 * arg_Y3) * arg_MU * arg_MU +
		(-0.5 * arg_Y0 + 0.5 * arg_Y2) * arg_MU +
		(arg_Y1));
}
void UPhaseFunctionNeuralNetwork::Cubic(Eigen::ArrayXXf& arg_Out, const Eigen::ArrayXXf& arg_Y0, const Eigen::ArrayXXf& arg_Y1, const Eigen::ArrayXXf& arg_Y2, const Eigen::ArrayXXf& arg_Y3, float arg_MU)
{
	arg_Out = (
		(-0.5 * arg_Y0 + 1.5 * arg_Y1 - 1.5 * arg_Y2 + 0.5 * arg_Y3) * arg_MU * arg_MU * arg_MU +
		(arg_Y0 - 2.5 * arg_Y1 + 2.0 * arg_Y2 - 0.5 * arg_Y3) * arg_MU * arg_MU +
		(-0.5 * arg_Y0 + 0.5 * arg_Y2) * arg_MU +
		(arg_Y1));
}

void UPhaseFunctionNeuralNetwork::Predict(float arg_Phase)
{
	float pamount;
	int pindex_0;
	int pindex_1;
	int pindex_2;
	int pindex_3;

	Xp = (Xp - Xmean) / Xstd;

	arg_Phase = FMath::Abs(arg_Phase);
	switch (mode)
	{
	case MODE_CONSTANT:
		pindex_1 = static_cast<int>((arg_Phase / (2 * PI)) * 50);
		H0 = (W0[pindex_1].matrix() * Xp.matrix()).array() + b0[pindex_1];
		ELU(H0);
		H1 = (W1[pindex_1].matrix() * H0.matrix()).array() + b1[pindex_1];
		ELU(H1);
		Yp = (W2[pindex_1].matrix() * H1.matrix()).array() + b2[pindex_1];
		break;

	case MODE_LINEAR:
		pamount = fmod((arg_Phase / (2 * PI)) * 10, 1.0);
		pindex_1 = (int)((arg_Phase / (2 * PI)) * 10);
		pindex_2 = ((pindex_1 + 1) % 10);
		Linear(W0p, W0[pindex_1], W0[pindex_2], pamount);
		Linear(W1p, W1[pindex_1], W1[pindex_2], pamount);
		Linear(W2p, W2[pindex_1], W2[pindex_2], pamount);
		Linear(b0p, b0[pindex_1], b0[pindex_2], pamount);
		Linear(b1p, b1[pindex_1], b1[pindex_2], pamount);
		Linear(b2p, b2[pindex_1], b2[pindex_2], pamount);
		H0 = (W0p.matrix() * Xp.matrix()).array() + b0p;
		ELU(H0);
		H1 = (W1p.matrix() * H0.matrix()).array() + b1p;
		ELU(H1);
		Yp = (W2p.matrix() * H1.matrix()).array() + b2p;
		break;

	case MODE_CUBIC:
		pamount = fmod((arg_Phase / (2 * PI)) * 4, 1.0);
		pindex_1 = (int)((arg_Phase / (2 * PI)) * 4);
		pindex_0 = ((pindex_1 + 3) % 4);
		pindex_2 = ((pindex_1 + 1) % 4);
		pindex_3 = ((pindex_1 + 2) % 4);
		Cubic(W0p, W0[pindex_0], W0[pindex_1], W0[pindex_2], W0[pindex_3], pamount);
		Cubic(W1p, W1[pindex_0], W1[pindex_1], W1[pindex_2], W1[pindex_3], pamount);
		Cubic(W2p, W2[pindex_0], W2[pindex_1], W2[pindex_2], W2[pindex_3], pamount);
		Cubic(b0p, b0[pindex_0], b0[pindex_1], b0[pindex_2], b0[pindex_3], pamount);
		Cubic(b1p, b1[pindex_0], b1[pindex_1], b1[pindex_2], b1[pindex_3], pamount);
		Cubic(b2p, b2[pindex_0], b2[pindex_1], b2[pindex_2], b2[pindex_3], pamount);
		H0 = (W0p.matrix() * Xp.matrix()).array() + b0p;
		ELU(H0);
		H1 = (W1p.matrix() * H0.matrix()).array() + b1p;
		ELU(H1);
		Yp = (W2p.matrix() * H1.matrix()).array() + b2p;
		break;

	default:
		break;
	}

	Yp = (Yp * Ystd) + Ymean;
}
