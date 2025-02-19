// BESDDXResponseHandler.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004-2009 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
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
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "config.h"

#include <memory>

#include <DDS.h>

#include "GlobalMetadataStore.h"
#include "BESDDXResponseHandler.h"
#include "BESDDSResponse.h"
#include "BESDapNames.h"
#include "BESTransmitter.h"
#include "BESRequestHandlerList.h"

#include "BESDebug.h"

using namespace libdap;
using namespace bes;

BESDDXResponseHandler::BESDDXResponseHandler(const string &name) :
    BESResponseHandler(name)
{
}

BESDDXResponseHandler::~BESDDXResponseHandler()
{
}

/**
 * Is there a function call in this CE? This function tests for a left paren
 * to indicate that a function is present. The CE will still be encoded at
 * this point, so test for the escape characters (%28) too.
 *
 * @note I copied this from BESDDSResponseHandler. I think there is probably
 * a better solution to this whole issue of CEs and functions, maybe in
 * BESContainer.
 *
 * @param ce The Constraint expression to test
 * @return True if there is a function call, false otherwise
 */
static bool function_in_ce(const string &ce)
{
    // 0x28 is '('
    return ce.find("(") != string::npos || ce.find("%28") != string::npos;   // hack
}

/** @brief executes the command 'get ddx for def_name;'
 *
 * For each container in the specified definition go to the request
 * handler for that container and have it first add to the OPeNDAP DDS response
 * object. Once the DDS object has been filled in, repeat the process but
 * this time for the OPeNDAP DAS response object. Then add the attributes from
 * the DAS object to the DDS object.
 *
 * @param dhi structure that holds request and response information
 * @see BESDataHandlerInterface
 * @see BESDDSResponse
 * @see BESDASResponse
 * @see BESRequestHandlerList
 */
void BESDDXResponseHandler::execute(BESDataHandlerInterface &dhi)
{
    BESDEBUG("dap", "Entering BESDDXResponseHandler::execute" << endl);

    dhi.action_name = DDX_RESPONSE_STR;

    GlobalMetadataStore *mds = GlobalMetadataStore::get_instance();
    GlobalMetadataStore::MDSReadLock lock;

    dhi.first_container();
    if (mds) lock = mds->is_dds_available(dhi.container->get_relative_name());

    if (mds && lock() && !function_in_ce(dhi.container->get_constraint())) {
        DDS *dds = mds->get_dds_object(dhi.container->get_relative_name());
        BESDDSResponse *bdds = new BESDDSResponse(dds);

        dds->set_request_xml_base(bdds->get_request_xml_base());

        bdds->set_constraint(dhi);
        bdds->clear_container();

        d_response_object = bdds;
    }
    else {
        // Make a blank DDS. It is the responsibility of the specific request
        // handler to set the BaseTypeFactory. It is set to NULL here
        DDS *dds = new DDS(NULL, "virtual");

        BESDDSResponse *bdds = new BESDDSResponse(dds);
        d_response_name = DDS_RESPONSE;
        dhi.action = DDS_RESPONSE;

        dds->set_request_xml_base(bdds->get_request_xml_base());

        d_response_object = bdds;

        BESRequestHandlerList::TheList()->execute_each(dhi);

        dhi.first_container();  // must reset container; execute_each() iterates over all of them

#if ANNOTATION_SYSTEM
            // Support for the experimental Dataset Annotation system. jhrg 12/19/18
            if (!d_annotation_service_url.empty()) {
                // resp_dds is a convenience object
                BESDDSResponse *resp_dds = static_cast<BESDDSResponse*>(d_response_object);

                // Add the Annotation Service URL attribute in the DODS_EXTRA container.
                AttrTable *dods_extra = resp_dds->get_dds()->get_attr_table().find_container(DODS_EXTRA_ATTR_TABLE);
                if (dods_extra)
                    dods_extra->append_attr(DODS_EXTRA_ANNOTATION_ATTR, "String", d_annotation_service_url);
                else {
                    auto_ptr<AttrTable> new_dods_extra(new AttrTable);
                    new_dods_extra->append_attr(DODS_EXTRA_ANNOTATION_ATTR, "String", d_annotation_service_url);
                    resp_dds->get_dds()->get_attr_table().append_container(new_dods_extra.release(), DODS_EXTRA_ATTR_TABLE);
                }
            }
#endif

            if (mds && !function_in_ce(dhi.container->get_constraint())) {
            // dhi.first_container();  // must reset container; execute_each() iterates over all of them
            mds->add_responses(static_cast<BESDDSResponse*>(d_response_object)->get_dds(), dhi.container->get_relative_name());
        }
    }

#if 0
    bdds = new BESDDSResponse(dds);
    d_response_object = bdds;
    d_response_name = DDS_RESPONSE;
    dhi.action = DDS_RESPONSE;

    BESDEBUG("bes", "about to set dap version to: " << bdds->get_dap_client_protocol() << endl);
    BESDEBUG("bes", "about to set xml:base to: " << bdds->get_request_xml_base() << endl);

    if (!bdds->get_dap_client_protocol().empty()) {
        dds->set_dap_version(bdds->get_dap_client_protocol());
    }

    dds->set_request_xml_base(bdds->get_request_xml_base());

    BESRequestHandlerList::TheList()->execute_each(dhi);

    dhi.action = DDX_RESPONSE;
    d_response_object = bdds;
#endif

    BESDEBUG("dap", "Leaving BESDDXResponseHandler::execute" << endl);
}

/** @brief transmit the response object built by the execute command
 *
 * If a response object was built then transmit it using the send_ddx method
 * on the specified transmitter object.
 *
 * @param transmitter object that knows how to transmit specific basic types
 * @param dhi structure that holds the request and response information
 * @see DDS
 * @see DAS
 * @see BESTransmitter
 * @see BESDataHandlerInterface
 */
void BESDDXResponseHandler::transmit(BESTransmitter * transmitter, BESDataHandlerInterface & dhi)
{
    if (d_response_object) {
        transmitter->send_response(DDX_SERVICE, d_response_object, dhi);
    }
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void BESDDXResponseHandler::dump(ostream &strm) const
{
    strm << BESIndent::LMarg << "BESDDXResponseHandler::dump - (" << (void *) this << ")" << endl;
    BESIndent::Indent();
    BESResponseHandler::dump(strm);
    BESIndent::UnIndent();
}

BESResponseHandler *
BESDDXResponseHandler::DDXResponseBuilder(const string &name)
{
    return new BESDDXResponseHandler(name);
}

