//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <string>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include "RMCore.h"

namespace hrp
{

	RMCore::RMCore(unsigned int rmax, double ita) throw() : _rmax(rmax), _ita(ita), _weights(rmax), _probs(rmax)
	{
		if (rmax >= 100)
		{
			throw hrpException("Invalid rmax value, too large!");
		}
		if (ita <=0 || ita >=1)
		{
			throw hrpException("Invalid ita value");
		}

		for (int i=0; i<rmax; i++)
		{
			_weights[i] = 1;
			_probs[i] = 1.0/((double)rmax);
		}
	}


	double RMCore::calcLoss(double alpha, unsigned int repl, unsigned int rmax, double latency, double one_hop_latency)
	{
		if (latency < 0)
			throw hrpException("Invalid latency value: negative");
		if (one_hop_latency < 0)
			throw hrpException("Invalid one_hop_latency value: negative");
		if (repl > rmax)
			throw hrpException("Invalid repl value: cannot be larger than rmax");
		return alpha * std::min(1.0, latency/one_hop_latency) + (1-alpha) * (double)(repl - 1) / (double)(rmax - 1);
	}


	void RMCore::receiveLossVec(const std::vector<double> &loss) throw()
	{
		assert(loss.size() == _rmax);
		double ts = 0;
		for (int i=0; i<_rmax; i++)
		{
			if (loss[i] < 0 || loss[i] > 1)
			{
				throw hrpException("Invalid loss vector");
			}
			_weights[i] = _weights[i] * (1 - _ita * loss[i]);
			ts += _weights[i];
		}

		for (int i=0; i<_rmax; i++)
		{
			_probs[i] = _weights[i] / ts;
		}
	}

	std::string RMCore::format_probs() const
	{
		std::stringstream ss;
		for (int i=0; i<_rmax; i++)
		{
			ss << _probs[i];
			if (i != _rmax - 1)
				ss << " ";
			else ss << std::endl;
		}
		return ss.str();
	}


	unsigned int RMCore::drawAction() const
	{
		double r = (double)rand() / (double)RAND_MAX;
		double s = 0.0;
		for (unsigned int i=0; i<_rmax; i++)
		{
			s += _probs[i];
			if (r < s)
				return i+1;
		}
		return _rmax;
	}


	unsigned int RMCore::get_rmax() const
	{
		return _rmax;
	}

	double RMCore::get_ita() const
	{
		return _ita;
	}

	const std::vector<double> &RMCore::get_weights() const
	{
		return _weights;
	}

	const std::vector<double> &RMCore::get_probs() const
	{
		return _probs;
	}

}
