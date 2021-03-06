//!
//! @file 			SafeNullLoggingTests.cpp
//! @author 		Geoffrey Hunter <gbmhunter@gmail.com> (www.cladlab.com)
//! @created		2014/04/09
//! @last-modified 	2014/05/19
//! @brief 			Tests that it is safe to "log" data if the debug and error ostream pointers havn't been assigned.
//! @details
//!					See README.rst in root dir for more info.

#include <iostream>
#include <fstream>
#include <cstdint>

#include "../api/CsvCpp.hpp"

#include "../lib/UnitTest++/src/UnitTest++.h"

namespace CsvCppTest
{
	SUITE(SafeNullLoggingTests)
	{

		TEST(SafeNullLoggingTest)
		{
			// Don't connect up error and debug streams
			CsvCpp::debugMsg.buff = NULL;
			CsvCpp::errorMsg.buff = NULL;

			// Even though the streams are NULL,
			// these calls should still be safe
			// (they won't do anything though)
			CsvCpp::debugMsg << "Testing!";
			CsvCpp::errorMsg << "Testing!";
		}

	} // SUITE(SafeNullLoggingTests)

} // namespace CsvCppTest
