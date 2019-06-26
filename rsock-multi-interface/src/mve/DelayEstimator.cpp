//
// Created by Chen on 7/21/17.
//

#include <iostream>
#include <cmath>
#include "DelayEstimator.h"

namespace hrp
{

	/**
	* Helper utility function. Calculate the total rate when
	* except the node
	* @param srcCore The MVECore for the src node
	* @param dst The node that is not considered
	* @param dstCore The MVECore for the dst node
	* @return The total rate when not considering dst
	*/
	double getTotalIndirectRate(const std::map<ect_set, double>& src_params, const hrpNode &dst, const ect_set &dst_allnodes)
	{
		double ret = 0;
		// traverse all encounter_sets
		for (auto itr = src_params.begin(); itr != src_params.end(); itr++)
		{
	
			// if the dst is in the encounter_set, then continue
			if (itr->first.find(dst) != itr->first.end())
				continue;
			// otherwise, see if this encounter_set has common elements with intersect_set, i.e. there's
			// some hrpNode that both src and dst meet with, if so, add the contact rate to the total
			ect_set tmp;
			std::set_intersection(itr->first.begin(), itr->first.end(), dst_allnodes.begin(), dst_allnodes.end(), std::inserter(tmp, tmp.end()));
			if (!tmp.empty())
			{
				ret += itr->second;
			}
		}
		return ret;
	}


	std::vector<double> calcDelays(unsigned int rmax, const MVECore &_srcCore, const MVECore &_dstCore)
	{
		if (rmax == 0)
			throw hrpException("Invalid rmax: cannot be zero");
		std::vector<double> ret(rmax);

		// Get total rate for direct contact
		double total_direct_rate_to_dst = _srcCore.getMarginal(ect_set({_dstCore.get_thisNode()}));

		// Get total rate for contacts to ect_sets not including dst
		double total_rate_to_carriers = getTotalIndirectRate(_srcCore.get_params(), _dstCore.get_thisNode(), _dstCore.get_allNodes());
		
		if (total_rate_to_carriers == 0)
		{
			// std::cout << "Warning, there's no one hop neighbor that has direct contact to destination" << std::endl;
			if (total_direct_rate_to_dst > 0)
				ret.assign(rmax, 1/total_direct_rate_to_dst);
			else
				ret.assign(rmax, std::numeric_limits<double>::max());
			return ret;
		}

		std::map<hrpNode, double> node_prob_per_contact;

		// Calculate per-hrpNode per-contact probability
		for (auto itr = _dstCore.get_allNodes().begin(); itr != _dstCore.get_allNodes().end(); itr++)
		{
			double val = 0;
			if (*itr == _srcCore.get_thisNode())
			{
				val = 1;
			}
			else
			{
				double sum_total_rate = 0;
				for (auto mapitr = _srcCore.get_params().begin(); mapitr != _srcCore.get_params().end(); mapitr++)
				{
					if (mapitr->first.find(*itr) != mapitr->first.end() && mapitr->first.find(_dstCore.get_thisNode()) == mapitr->first.end())
					{
						// Note, r might be zero in some cases
						double r = mapitr->second;
						ect_set intersect_set;
						std::set_intersection(
									mapitr->first.begin(), mapitr->first.end(),
									_dstCore.get_allNodes().begin(),
									_dstCore.get_allNodes().end(),
									std::inserter(intersect_set, intersect_set.end())
								);
						double one_over_s = 1.0 / (double) intersect_set.size();

						// sum_total_rate might be zero if r == 0
						sum_total_rate += (r * one_over_s);
					}
				}
				// val might be zero if sum_total_rate == 0
				val = sum_total_rate/total_rate_to_carriers;
			}
			node_prob_per_contact.insert(std::pair<hrpNode,double>(*itr, val));
		}

		ret[0] = total_direct_rate_to_dst==0 ? std::numeric_limits<double>::max() : 1/total_direct_rate_to_dst;
		
		for (unsigned int repl = 2; repl <= rmax; repl++ )
		{
			double total_rate_to_dst = 0;

			for (auto itr = _dstCore.get_params().begin(); itr != _dstCore.get_params().end(); itr++)
			{
				ect_set encountered = itr->first;
				double rate_for_encountered = itr->second;
				double set_prob_not_chosen = 1;
				for (hrpNode n : encountered)
				{
					auto node_prob_itr = node_prob_per_contact.find(n);
					if (node_prob_itr != node_prob_per_contact.end())
					{
						if (node_prob_itr->second == 1 || node_prob_itr->second == 0)
						{
							set_prob_not_chosen = node_prob_itr->second == 1 ? 0 : 1;
							break;
						}
						set_prob_not_chosen *= (1 - node_prob_itr->second * (1 - std::pow(1 - node_prob_itr->second, repl-1)) / (node_prob_itr->second));
					}
				}
				total_rate_to_dst += (1 - set_prob_not_chosen) * rate_for_encountered;
			}

			if (total_rate_to_dst == 0)
				ret[repl-1] = std::numeric_limits<double>::max();
			else
				ret[repl-1] = (repl - 1)/total_rate_to_carriers + (1.0 / total_rate_to_dst);
		}

		return ret;
	}
}
