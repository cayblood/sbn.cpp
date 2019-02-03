/*
 * node.cpp - Implementation of sbn::Node class
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


#include "sbn.h"


namespace sbn
{
	int Node::m_count = 0;


	Node::Node(const string& name)
	{
		if (name.empty()) m_name = "Node" + std::to_string(++m_count);
		else m_name = name;
	}


	Node::Node(const Node& node)
	{
		*this = node;
	}


	Node& Node::operator=(const Node& node)
	{
		if (this != &node)
		{
			m_name = node.m_name;
			m_probabilities = node.m_probabilities;
			m_parents = node.m_parents;
			m_children = node.m_children;
			m_states = node.m_states;
		}
		return *this;
	}


	string Node::get_name() const
	{
		return m_name;
	}


	void Node::add_state(const string& name)
	{
		m_states.push_back(name);
	}


	void Node::add_child(Node* child)
	{
		if (child == this) return;
		m_children.push_back(child);
		child->m_parents.push_back(this);
	}


	void Node::add_parent(Node* parent)
	{
		if (parent == this) return;
		m_parents.push_back(parent);
		parent->m_children.push_back(this);
	}


	// TODO: improve this method so that it recognizes when
	// a node has probabilities set for all states but one
	// and fills the last state in with (1 - sum_of_other_states)
	void Node::set_probability(Event e, double prob)
	{
		m_probabilities[e] = prob;
	}


	// A node can't be evaluated unless its parent nodes have
	// been observed
	bool Node::can_be_evaluated(Event& evidence)
	{
		if (m_parents.empty()) return true;

		for (NodeVector::iterator iter = m_parents.begin();
		     iter != m_parents.end();
		     ++iter)
		{
			if (!evidence.has_node((*iter)->get_name())) return false;
		}

		return true;
	}


	// In order to draw uniformly from the probabilty space, we can't
	// just pick a random state.  Instead we generate a random number
	// between zero and one and iterate through the states until the
	// cumulative sum of their probabilities exceeds our random number.
	string Node::get_random_state(Event& event)
	{
		double sum = 0.0;
		double num = ((double)random()) / RAND_MAX;
		string random_state;

		for (vector<string>::iterator iter = m_states.begin();
		     iter != m_states.end();
		     ++iter)
		{
			random_state = *iter;
			sum += evaluate_marginal(random_state, event);
			if (num < sum) break;
		}

		return random_state;
	}


	// similar to get_random_state() except it evaluates a node's markov
	// blanket in addition to the node itself.
	string Node::get_random_state_with_markov_blanket(Event& event)
	{
		double sum = 0.0;
		double num = ((double)random()) / RAND_MAX;
		vector<double> evaluations;
		string state;
		vector<string>::iterator iter;

		for (iter = m_states.begin(); iter != m_states.end(); ++iter)
		{
			double temp = evaluate_markov_blanket(*iter, event);
			evaluations.push_back(temp);
		}

		// normalize results
		double magnitude = 0.0;
		vector<double>::iterator num_iter;
		for (num_iter = evaluations.begin(); num_iter != evaluations.end(); ++num_iter)
		{
			magnitude += *num_iter;
		}
		transform(evaluations.begin(),
		          evaluations.end(),
		          evaluations.begin(),
		          std::bind2nd(std::divides<double>(), magnitude));

		num_iter = evaluations.begin();
		for (iter = m_states.begin(); iter != m_states.end(); ++iter)
		{
			state = *iter;
			sum += *num_iter;
			if (num < sum) break;
			++num_iter;
		}

		return state;
	}


	double Node::evaluate_marginal(const string& state, Event& event)
		throw(runtime_error)
	{
		ProbabilityMap prob = m_probabilities;
		ProbabilityMap::iterator iter;
		double returnval = 0.0;

		remove_irrelevant_states(prob, state, event);
		for (iter = prob.begin(); iter != prob.end(); ++iter)
		{
			returnval += iter->second;
		}

		return returnval;
	}


	double Node::evaluate_markov_blanket(const string& state, Event& event)
		throw(runtime_error)
	{
		ProbabilityMap prob = m_probabilities;
		ProbabilityMap::iterator iter;
		double returnval = 1.0;

		remove_irrelevant_states(prob, state, event);
		string temp = event.get_node_state(m_name);
		event.set_node(m_name, state);
		returnval *= evaluate_marginal(state, event);
		NodeVector::iterator node_iter;

		string childname;
		for (node_iter = m_children.begin(); node_iter != m_children.end(); ++node_iter)
		{
			childname = (*node_iter)->get_name();
			returnval *= (*node_iter)->evaluate_marginal(event.get_node_state(childname),
			                                             event);
		}
		event.set_node(m_name, temp);

		return returnval;
	}


	void Node::remove_irrelevant_states(ProbabilityMap& prob,
	                                    const string& state,
	                                    Event& evidence) throw(runtime_error)
	{
		// TODO: switch this over to use STL algorithms (remove_if)

		// remove probabilities that don't have the state we're looking for
		ProbabilityMap::iterator i, j;
		Event e;

		i = prob.begin();
		while (i != prob.end())
		{
			e = i->first;

			if (!e.node_has_state(m_name, state))
			{
				prob.erase(i++);
			}
			else ++i;
		}

		string parentname;
		NodeVector::iterator parent_iter;
		for (parent_iter = m_parents.begin();
		     parent_iter != m_parents.end(); ++parent_iter)
		{
			parentname = (*parent_iter)->get_name();

			if (!evidence.has_node(parentname))
				throw runtime_error("Marginal cannot be evaluated");

			// remove irrelevant parent states
			i = prob.begin();
			while (i != prob.end())
			{
				e = i->first;
				if (!e.node_has_state(parentname,
				                      evidence.get_node_state(parentname)))
				{
					prob.erase(i++);
				}
				else ++i;
			}
		}
	}


	Event& Node::next_combination(Event& event) throw(runtime_error)
	{
		Node *parent;
		string current_state;
		bool flipped = false;
		bool changed_state = false;
		vector<string>::iterator i, start, end;

		// Go through the parents in reverse order, incrementing each state.  If a
		// state is at the end of its list, reset it back to its first state, move
		// left and increment again.  Stop when a state is incremented normally.
		NodeVector::reverse_iterator iter = m_parents.rbegin();
		if (iter != m_parents.rend())
		{
			do
			{
				parent = *iter;
				current_state = event.get_node_state(parent->m_name);
				start = parent->m_states.begin();
				end = parent->m_states.end();
				i = find(start, end, current_state);
				if (i == end) throw runtime_error("Event contains invalid state");
				if (++i == end)
				{
					i = start;
					if (i == end) throw runtime_error("Encountered stateless node");
					flipped = true;
				}
				else flipped = false;
				event.set_node(parent->m_name, *i);
				changed_state = true;
				iter++;
			} while (iter != m_parents.rend() && flipped);
		}

		// if the most recently checked parent state was flipped, or if we haven't
		// yet changed a state, we need to increment the current node's state.
		if (flipped || !changed_state)
		{
			start = m_states.begin();
			end = m_states.end();
			current_state = event.get_node_state(m_name);
			i = find(start, end, current_state);
			if (i == end) throw runtime_error("Event contains invalid state");
			if (++i == end)
			{
				i = start;
				if (i == end) throw runtime_error("Encountered stateless node");
			}
			event.set_node(m_name, *i);
		}

		return event;
	}

}
