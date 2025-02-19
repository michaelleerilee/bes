
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2013 OPeNDAP, Inc.
// Authors: Nathan Potter <npotter@opendap.org>
//		James Gallagher <jgallagher@opendap.org>
//
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

#include <ServerFunction.h>

namespace libdap {
class BaseType;
class DDS;
}

namespace functions {

void function_bind_shape_dap2(int argc, libdap::BaseType *argv[], libdap::DDS &dds, libdap::BaseType **btpp) ;
libdap::BaseType *function_bind_shape_dap4(libdap::D4RValueList *args, libdap::DMR &dmr);

class BindShapeFunction: public libdap::ServerFunction {
public:
	BindShapeFunction()
    {
		setName("bind_shape");
		setDescriptionString("The bind_shape() function sets the shape of a DAP Array.");
		setUsageString("bind_shape(shape,variable)");
		setRole("http://services.opendap.org/dap4/server-side-function/bind_shape");
		setDocUrl("http://docs.opendap.org/index.php/Server_Side_Processing_Functions#bind_shape");
		setFunction(function_bind_shape_dap2);
		setFunction(function_bind_shape_dap4);
		setVersion("1.0");
    }
    virtual ~BindShapeFunction()
    {
    }
};

} // functions namespace
