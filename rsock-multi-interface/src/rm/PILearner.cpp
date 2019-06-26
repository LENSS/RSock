//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <cassert>
#include <hrpMessages.h>
#include "PILearner.h"


namespace hrp
{
	PILearner::PILearner(RMCore &core, double alpha) : _core(core), _alpha(alpha)
	{
		_pi_timeout = HRP_DEFAULT_PI_PROBE_EXPIRATION;
		_seq = 0;
		_response_seq = 0;
		_pi_action = 0;
		_pi_epoch = _core.get_rmax();
		_oc_latency_cnt = 0;
		_one_copy_latency = 0;
		_nrof_round_per_epoch = 1;
		_curr_nr_of_round = 0;

		_logger = spdlog::get("PILearner");
		if (!_logger)
			_logger = std::make_shared<spdlog::logger>("PILearner", file_sink);
		if (debug_mode)
			_logger->set_level(spdlog::level::debug);
	}

	pi_probe_meta PILearner::prepareProbe()
	{
		std::lock_guard<std::mutex> gl(_mutex);
		pi_probe_meta ret;
		ret.seq = _seq++;
		ret.repl = drawNextAction();

		// sentProbe may not be called since tx may fail, so insert it to
		// track table now, and then if sentProbe is called, we update it again
		auto itr = _track_table.find(ret.seq);
		// if probe needs to send multiple times, only count the first one
		if (itr == _track_table.end())
		{
			std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
			std::pair<unsigned int, std::chrono::steady_clock::time_point>
			state(ret.repl, t);
			_track_table[ret.seq] = state;
		}
		return ret;
	}



	void PILearner::sentProbe(const pi_probe_meta &meta)
	{
		std::lock_guard<std::mutex> gl(_mutex);

		//auto itr = _track_table.find(meta.seq);
		// if probe needs to send multiple times, only count the first one
		//if (itr == _track_table.end()) {

		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
		std::pair<unsigned int, std::chrono::steady_clock::time_point> state(meta.repl, t);
		_track_table[meta.seq] = state;

		//}
	}


	void PILearner::recvResponse(const pi_response_tx &data)
	{

		double latency = -1;
		{
			std::lock_guard<std::mutex> gl(_mutex);
			auto itr = _track_table.find(data.seq);
			if (itr != _track_table.end())
			{
				std::chrono::duration<double> d = std::chrono::steady_clock::now() - itr->second.second;
				latency = d.count();
			}
		}

		if (latency != -1)
			recvResponse(data, latency);
	}


	void PILearner::recvResponse(const pi_response_tx &meta, double latency)
	{

		std::lock_guard<std::mutex> gl(_mutex);

		auto itr = _track_table.find(meta.seq);
		if (itr != _track_table.end())
		{

			std::pair<unsigned int, std::chrono::steady_clock::time_point> state = itr->second;
			unsigned int seq = itr->first;
			unsigned int repl = state.first;
			pi_response_local resp;
			resp.latency = latency;
			resp.meta = meta;

			// std::cout << "Measured latency: " << resp.latency * 1000 << " ms" << std::endl;

			/** how to implement this part? gathering latency info
			* and update the RMCore if necessary. Maybe just implement a
			* queue A, and a expected received response. Put not-in-order
			* response in another queue B. When purging, add those into the
			* queue as well. Then when queue A is full (size == epoch),
			* calculate the loss vector, and update the RMCore.
			*/

			// if this is the expected next seq
			if (seq == _response_seq)
			{
				_loss_aggr.push_back(resp);
				_response_seq++;

				moveOutOfOrderBuf();
				if (_loss_aggr.size() == _pi_epoch)
					updateCore();
			}
			else
			{
				_out_order_buf.push(resp);
			}

			// in either way, this has to be deleted from _track_table

			_track_table.erase(itr);
		}
		// otherwise we just ignore it
	}


	void PILearner::update()
	{
		_curr_nr_of_round = _curr_nr_of_round >= _nrof_round_per_epoch ? 0 : (_curr_nr_of_round + 1);
		purgeOutdatedProbes();
	}


	void PILearner::purgeOutdatedProbes()
	{

		std::lock_guard<std::mutex> gl(_mutex);

		//_logger->debug("purgeOutdatedProbes");
		//_logger->debug("_response_seq: {}", _response_seq);
		//_logger->debug("_tracking_table: {}", format_track_table());
		//_logger->debug("_out_of_order_buf: {}", format_out_of_order_buf());

		if (!_track_table.empty())
		{
			for (auto itr = _track_table.begin(); itr != _track_table.end(); )
			{
				unsigned int seq = itr->first;
				// if the seq does not equal to _response_seq, then we may have a few
				// out-of-order responses in the buffer
				if (seq != _response_seq)
					moveOutOfOrderBuf();

				//if (_response_seq != seq) {
				//_logger->debug("_response_seq: {}", _response_seq);
				//_logger->debug("_tracking_table: {}", format_track_table());
				//_logger->debug("_out_of_order_buf: {}", format_out_of_order_buf());
				//}

				assert(_response_seq == seq);
				std::pair<unsigned int, std::chrono::steady_clock::time_point>
				state = itr->second;
				std::chrono::duration<double> duration = std::chrono::steady_clock::now() - state.second;


				// if it's timeout, we need to do something
				if (duration.count() > _pi_timeout)
				{
					// construct a response entry
					pi_response_local data;
					data.latency = _pi_timeout;
					data.meta.seq = seq;
					data.meta.repl = state.first;

					// add it to _loss_aggr
					_loss_aggr.push_back(data);
					// advance _response_seq
					_response_seq++;

					// delete it from the map
					itr = _track_table.erase(itr);
				}
				else
					break;

				if (_loss_aggr.size() == _pi_epoch)
					updateCore();
			}

			moveOutOfOrderBuf();
			if (_loss_aggr.size() == _pi_epoch)
				updateCore();
		}
	}


