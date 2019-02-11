// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThirdParty/Eigen/Dense"

/**
 * 
 */
class SURVIVALGAME_API PhaseFunctionNeuralNetwork
{
public:
	PhaseFunctionNeuralNetwork(const int pfnnmode);
	~PhaseFunctionNeuralNetwork();

	enum { XDIM = 342, YDIM = 311, HDIM = 512 };
	enum { MODE_CONSTANT, MODE_LINEAR, MODE_CUBIC };

	int mode;

	Eigen::ArrayXf Xmean, Xstd;
	Eigen::ArrayXf Ymean, Ystd;

	TArray<Eigen::ArrayXXf> W0, W1, W2;
	TArray<Eigen::ArrayXf>  b0, b1, b2;

	Eigen::ArrayXf  Xp, Yp;
	Eigen::ArrayXf  H0, H1;
	Eigen::ArrayXXf W0p, W1p, W2p;
	Eigen::ArrayXf  b0p, b1p, b2p;

	void load();
	static void load_weights(Eigen::ArrayXXf &A, int rows, int cols, const FString FileName, ...);
	static void load_weights(Eigen::ArrayXf &V, int items, const FString FileName, ...);
	
	static void ELU(Eigen::ArrayXf& x);

	static void linear(Eigen::ArrayXf& o, const Eigen::ArrayXf& y0, const Eigen::ArrayXf& y1, float mu);
	static void linear(Eigen::ArrayXXf& o, const Eigen::ArrayXXf& y0, const Eigen::ArrayXXf& y1, float mu);

	static void cubic(Eigen::ArrayXf& o, const Eigen::ArrayXf& y0, const Eigen::ArrayXf& y1, const Eigen::ArrayXf& y2, const Eigen::ArrayXf& y3, float mu);
	static void cubic(Eigen::ArrayXXf& o, const Eigen::ArrayXXf& y0, const Eigen::ArrayXXf& y1, const Eigen::ArrayXXf& y2, const Eigen::ArrayXXf& y3, float mu);

	void Predict(float arg_Phase);
};
