//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_MVEESTIMATOR_H
#define HRP_MVEESTIMATOR_H


#include <hrpNode.h>
#include <chrono>
#include <map>
#include <mutex>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include "MVECore.h"
#include "common.h"


namespace hrp
{

	class MVEEstimator
	{
	
		public:
		MVEEstimator(MVECore &core, std::mutex &mutex_core) : _core(core), _mutex_core(mutex_core), _io_service(NULL)
		{
			_lastCheck = std::chrono::steady_clock::now();
			_epochDuration = HRP_DEFAULT_MVE_EPOCH_DURATION;
			_epochStarted = false;
			_totalEncounter = 0;
			_totalDuration = std::chrono::duration<double>(0);
			_logger = spdlog::get("MVEEstimator");
			if (!_logger)
				_logger = std::make_shared<spdlog::logger>("MVEEstimator", file_sink);
			if (debug_mode)
				_logger->set_level(spdlog::level::debug);
		}

		//~MVEEstimator() { delete _mve_timer; }

		void init();
		void set_io_service(asio::io_service *io) { _io_service = io; }

		void addNode(const hrpNode& _n);

		/**
		* Update all the parameters.
		*/
		void updateParams();

		/**
		* Calculate the contact rates given total number of encounters,
		* number of encounters for a given set, and total duration.
		* See Eq. (1) in the HRP paper. This method facilitates unit testing.
		* @param total Total number of encounters
		* @param encouters Total number of encounters for a given set
		* @param duration  Total duration
		* @return  Estimated contact rates
		*/
		static double rateCalc(unsigned int total, double encouters, double duration);

		// Setter and getters
		void set_epochDuration(double _epochDuration);
		double get_epochDuration() const;
		unsigned int get_totalEncounter() const;
		const ect_set &get_encountered() const;
		const std::map<ect_set, double> &get_counters() const;
		const std::chrono::duration<double, std::ratio<1, 1>> &get_totalDuration() const;


		private:
		/**
		* Start a new epoch for calculating MVE parameters.
		* This method will automatically start another thread for
		* timer purpose. Once the timer fired, stopEpoch() method
		* is called.
		*/
		void startEpoch();

		/**
		* Wait for a duration, and stop the epoch. This method will be
		* automatically invoked in a new thread created in startEpoch() method.
		*/
		void stopEpoch(const std::error_code &error);

		/**
		* Utility function for calculation of the MVE parameters given
		* the encounter counts for a specific set of node. See Eq. (1)
		* in the Mobihoc '16 paper.
		* @param cntForSet The count of encounters
		* @return  The estimated contact rates for this set of nodes
		*/
		double paramCalc(double cntForSet);

		private:
		// Logger
		std::shared_ptr<spdlog::logger> _logger;
		asio::io_service *_io_service;
		asio::steady_timer  *_mve_timer;    /// timer for topology fetching

		MVECore &_core;                                 /// The MVECore this estimator is to maintain
		std::mutex &_mutex_core;                        /// The mutex object for the above core
		unsigned int _totalEncounter;                   /// Total number of encounters
		ect_set _encountered;                           /// The EncounterSet
		std::map<ect_set, double> _counters;            /// The counters for each EncouterSet
		std::chrono::duration<double> _totalDuration;   /// Total duration until now (in seconds)
		std::chrono::steady_clock::time_point _lastCheck;   /// The time point when last epoch was started
		double  _epochDuration;                         /// The duration of an epoch (in seconds)
		std::mutex _mutex;
		bool _epochStarted;
	};
}
#endif //HRP_MVEESTIMATOR_H
