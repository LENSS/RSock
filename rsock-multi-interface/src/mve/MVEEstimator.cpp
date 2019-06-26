//
// Cretaed by Chen Yang and Ala Altaweel
//

#include <thread>
#include "MVEEstimator.h"
#include "MVECore.h"


namespace hrp
{

	void MVEEstimator::init()
	{
		_mve_timer = new asio::steady_timer(*_io_service);
		if (_mve_timer == NULL)
		{
			_logger->error("init, cannot initialize _mve_timer");
			exit(1);
		}
	}

	void MVEEstimator::startEpoch()
	{
		if (!_epochStarted)
		{
			//_logger->debug("startEpoch entered");
			_epochStarted = true;
			_totalDuration += std::chrono::steady_clock::now() - _lastCheck;
			_lastCheck = std::chrono::steady_clock::now();

			// Should start a timer to stop this epoch
			// std::thread t(&MVEEstimator::stopEpoch, this);
			// t.detach();

			_mve_timer->expires_from_now(std::chrono::seconds((long long)_epochDuration));
			_mve_timer->async_wait(std::bind(&MVEEstimator::stopEpoch, this, std::placeholders::_1));
		}
		//_logger->debug("startEpoch exiting");
	}

	void MVEEstimator::stopEpoch(const std::error_code &error)
	{
		// Wait for epoch seconds, before going through
		// std::chrono::duration<double> waitDuration(_epochDuration);
		// std::this_thread::sleep_for(waitDuration);
		updateParams();
	}

	void MVEEstimator::updateParams()
	{
		//_logger->debug("updateParams entered");
		std::lock_guard<std::mutex> lock(_mutex);
		if (_epochStarted)
		{
			_epochStarted = false;
			_totalDuration += std::chrono::steady_clock::now() - _lastCheck;
			_lastCheck = std::chrono::steady_clock::now();
			// Update counters
			if (!_encountered.empty())
			{
				// Total number of contacts increment
				_totalEncounter++;
				// Counter for a specific set increment, or added
				auto itr = _counters.find(_encountered);
				if (itr != _counters.end())
				{
					itr->second++;
				}
				else
				{
					_counters.insert(std::pair<ect_set,double>(_encountered, 1));
				}
			}

			// Update MVECore parameters
			{
				std::lock_guard<std::mutex> lg(_mutex_core);
				for (auto itr = _counters.begin(); itr != _counters.end(); itr++)
				{
					double lambda = paramCalc(itr->second);
					_core.updateParam(itr->first, lambda);
				}
			}
	
			// Clear _encounterSet
			_encountered.clear();
		}
		//_logger->debug("updateParams exiting");
	}


	double MVEEstimator::rateCalc(unsigned int total, double encounters, double duration)
	{
		if (total <= 1)
			return 0;
		
		return 1.0/((double)total) * encounters / (duration/((double)total-1));
	}


	void MVEEstimator::addNode(const hrpNode &_n)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_epochStarted)
			startEpoch();
		_encountered.insert(_n);
	}

	double MVEEstimator::paramCalc(double cntForSet)
	{
		return rateCalc(_totalEncounter, cntForSet, _totalDuration.count());
	}

	void MVEEstimator::set_epochDuration(double _epochDuration)
	{
		MVEEstimator::_epochDuration = _epochDuration;
	}

	double MVEEstimator::get_epochDuration() const
	{
		return _epochDuration;
	}

	unsigned int MVEEstimator::get_totalEncounter() const
	{
		return _totalEncounter;
	}

	const ect_set &MVEEstimator::get_encountered() const
	{
		return _encountered;
	}

	const std::map<ect_set, double> &MVEEstimator::get_counters() const
	{
		return _counters;
	}

	const std::chrono::duration<double, std::ratio<1, 1>> &MVEEstimator::get_totalDuration() const
	{
		return _totalDuration;
	}
}
