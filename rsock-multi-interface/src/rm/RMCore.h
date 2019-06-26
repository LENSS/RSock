//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_RMCORE_H
#define HRP_RMCORE_H

#include <vector>
#include <string>
#include <sstream>
#include "hrpEexception.h"

namespace hrp
{
	class RMCore
	{
		public:
		RMCore(unsigned int rmax, double ita) throw();

		/**
		* Receive a loss vector and update the weights and probs
		* @param loss The loss vector received. Its size should equal to _rmax
		*/
		void receiveLossVec(const std::vector<double>& loss) throw();

		/**
		* Randomly draw an action from the action list, 1,...,_rmax
		* @return An action ranging from 1,...,_rmax
		*/

		unsigned int drawAction() const;

		static double calcLoss(double alpha, unsigned int repl, unsigned int rmax, double latency, double one_hop_latency);

		/**
		* Getters
		*/

		unsigned int get_rmax() const;
		double get_ita() const;
		const std::vector<double> &get_weights() const;
		const std::vector<double> &get_probs() const;
		std::string format_probs() const;

		private:
		/// \var The max allowed action, i.e. action list 1,...,_rmax
		unsigned int _rmax;
		/// \var The learning rate ita
		double _ita;
		/// \var Weight vector
		std::vector<double> _weights;
		/// \var Probability vector based on weights
		std::vector<double> _probs;
	};

}



#endif //HRP_RMCORE_H
