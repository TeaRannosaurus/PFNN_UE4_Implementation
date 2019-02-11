	// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "PhaseFunctionNeuralNetwork.h"
#include "GenericPlatformFile.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "PlatformFilemanager.h"

PhaseFunctionNeuralNetwork::PhaseFunctionNeuralNetwork(const int pfnnmode): mode(pfnnmode)
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

PhaseFunctionNeuralNetwork::~PhaseFunctionNeuralNetwork()
{
}

void PhaseFunctionNeuralNetwork::load()
{
	//TODO: Look into Asynchronous Asset Loading -Elwin
	load_weights(Xmean,	XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Xmean.bin")));
	load_weights(Xstd,	XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Xstd.bin")));
	load_weights(Ymean,	YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Ymean.bin")));
	load_weights(Ystd,	YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/Ystd.bin")));

	switch (mode)
	{

	case MODE_CONSTANT:

		W0.SetNum(50); W1.SetNum(50); W2.SetNum(50);
		b0.SetNum(50); b1.SetNum(50); b2.SetNum(50);

		for (int i = 0; i < 50; i++) 
		{
			load_weights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03d.bin"), i));
			load_weights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03d.bin"), i));
			load_weights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03d.bin"), i));
			load_weights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03d.bin"), i));
			load_weights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03d.bin"), i));
			load_weights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03d.bin"), i));
		}

		break;

	case MODE_LINEAR:

		W0.SetNum(10); W1.SetNum(10); W2.SetNum(10);
		b0.SetNum(10); b1.SetNum(10); b2.SetNum(10);
		
		for (int i = 0; i < 10; i++) 
		{
			load_weights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03d.bin"), i * 5));
			load_weights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03d.bin"), i * 5));
			load_weights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03d.bin"), i * 5));
			load_weights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03d.bin"), i * 5));
			load_weights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03d.bin"), i * 5));
			load_weights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03d.bin"), i * 5));
		}

		break;

	case MODE_CUBIC:

		W0.SetNum(4); W1.SetNum(4); W2.SetNum(4);
		b0.SetNum(4); b1.SetNum(4); b2.SetNum(4);

		for (int i = 0; i < 4; i++) 
		{
			load_weights(W0[i], HDIM, XDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W0_%03i.bin"), static_cast<int>(i * 12.5)));
			load_weights(W1[i], HDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W1_%03i.bin"), static_cast<int>(i * 12.5)));
			load_weights(W2[i], YDIM, HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/W2_%03i.bin"), static_cast<int>(i * 12.5)));
			load_weights(b0[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b0_%03i.bin"), static_cast<int>(i * 12.5)));
			load_weights(b1[i], HDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b1_%03i.bin"), static_cast<int>(i * 12.5)));
			load_weights(b2[i], YDIM, FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/Weights/b2_%03i.bin"), static_cast<int>(i * 12.5)));
		}

		break;
	}
}
void PhaseFunctionNeuralNetwork::load_weights(Eigen::ArrayXXf& A, int rows, int cols, const FString FileName, ...)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString RelativePath = FPaths::ProjectDir();
	const FString FullPath = RelativePath += FileName;

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPath);

	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
		return;
	}

	A = Eigen::ArrayXXf(rows, cols);
	for(int x = 0; x < rows; x++)
	{
		for(int y = 0; y < cols; y++)
		{
			FFloat32 item;
			uint8* ByteBuffer = reinterpret_cast<uint8*>(&item);

			FileHandle->Read(ByteBuffer, sizeof(FFloat32));
			A(x, y) = item.FloatValue;
		}
	}

	delete FileHandle;
}
void PhaseFunctionNeuralNetwork::load_weights(Eigen::ArrayXf& V, int items, const FString FileName, ...)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString RelativePath = FPaths::ProjectDir();
	const FString FullPath = RelativePath += FileName;

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPath);

	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load Phase Function Neural Network weights. File name "));
		return;
	}

	V = Eigen::ArrayXf(items);

	for(int i = 0; i < items; i++)
	{
		FFloat32 item;
		uint8* ByteBuffer = reinterpret_cast<uint8*>(&item);

		FileHandle->Read(ByteBuffer, sizeof(FFloat32));
		V(i) = item.FloatValue;
	}

	delete FileHandle;
}

void PhaseFunctionNeuralNetwork::ELU(Eigen::ArrayXf& x)
{
	x = x.max(0) + x.min(0).exp() - 1;
}

void PhaseFunctionNeuralNetwork::linear(Eigen::ArrayXf& o, const Eigen::ArrayXf& y0, const Eigen::ArrayXf& y1, float mu)
{
	o = (1.0f - mu) * y0 + (mu) * y1;
}
void PhaseFunctionNeuralNetwork::linear(Eigen::ArrayXXf& o, const Eigen::ArrayXXf& y0, const Eigen::ArrayXXf& y1, float mu)
{
	o = (1.0f - mu) * y0 + (mu) * y1;
}

void PhaseFunctionNeuralNetwork::cubic(Eigen::ArrayXf& o, const Eigen::ArrayXf& y0, const Eigen::ArrayXf& y1, const Eigen::ArrayXf& y2, const Eigen::ArrayXf& y3, float mu)
{
	o = (
		(-0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3) * mu * mu * mu +
		(y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3) * mu * mu +
		(-0.5 * y0 + 0.5 * y2) * mu +
		(y1));
}
void PhaseFunctionNeuralNetwork::cubic(Eigen::ArrayXXf& o, const Eigen::ArrayXXf& y0, const Eigen::ArrayXXf& y1, const Eigen::ArrayXXf& y2, const Eigen::ArrayXXf& y3, float mu)
{
	o = (
		(-0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3) * mu * mu * mu +
		(y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3) * mu * mu +
		(-0.5 * y0 + 0.5 * y2) * mu +
		(y1));
}

void PhaseFunctionNeuralNetwork::Predict(float arg_Phase)
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
		linear(W0p, W0[pindex_1], W0[pindex_2], pamount);
		linear(W1p, W1[pindex_1], W1[pindex_2], pamount);
		linear(W2p, W2[pindex_1], W2[pindex_2], pamount);
		linear(b0p, b0[pindex_1], b0[pindex_2], pamount);
		linear(b1p, b1[pindex_1], b1[pindex_2], pamount);
		linear(b2p, b2[pindex_1], b2[pindex_2], pamount);
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
		cubic(W0p, W0[pindex_0], W0[pindex_1], W0[pindex_2], W0[pindex_3], pamount);
		cubic(W1p, W1[pindex_0], W1[pindex_1], W1[pindex_2], W1[pindex_3], pamount);
		cubic(W2p, W2[pindex_0], W2[pindex_1], W2[pindex_2], W2[pindex_3], pamount);
		cubic(b0p, b0[pindex_0], b0[pindex_1], b0[pindex_2], b0[pindex_3], pamount);
		cubic(b1p, b1[pindex_0], b1[pindex_1], b1[pindex_2], b1[pindex_3], pamount);
		cubic(b2p, b2[pindex_0], b2[pindex_1], b2[pindex_2], b2[pindex_3], pamount);
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
