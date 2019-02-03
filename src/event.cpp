/*
 * event.cpp - Implementation of Event class 
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

	Event::Event() { }
	Event::Event(const Event& event) { *this = event; }
	Event& Event::operator=(const Event& event)
	{
		if (this != &event)
		{
			m_observations = event.m_observations;
			generate_string_representation();
		}
		return *this;
	}


	bool Event::operator<(const Event& event) const
	{
		return m_string_representation < event.m_string_representation;
	}


	void Event::generate_string_representation()
	{
		m_string_representation = "";
		for (ObservationMap::iterator iter = m_observations.begin();
		     iter != m_observations.end();
		     ++iter)
		{
			if (m_string_representation.length() > 0)
				m_string_representation += ", ";
			m_string_representation += iter->first + " = " + iter->second;
		}
	}
	

	// for pretty printing
	Event::operator const char *()
	{
		return m_string_representation.c_str();
	}


	string Event::get_node_state(const string& nodename) throw(runtime_error)
	{
		ObservationMap::iterator iter = m_observations.find(nodename);
		if (iter == m_observations.end()) throw runtime_error("Invalid node");
		return iter->second;
	}


	bool Event::has_node(const string& nodename)
	{
		ObservationMap::iterator iter = m_observations.find(nodename);
		if (iter != m_observations.end()) return true;
		return false;
	}


	bool Event::node_has_state(const string& nodename, const string& state)
	{
		ObservationMap::iterator iter = m_observations.find(nodename);
		if (iter != m_observations.end())
		{
			if (iter->second == state) return true;
		}
		return false;
	}


	void Event::set_node(const string& nodename, const string& state)
	{
		m_observations[nodename] = state;
		generate_string_representation();
	}


	void Event::remove_node(const string& nodename)
	{
		ObservationMap::iterator iter = m_observations.find(nodename);
		if (iter != m_observations.end())
		{
			m_observations.erase(iter);
		}
		generate_string_representation();
	}


	void Event::clear()
	{
		m_observations.clear();
		generate_string_representation();
	}


	ObservationMap& Event::get_observations()
	{
		return m_observations;
	}

}
