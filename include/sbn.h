/*
 * sbn.h - Main include file
 *
 * SBN - Simple Bayesian Networking library
 * Copyright (c) 2005 Carl Youngblood
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SBN_H__
#define __SBN_H__


#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>


namespace sbn
{
	/// Possible inference methods. Only MCMC has been implemented so far.
	enum { INFERENCE_MODE_EXACT,
	       INFERENCE_MODE_REJECTION_SAMPLING,
	       INFERENCE_MODE_LIKELIHOOD_WEIGHTING,
	       INFERENCE_MODE_MARKOV_CHAIN_MONTE_CARLO };

	// TODO: make inference parameters more customizeable
	static const int MCMC_NUM_SAMPLES = 1000;

	// bring some frequently-used classes into the namespace
	using std::string;
	using std::vector;
	using std::set;
	using std::map;
	using std::runtime_error;

	// forward declarations
	class Event;
	class Node;
	typedef map<string, string> ObservationMap;
	typedef map<Event, double> ProbabilityMap;
	typedef map<string, double> StateProbabilityMap;
	typedef map<string, Node*> NodeMap;
	typedef vector<Node*> NodeVector;

	/** Stores a possible configuration of variables in a Bayesian network, or a
	 * set of observed values for nodes in a network.
	 */
	class Event
	{
	public:
		/// Default constructor
		Event();

		/// Copy constructor
		Event(const Event& event);

		/// Copy assignment
		Event& operator=(const Event& event);

		/// Necessary for storing in std::map key
		bool operator<(const Event& event) const;

		/// For pretty printing
		operator const char *();

		/// Used to determine if a node is set in this event
		bool has_node(const string& nodename);

		/// Used to determine if a node is set to a specific state
		bool node_has_state(const string& nodename, const string& state);

		/// Used to retrieve the state of a node that has been set
		string get_node_state(const string& node) throw(runtime_error);

		/// Used to set observed variables for an event, also when setting the
		/// conditional probability tables for a node
		void set_node(const string& nodename, const string& state);

		/// Used to remove previously set variables from an event
		void remove_node(const string& nodename);

		/// Used to remove all previously set variables from an event
		void clear();

	private:
		void generate_string_representation();
		ObservationMap& get_observations();

		ObservationMap m_observations;
		string m_string_representation;

		// Make Net a friend so that it can call methods which should not be a part
		// of the public API, but which logically belong to this class
		friend class Net;
	};


	/** Stores information about the state of a variable in a Bayesian network
	 * and its relationship to other variables.
	 */
	class Node
	{
	public:
		/// Default constructor
		Node(const string& name = "");

		/// Copy constructor
		Node(const Node& node);

		/// Copy assignment
		Node& operator=(const Node& node);

		/// Returns node name
		string get_name() const;

		/// Adds another possible state that the node can be in
		void add_state(const string& name);

		/// Makes the specified node a child of this node
		void add_child(Node* child);

		/// Makes the specified node a parent of this node
		void add_parent(Node* parent);

		/// Sets the probability of an event (i.e., a combination of node states)
		void set_probability(Event e, double prob);

		/** Toggles an event to the next combination of possible states for this
		 * node.
	 	 *
	 	 * Net::next_combination() is used to iterate over all possible states for
	 	 * the parent nodes that have been set in an Event or evidence variable.
	 	 * If, for example, a node had two parents, and all three nodes could be in
	 	 * state T or state F, Net::next_combination() would cause the states in
	 	 * the Event variable to change in the following order:
	 	 *
	 	 * - 1 = T, 2 = T, 3 = T
	 	 * - 1 = T, 2 = T, 3 = F
	 	 * - 1 = T, 2 = F, 3 = T
	 	 * - 1 = T, 2 = F, 3 = F
	 	 * - 1 = F, 2 = T, 3 = T
	 	 * - 1 = F, 2 = T, 3 = F
	 	 * - 1 = F, 2 = F, 3 = T
	 	 * - 1 = F, 2 = F, 3 = F
	 	 */
	 	Event& next_combination(Event& event) throw(runtime_error);

	private:
		bool can_be_evaluated(Event& evidence);
		string get_random_state(Event& event);
		string get_random_state_with_markov_blanket(Event& event);
		void remove_irrelevant_states(ProbabilityMap& prob,
		                              const string& state,
		                              Event& evidence) throw(runtime_error);
		double evaluate_marginal(const string& state,
		                         Event& event) throw(runtime_error);
		double evaluate_markov_blanket(const string& state,
		                               Event& event) throw(runtime_error);

		static int m_count;
		string m_name;
		ProbabilityMap m_probabilities;

		// using vectors on these to preserve ordering information
		NodeVector m_parents;
		NodeVector m_children;
		vector<string> m_states;

		// Make Net a friend so that it can call methods which should not be a part
		// of the public API, but which logically belong to this class
		friend class Net;
	};


	/** Main interface class for a Bayesian network. It holds instances of the Node
	 * class, as well as observations that have been made about the state of the
	 * observed nodes in the network (called "evidence").
	 */
	class Net
	{
	public:

		/** Default Constructor
	 	 *
	 	 * Constructs a new network.  If a title for a network is not specified, it
	 	 * gets set to  Net1, Net2, etc.  depending on how many networks have already
	 	 * been created. Title is used to export network to a file--a feature that is
	 	 * still pending.
	 	 */
		Net(const string& title = "");

		/// Copy constructor
		Net(const Net& net);

    /// Assignment operator
		Net& operator=(const Net& net);

		/// Used to add newly-created nodes to a network.
		void add_node(Node *node);

		/// Used to indicate the observed states of some nodes in the network.
		void set_evidence(Event& e);

		/// Returns a probability for each possible state in the requested node.
		StateProbabilityMap query_node(string nodename);

	private:
		Event generate_random_event();

		static int m_count;

		string m_title;
		NodeMap m_nodes;
		Event m_evidence;
	};


} // namespace


#endif // __SBN_H__
