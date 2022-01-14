#include <iostream>
#include <string>

#include "deduce.hpp"

std::string Deduce::deduceServerFile() {
	std::cout << "Testing the deducer! It currently does nothing" << std::endl;
	return std::string("foo");
}

std::string Deduce::deduceUsagePattern() {
	return std::string("bar");
}

Deduce deducer;
