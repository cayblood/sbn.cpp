/*
 * sbntest.cpp - Test program
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

#include <math.h>
#include "sbn.h"

using namespace sbn;
using std::cout;
using std::endl;

int main(int argc, char **argv)
{
	// declare net and nodes
	Net net("Grass Wetness Belief Net");
	Node cloudy("Cloudy"),
	     sprinkler("Sprinkler"),
	     rain("Rain"),
	     grasswet("GrassWet");

	// add all possible states to nodes
	cloudy.add_state("T");
	cloudy.add_state("F");
	sprinkler.add_state("T");
	sprinkler.add_state("F");
	rain.add_state("T");
	rain.add_state("F");
	grasswet.add_state("T");
	grasswet.add_state("F");

	// add nodes to network
	net.add_node(&cloudy);
	net.add_node(&sprinkler);
	net.add_node(&rain);
	net.add_node(&grasswet);

	// link nodes together
	cloudy.add_child(&sprinkler);
	cloudy.add_child(&rain);
	sprinkler.add_child(&grasswet);
	rain.add_child(&grasswet);

	// Set probabilities for all states. State conditions are
	// expressed with instances of class Event.
	Event e;
	e.set_node("Cloudy", "T");
	cloudy.set_probability(e, 0.5); // cloudy = T
	cout << e << " = 0.5" << endl;
	cloudy.set_probability(cloudy.next_combination(e), 0.5); // cloudy = F
	cout << e << " = 0.5" << endl;

	e.set_node("Sprinkler", "T"); // e now expresses event cloudy = F AND sprinkler = T
	sprinkler.set_probability(e, 0.5);                    // F, T
	cout << e << " = 0.5" << endl;
	sprinkler.set_probability(sprinkler.next_combination(e), 0.5); // F, F
	cout << e << " = 0.5" << endl;
	sprinkler.set_probability(sprinkler.next_combination(e), 0.1); // T, T
	cout << e << " = 0.1" << endl;
	sprinkler.set_probability(sprinkler.next_combination(e), 0.9); // T, F
	cout << e << " = 0.9" << endl;
	
	e.remove_node("Sprinkler");
	e.set_node("Rain", "T"); // cloudy = T AND rain = T
	rain.set_probability(e, 0.8);                     // T, T
	cout << e << " = 0.8" << endl;
	rain.set_probability(rain.next_combination(e), 0.2);  // T, F
	cout << e << " = 0.2" << endl;
	rain.set_probability(rain.next_combination(e), 0.2);  // F, T 
	cout << e << " = 0.2" << endl;
	rain.set_probability(rain.next_combination(e), 0.8);  // F, F
	cout << e << " = 0.8" << endl;
	
	e.remove_node("Cloudy");
	e.set_node("GrassWet", "T");
	e.set_node("Sprinkler", "T"); // grasswet = T AND sprinkler = T AND rain = F
	grasswet.set_probability(e, 0.90);                    // T, T, F
	cout << e << " = 0.9" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.90); // T, F, T 
	cout << e << " = 0.9" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.00); // T, F, F
	cout << e << " = 0.0" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 1.0);  // F, F, F
	cout << e << " = 1.0" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.1);  // F, F, T 
	cout << e << " = 0.1" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.1);  // F, T, F 
	cout << e << " = 0.1" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.01); // F, T, T 
	cout << e << " = 0.01" << endl;
	grasswet.set_probability(grasswet.next_combination(e), 0.99); // T, T, T 
	cout << e << " = 0.99" << endl;

	// run a sample query
	e.clear();
	e.set_node("Sprinkler", "F");
	e.set_node("Rain", "T");
	map<string, double> result;
	net.set_evidence(e);
	result = net.query_node("GrassWet");

	cout.precision(3);
	cout << std::showpoint;
	map<string, double>::iterator iter = result.begin();
	while (iter != result.end())
	{
		std::cout <<
			"Posterior probability of GrassWet = " <<
			iter->first <<
			" given " << e << " is " <<
			iter->second <<
			std::endl;
		++iter;
	}

	// verify that results are correct
	if (round(result["T"] * 10) == 9.0 && round(result["F"] * 10) == 1.0)
		return 0; // success
	
	return 1; // failure
}
