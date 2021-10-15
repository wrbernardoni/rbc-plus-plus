#ifndef WRB_CHESS_NEURAL_MODEL_H_
#define WRB_CHESS_NEURAL_MODEL_H_

#include "../chess/board.h"
#include <Eigen/Dense>
#include <vector>
#include <string>

namespace WRB_Chess
{
	class NeuralModel
	{
	protected:
		std::vector<Eigen::MatrixXd> model;
	public:
		NeuralModel();
		NeuralModel(int, int);
		NeuralModel(std::string);

		double runForward(const WRB_Chess::Bitboard&, bool);
		double train(const WRB_Chess::Bitboard&, bool, double, double);
		double trainBatch(std::vector<WRB_Chess::Bitboard>, std::vector<bool>, std::vector<double>, double);

		void save(std::string);

		WRB_Chess::NeuralModel& operator=(const WRB_Chess::NeuralModel&);
	};
};

#endif
