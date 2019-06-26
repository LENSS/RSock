//
// Cretaed by Chen Yang and Ala Altaweel
//

#ifndef HRP_HRPGRAPH_H
#define HRP_HRPGRAPH_H
#include <lemon/list_graph.h>
#include <lemon/dijkstra.h>
#include <lemon/adaptors.h>
#include <vector>
#include <mutex>
#include "spdlog/spdlog.h"
#include "common.h"
#include "hrpNode.h"

#define HRPGRAPH_UNREACHABLE_PATH -1

namespace hrp {
	/**
	* A wrapper class around lemon graph library, with convenient methods
	*/

	class HrpGraph
	{
		public:
		typedef typename std::vector<hrpNode> hrpPath;
		typedef typename std::pair<hrpPath, double> hrpPathEntry;
		typedef typename std::map<hrpNode, std::set<hrpNode>> succMap;

		/// Sources of below definitions:
		HrpGraph() : _vertexNames(_graph), _arcLength(_graph), _vertices() {}

		/**
		* Add a node to the graph
		* @param n
		*/
		void addNode(const hrpNode& n);

		/**
		* Add (or update) an edge. Throws exception if 1) length is negative,
		* or 2) the nodes are not in the graph already
		* @param n1 One end of the edge
		* @param n2 The other end of the edge
		* @param length The length of the edge
		*/
		void addArc(const hrpNode &n1, const hrpNode &n2, double length);
		void delNode(const hrpNode& n);
		void delEdge(const hrpNode&n1, const hrpNode& n2);

		/**
		* Return the k-shortest paths as a map
		* @param src   The source
		* @param dst   The destination
		* @param k     The number of paths
		* @return      A map represents the k-paths.
		*/
		std::vector<hrpPathEntry> findKShortestPath(const hrpNode& src, const hrpNode& dst, int k);
		static succMap convertKPathToSuccMap(const std::vector<hrpPathEntry> &paths);
		const lemon::ListDigraph &get_graph() const;
		const hrpNode& vertex(const lemon::ListDigraph::Node &n) { return _vertexNames[n]; }
		const std::map<hrpNode, lemon::ListDigraphBase::Node> &get_vertices() const;

		lemon::ListDigraph &get_mutable_graph() { return _graph; }
		std::map<hrpNode, lemon::ListDigraphBase::Node> &get_mutable_vertices() { return _vertices; };

		std::string pretty_print() const;

		private:
		std::shared_ptr<spdlog::logger> _logger;
		hrpPathEntry dijkstra(const hrpNode &src, const hrpNode& dst, lemon::SubDigraph<lemon::ListDigraph> &sg);
		std::mutex          _graph_mutex;
		lemon::ListDigraph _graph;
		lemon::ListDigraph::NodeMap<hrpNode> _vertexNames;
		lemon::ListDigraph::ArcMap<double> _arcLength;
		std::map<hrpNode, lemon::ListDigraph::Node> _vertices;
	};
}
#endif //HRP_HRPGRAPH_H
