// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// stareMappingTest.cc
// Copyright (c) 2016 OPeNDAP, Inc.
// Created on: Mar 28, 2019
//		Author: Kodi Neumiller <kneumiller@opendap.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.


#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include "config.h"

#include "SpatialIndex.h"
#include "SpatialInterface.h"

#include <D4Connect.h>
#include <Connect.h>
#include <Response.h>
#include <Array.h>

#include <Error.h>

namespace bes {

class stareMappingTest: public TestFixture {
private:
	DDS *dds1, *dds2;
	BaseTypeFactory factory;

public:
	stareMappingTest() :
		dds1(0), dds2(0), dds_cache(0), dds(0)
	{
	}
	~stareMappingTest()
	{
	}

	void setup()
	{
		dds1 = new DDS(&factory, "test1");
		dds2 = new DDS(&factory, "test2");

		DBG2(cerr << "setUp() - BEGIN" << endl);

		dds_cache = new ObjMemCache;

		// Load in 10 DDS*s and then purge
		BaseTypeFactory factory;
		auto_ptr<DDS> dds(new DDS(&factory, "empty_DDS"));

		ostringstream oss;
		for (int i = 0; i < 10; ++i) {
			oss << i << "_DDS";
			string name = oss.str();
			DBG2(cerr << "Adding name: " << name << endl);
			// Need to add new pointers since the cache will delete them
			dds_cache->add(new DDS(*dds.get()), name);
			oss.str("");
		}

		DBG2(dds_cache->dump(cerr));

		DBG2(cerr << "setUp() - END" << endl);
	}

	void tearDown()
	{
		delete dds1;
		dds1 = 0;
		delete dds2;
		dds2 = 0;

		delete dds_cache;
		delete dds;
	}

	void stare_index_test()
	{
		int lat = 0;
		int lon = 0;
		int expectedId = 4431095428099621448;

		float64 level = 8;
		float64 buildlevel = 8;
		htmInterface htm(level, buildlevel);
		const SpatialIndex &index = htm.index();

		int stareIndex = index.idByLatLon(lat, lon);

		CPPUNIT_ASSERT(stareIndex == expectedId);
	}

	void mapping_test()
	{

	}

	void write_hdf5_test()
	{

	}

CPPUNIT_TEST_SUITE( stareMappingTest );

	CPPUNIT_TEST(mapping_test);

	CPPUNIT_TEST_SUITE_END()
	;
};

int main(int argc, char argv*[])
{
	bool wasSuccessful = true;
	    string test = "";
	    int i = getopt.optind;
	    if (i == argc) {
	        // run them all
	        wasSuccessful = runner.run("");
	    }

	return wasSuccessful ? 0 : 1;
}
}
