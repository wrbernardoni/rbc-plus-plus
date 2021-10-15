#include "NeuralModel.h"

#include <random>

#include <iostream>
#include <cmath>

#include <fstream>

#include "../utilities/json.hpp"
using json = nlohmann::json;

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

WRB_Chess::NeuralModel::NeuralModel(int w, int d)
{
	// Make everything randomly
	std::default_random_engine generator;
	std::poisson_distribution<int> layerWidthDist(w);
	std::poisson_distribution<int> networkDepth(d);

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

WRB_Chess::NeuralModel::NeuralModel()
{
	// Make everything randomly
	std::default_random_engine generator;
	std::poisson_distribution<int> layerWidthDist(64 * 6);
	std::poisson_distribution<int> networkDepth(10);

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

WRB_Chess::NeuralModel::NeuralModel(std::string fn)
{
	// Load from file
	json mod;
	std::ifstream outputFile(fn);
	outputFile >> mod;
	outputFile.close();

	int layers = mod["layers"];

	for (int i = 0; i < layers; i++)
	{
		int rows = mod["shape"][i]["rows"];
		int cols = mod["shape"][i]["cols"];
		model.push_back(Eigen::MatrixXd(rows, cols));

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < cols; c++)
			{
				model[i](r,c) = mod["weights"][i][r*cols + c];
			}
		}
	}
}

void WRB_Chess::NeuralModel::save(std::string fn)
{
	json mod;

	mod["layers"] = model.size();

	for (int i = 0; i < model.size(); i++)
	{
		json lShape;
		lShape["rows"] = model[i].rows();
		lShape["cols"] = model[i].cols();
		mod["shape"].push_back(lShape);

		json weights;

		for (int r = 0; r < model[i].rows(); r++)
		{
			for (int c = 0; c < model[i].cols(); c++)
			{
				weights.push_back(model[i](r,c));
			}
		}

		mod["weights"].push_back(weights);
	}

	std::ofstream outputFile(fn);
	outputFile << mod << std::endl;
	outputFile.close();
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

	return test.unaryExpr(&sigmoid)(0,0);
}

WRB_Chess::NeuralModel& WRB_Chess::NeuralModel::operator=(const WRB_Chess::NeuralModel& rhs)
{
	model.clear();

	for (int i = 0; i < rhs.model.size(); i++)
	{
		Eigen::MatrixXd cpy = rhs.model[i];
		model.push_back(cpy);
	}

	return *this;
}

#define DROPOUT_RATE 0.7

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
	double outErr = score - inp.unaryExpr(&sigmoid)(0,0);
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

double WRB_Chess::NeuralModel::trainBatch(std::vector<WRB_Chess::Bitboard> brd, std::vector<bool> toMove, std::vector<double> score, double lr)
{
	std::vector<std::vector<Eigen::MatrixXd>> deltasB;
	std::vector<std::vector<Eigen::MatrixXd>> inputsB;

	double sumErr = 0.0;
	for (int b = 0; b < brd.size(); b++)
	{
		Eigen::MatrixXd inp = boardToVec(brd[b], toMove[b]);

		std::vector<Eigen::MatrixXd> deltas;
		std::vector<Eigen::MatrixXd> inputs;

		for (int i = 0; i < model.size(); i++)
		{
			deltas.push_back(inp);
			inp.conservativeResize(inp.rows() + 1, inp.cols());
			inp.bottomLeftCorner(1,inp.cols()) = Eigen::MatrixXd::Constant(1,inp.cols(), 1.0);
			inp = ((1.0 / DROPOUT_RATE) * inp).cwiseProduct(Eigen::MatrixXd::Random(inp.rows(), inp.cols()).unaryExpr(&dropout));
			inputs.push_back(inp);
			inp = model[i] * inp.unaryExpr(&sigmoid);
		}
		inputs.push_back(inp);
		deltas.push_back(inp);
		double outErr = score[b] - inp.unaryExpr(&sigmoid)(0,0);
		sumErr += std::pow(outErr,2);
		deltas[deltas.size() - 1] = outErr * inputs[inputs.size() - 2].unaryExpr(&dsigmoid);

		for (int i = deltas.size() - 2; i >= 1; i--)
		{
			deltas[i] = (model[i-1].transpose() * deltas[i]).cwiseProduct(inputs[i-1].unaryExpr(&dsigmoid));
		}

		deltasB.push_back(deltas);
		inputsB.push_back(inputs);
	}

	std::vector<Eigen::MatrixXd> oldModel;
	for (int i = 0; i < model.size(); i++)
	{
		oldModel.push_back(model[i]);
		for (int j = 0; j < model[i].rows() - 1; j++)
		{
			for (int b = 0; b < brd.size(); b++)
			{
				model[i].row(j) = model[i].row(j) - (1.0/(double)brd.size())* lr * deltasB[b][i+2](j,0) * inputsB[b][i].unaryExpr(&sigmoid).transpose();
			}
		}
	}

	double eS = 0.0;
	for (int b = 0; b < brd.size(); b++)
	{
		double postTrainScore = runForward(brd[b], toMove[b]);
		eS += std::pow(score[b] - postTrainScore,2);
	}
	
	double dErr = (eS - sumErr)/((double)brd.size());
	
	/*
	if (dErr >= 0)
	{
		model = oldModel;
	}
	*/

	return dErr;
}
