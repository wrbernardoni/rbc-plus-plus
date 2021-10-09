#include "NeuralModel.h"

#include <random>

#include <iostream>
#include <cmath>

double sigmoid(double d)
{
	return 1.0 / (1.0 + std::exp(-d));
}

double dsigmoid(double d)
{
	return sigmoid(d)*(1.0 - sigmoid(d));
}

Eigen::MatrixXd boardToVec(const WRB_Chess::Bitboard& brd, bool toMove)
{
	Eigen::MatrixXd out(64 * 7 + 5,1);

	for (int p = 0; p < 6; p++)
	{
		std::bitset<64> wP = brd.Pieces(WRB_Chess::Color::White, (WRB_Chess::Piece)p);
		std::bitset<64> bP = brd.Pieces(WRB_Chess::Color::Black, (WRB_Chess::Piece)p);

		for (int i = 0; i < 64; i++)
		{
			if (wP[i])
			{
				out(p*64 + i, 0) = 1.0;
			}
			else if (bP[i])
			{
				out(p*64 + i, 0) = -1.0;
			}
			else
			{
				out(p*64 + i, 0) = 0.0;
			}
		}
	}

	for (int i = 0; i < 64; i++)
	{
		if (i != brd.getEP())
			out(6*64 + i,0) = 0;
		else
			out(6*64 + i,0) = 1.0;
	}

	if (brd.queenCastle(true))
	{
		out(7*64,0) = 1.0;
	}
	else
	{
		out(7*64,0) = 0.0;
	}

	if (brd.queenCastle(false))
	{
		out(7*64 + 1,0) = 1.0;
	}
	else
	{
		out(7*64 + 1,0) = 0.0;
	}

	if (brd.kingCastle(true))
	{
		out(7*64 + 2,0) = 1.0;
	}
	else
	{
		out(7*64 + 2,0) = 0.0;
	}

	if (brd.kingCastle(false))
	{
		out(7*64 + 3,0) = 1.0;
	}
	else
	{
		out(7*64 + 3,0) = 0.0;
	}

	if (toMove)
	{
		out(7*64 + 4,0) = 1.0;
	}
	else
	{
		out(7*64 + 4,0) = -1.0;
	}

	return out;
}

WRB_Chess::NeuralModel::NeuralModel()
{
	// Make everything randomly
	std::default_random_engine generator;
	std::poisson_distribution<int> layerWidthDist(64 * 7 + 5);
	std::poisson_distribution<int> networkDepth(20);

	std::vector<int> modelShape;
	modelShape.push_back(64 * 7 + 5); // Input layer fixed

	int depth = networkDepth(generator);
	if (depth < 3)
		depth = 3;

	for (int i = 0; i < depth; i++)
	{
		int lW = layerWidthDist(generator);
		if (lW < 1)
			lW = 1;
		modelShape.push_back(lW);
	}
	modelShape.push_back(1);

	for (int i = 1; i < modelShape.size(); i++)
	{
		int prec = modelShape[i - 1];
		int fol = modelShape[i];

		Eigen::MatrixXd weights = Eigen::MatrixXd::Random(fol, prec + 1);
		weights = weights/weights.norm();
		model.push_back(weights);
	}
}

double WRB_Chess::NeuralModel::runForward(const WRB_Chess::Bitboard& brd, bool toMove)
{
	Eigen::MatrixXd test = boardToVec(brd, toMove);

	for (int i = 0; i < model.size(); i++)
	{
		test.conservativeResize(test.rows() + 1, test.cols());
		test.bottomLeftCorner(1,test.cols()) = Eigen::MatrixXd::Constant(1,test.cols(), 1.0);
		test = model[i] * test.unaryExpr(&sigmoid);
	}

	return test(0,0);
}

#define DROPOUT_RATE 0.8

double dropout(double w)
{
	if (std::abs(w) > DROPOUT_RATE)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

double WRB_Chess::NeuralModel::train(const WRB_Chess::Bitboard& brd, bool toMove, double score, double lr)
{
	Eigen::MatrixXd inp = boardToVec(brd, toMove);

	std::vector<Eigen::MatrixXd> deltas;
	std::vector<Eigen::MatrixXd> inputs;

	for (int i = 0; i < model.size(); i++)
	{
		deltas.push_back(inp);
		inp.conservativeResize(inp.rows() + 1, inp.cols());
		inp.bottomLeftCorner(1,inp.cols()) = Eigen::MatrixXd::Constant(1,inp.cols(), 1.0);
		//inp = ((1.0 / DROPOUT_RATE) * inp).cwiseProduct(Eigen::MatrixXd::Random(inp.rows(), inp.cols()).unaryExpr(&dropout));
		inputs.push_back(inp);
		inp = model[i] * inp.unaryExpr(&sigmoid);
	}
	inputs.push_back(inp);
	deltas.push_back(inp);
	double outErr = score - inp(0,0);
	deltas[deltas.size() - 1] = outErr * inputs[inputs.size() - 2].unaryExpr(&dsigmoid);

	for (int i = deltas.size() - 2; i >= 1; i--)
	{
		deltas[i] = (model[i-1].transpose() * deltas[i]).cwiseProduct(inputs[i-1].unaryExpr(&dsigmoid));
	}

	std::vector<Eigen::MatrixXd> oldModel;
	for (int i = 0; i < model.size(); i++)
	{
		oldModel.push_back(model[i]);
		for (int j = 0; j < model[i].rows() - 1; j++)
		{
			model[i].row(j) = model[i].row(j) - lr * deltas[i+2](j,0) * inputs[i].unaryExpr(&sigmoid).transpose();
		}
	}

	double postTrainScore = runForward(brd, toMove);
	double dErr = std::pow(score - postTrainScore,2) - std::pow(outErr,2);
	if (dErr >= 0)
	{
		model = oldModel;
	}

	return dErr;
}