	void PILearner::moveOutOfOrderBuf()
	{

		// look at _out_order_buf, and add resp to _loss_aggr
		while (!_out_order_buf.empty())
		{
			unsigned int tseq = _out_order_buf.top().meta.seq;
			if ( tseq != _response_seq)
				break;

			if (_loss_aggr.size() == _pi_epoch)
				updateCore();
			_loss_aggr.push_back(_out_order_buf.top());
			_out_order_buf.pop();
			_response_seq++;
		}
	}


	void PILearner::updateCore()
	{

		std::vector<double> loss(_core.get_rmax());
		std::vector<int> cnt(_core.get_rmax());

		// update one hop latency first
		for (int i=0; i<_loss_aggr.size(); i++)
		{
			pi_response_local &data = _loss_aggr[i];
			if (data.meta.repl == 1)
			{
				_one_copy_latency = _one_copy_latency + (data.latency - _one_copy_latency) / (double)(++_oc_latency_cnt);
			}
		}

		for (int i=0; i<_loss_aggr.size(); i++)
		{
			pi_response_local &data = _loss_aggr[i];
			cnt[data.meta.repl-1]++;
			loss[data.meta.repl-1] += RMCore::calcLoss(_alpha, data.meta.repl, _core.get_rmax(), data.latency, _one_copy_latency);
		}

		for (int i=0; i<_core.get_rmax(); i++)
		{
			if (cnt[i] != 0)
				loss[i] /= (double)cnt[i];
		}

		std::vector<double> prev_prob = _core.get_probs();
		_core.receiveLossVec(loss);
		const std::vector<double> &curr_prob = _core.get_probs();
		bool greater = false;
		for (int i=0; i<curr_prob.size(); i++)
		{
			if (fabs(prev_prob[i] - curr_prob[i]) >= 0.01)
			{
				greater = true;
				break;
			}
		}

		_nrof_round_per_epoch = greater ? 1 : 2 * _nrof_round_per_epoch;

		//std::cout << _core.format_probs();
		//std::cout << "round = " << _nrof_round_per_epoch << std::endl;

		// clear _loss_aggr
		_loss_aggr.clear();
		//_logger->debug("probs: {}", _core.format_probs());
	}

	unsigned int PILearner::drawNextAction()
	{
		unsigned int ret;
		if (_pi_action < _core.get_rmax())
			ret = _pi_action + 1;
		else
			ret = _core.drawAction();
		_pi_action = (_pi_action+1)%_pi_epoch;
		return ret;
	}

	unsigned int PILearner::get_seq() const
	{
		return _seq;
	}

	unsigned int PILearner::get_pi_epoch() const
	{
		return _pi_epoch;
	}


	const std::map<unsigned int, std::pair<unsigned int, std::chrono::steady_clock::time_point>> &PILearner::get_track_table() const
	{
		return _track_table;
	}

	unsigned int PILearner::get_pi_action() const
	{
		return _pi_action;
	}

	void PILearner::set_pi_epoch(unsigned int _pi_epoch)
	{
		PILearner::_pi_epoch = _pi_epoch;
	}

	void PILearner::set_pi_timeout(double _pi_timeout)
	{
		PILearner::_pi_timeout = _pi_timeout;
	}

	const std::vector<pi_response_local> &PILearner::get_loss_aggr() const
	{
		return _loss_aggr;
	}


	const std::priority_queue<pi_response_local, std::vector<pi_response_local>, PRDCompare> & PILearner::get_out_order_buf() const
	{
		return _out_order_buf;
	}

	unsigned int PILearner::get_response_seq() const
	{
		return _response_seq;
	}

	std::string PILearner::format_track_table() const
	{
		std::stringstream ss;
		ss << "{";
		for (auto itr : _track_table)
		{
			ss << "[" << itr.first << ", " << itr.second.first << "], ";
		}
		ss << "}";
		return ss.str();
	}

	std::string PILearner::format_out_of_order_buf() const
	{
		std::priority_queue<pi_response_local, std::vector<pi_response_local>, PRDCompare> tmp(_out_order_buf);
		std::stringstream ss;
		ss << "<";
		while (!tmp.empty())
		{
			ss << tmp.top().meta.seq << ", ";
			tmp.pop();
		}
		ss << ">";
		return ss.str();
	}
}
