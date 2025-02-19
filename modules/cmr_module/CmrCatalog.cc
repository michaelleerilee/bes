// -*- mode: c++; c-basic-offset:4 -*-
//
// CMRCatalog.cc
//
// This file is part of BES cmr_module
//
// Copyright (c) 2018 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
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
// Please read the full copyright statement in the file COPYRIGHT_URI.
//

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <cstring>
#include <cerrno>

#include <sstream>
#include <cassert>

#include <memory>
#include <algorithm>

#include "BESUtil.h"
#include "BESCatalogUtils.h"
#include "BESCatalogEntry.h"

#include "CatalogNode.h"
#include "CatalogItem.h"

#include "BESInfo.h"
#include "BESContainerStorageList.h"
#include "BESFileContainerStorage.h"
#include "BESLog.h"

#include "BESInternalError.h"
#include "BESForbiddenError.h"
#include "BESNotFoundError.h"
#include "BESSyntaxUserError.h"

#include "TheBESKeys.h"
#include "BESDebug.h"

#include "CmrApi.h"
#include "CmrNames.h"
#include "CmrCatalog.h"

using namespace bes;
using namespace std;

#define prolog std::string("CmrCatalog::").append(__func__).append("() - ")

namespace cmr {

/**
 * @brief A catalog based on NASA's CMR system
 *
 * CMRCatalog is BESCatalog specialized for  NASA's CMR system.
 *
 * @note Access to the host's file system is made using BESCatalogUtils,
 * which is initialized using the catalog name.
 *
 * @param name The name of the catalog.
 * @see BESCatalogUtils
 */
CmrCatalog::CmrCatalog(const std::string &name /* = “CMR” */) : BESCatalog(name) {
    bool found = false;
    TheBESKeys::TheKeys()->get_values(CMR_COLLECTIONS, d_collections, found);
    if(!found){
        throw BESInternalError(string("The CMR module must define at least one collection name using the key; '")+CMR_COLLECTIONS
            +"'", __FILE__, __LINE__);
    }

    found = false;
    TheBESKeys::TheKeys()->get_values(CMR_FACETS, d_facets, found);
    if(!found){
        throw BESInternalError(string("The CMR module must define at least one facet name using the key; '")+CMR_COLLECTIONS
            +"'", __FILE__, __LINE__);
    }
}

CmrCatalog::~CmrCatalog()
{
}
bes::CatalogNode *
CmrCatalog::get_node(const string &path) const
{
    return get_node_NEW(path);
}


bes::CatalogNode *
CmrCatalog::get_node_NEW(const string &ppath) const
{
    string path = BESUtil::normalize_path(ppath,true, false);
    vector<string> path_elements = BESUtil::split(path);
    BESDEBUG(MODULE, prolog << "path: '" << path << "'   path_elements.size(): " << path_elements.size() << endl);

    string epoch_time = BESUtil::get_time(0,false);

    CmrApi cmrApi;
    bes::CatalogNode *node;

    if(path_elements.empty()){
        node = new CatalogNode("/");
        node->set_lmt(epoch_time);
        node->set_catalog_name(CMR_CATALOG_NAME);
        for(size_t i=0; i<d_collections.size() ; i++){
            CatalogItem *collection = new CatalogItem();
            collection->set_name(d_collections[i]);
            collection->set_type(CatalogItem::node);
            node->add_node(collection);
        }
    }
    else {
        for(size_t i=0; i< path_elements.size() ;i++){
            if(path_elements[i]=="-")
                path_elements[i] = "";
        }

        string collection = path_elements[0];
        BESDEBUG(MODULE, prolog << "Checking for collection: " << collection << " d_collections.size(): " << d_collections.size() << endl);
        bool valid_collection = false;
        for(size_t i=0; i<d_collections.size() && !valid_collection ; i++){
            if(collection == d_collections[i])
                valid_collection = true;
        }
        if(!valid_collection){
            throw BESNotFoundError("The CMR catalog does not contain a collection named '"+collection+"'",__FILE__,__LINE__);
        }
        BESDEBUG(MODULE, prolog << "Collection " << collection << " is valid." << endl);
        if(path_elements.size() >1){
            string facet = path_elements[1];
            bool valid_facet = false;
            for(size_t i=0; i<d_facets.size() && !valid_facet ; i++){
                if(facet == d_facets[i])
                    valid_facet = true;
            }
            if(!valid_facet){
                throw BESNotFoundError("The CMR collection '"+collection+"' does not contain a facet named '"+facet+"'",__FILE__,__LINE__);
            }

            if(facet=="temporal"){
                BESDEBUG(MODULE, prolog << "Found Temporal Facet"<< endl);
                node = new CatalogNode(path);
                node->set_lmt(epoch_time);
                node->set_catalog_name(CMR_CATALOG_NAME);


                switch( path_elements.size()){

                case 2: // The path ends at temporal facet, so we need the years.
                {
                    vector<string> years;

                    BESDEBUG(MODULE, prolog << "Getting year nodes for collection: " << collection<< endl);
                    cmrApi.get_years(collection, years);
                    for(size_t i=0; i<years.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(years[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                break;

                case 3:
                {
                    string year = path_elements[2];
                    string month("");
                    string day("");
                    vector<string> months;

                    BESDEBUG(MODULE, prolog << "Getting month nodes for collection: " << collection << " year: " << year << endl);
                    cmrApi.get_months(collection, year, months);
                    for(size_t i=0; i<months.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(months[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                break;

                case 4:
                {
                    string year = path_elements[2];
                    string month = path_elements[3];
                    string day("");
                    vector<string> days;

                    BESDEBUG(MODULE, prolog << "Getting day nodes for collection: " << collection << " year: " << year << " month: " << month << endl);
                    cmrApi.get_days(collection, year, month, days);
                    for(size_t i=0; i<days.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(days[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                break;

                case 5:
                {
                    string year = path_elements[2];
                    string month = path_elements[3];
                    string day = path_elements[4];
                    BESDEBUG(MODULE, prolog << "Getting granule leaves for collection: " << collection << " year: " << year << " month: " << month <<  " day: " << day << endl);
                    vector<Granule *> granules;
                    cmrApi.get_granules(collection, year, month, day, granules);
                    for(size_t i=0; i<granules.size() ; i++){
                        node->add_leaf(granules[i]->getCatalogItem(get_catalog_utils()));
                    }
                }
                break;

                case 6:
                {
                    string year = path_elements[2];
                    string month = path_elements[3];
                    string day = path_elements[4];
                    string granule_id = path_elements[5];
                    BESDEBUG(MODULE, prolog << "Request resolved to leaf granule/dataset name,  collection: " << collection << " year: " << year
                        << " month: " << month <<  " day: " << day << " granule: " << granule_id << endl);
                    Granule *granule = cmrApi.get_granule(collection,year,month,day,granule_id);
                    if(granule){
                        CatalogItem *granuleItem = new CatalogItem();
                        granuleItem->set_type(CatalogItem::leaf);
                        granuleItem->set_name(granule->getName());
                        granuleItem->set_is_data(true);
                        granuleItem->set_lmt(granule->getLastModifiedStr());
                        granuleItem->set_size(granule->getSize());
                        node->set_leaf(granuleItem);
                    }
                    else {
                        throw BESNotFoundError("No such resource: "+path,__FILE__,__LINE__);
                    }
                }
                break;

                default:
                {
                    throw BESSyntaxUserError("CmrCatalog: The path '"+path+"' does not describe a valid temporal facet search.",__FILE__,__LINE__);
                }
                break;
                }

            }
            else {
                throw BESNotFoundError("The CMR catalog only supports temporal faceting.",__FILE__,__LINE__);
            }
        }
        else {
            BESDEBUG(MODULE, prolog << "Building facet list for collection: " << collection << endl);
            node = new CatalogNode(path);
            node->set_lmt(epoch_time);
            node->set_catalog_name(CMR_CATALOG_NAME);
            for(size_t i=0; i<d_facets.size() ; i++){
                CatalogItem *collection = new CatalogItem();
                collection->set_name(d_facets[i]);
                collection->set_type(CatalogItem::node);
                collection->set_lmt(epoch_time);
                BESDEBUG(MODULE, prolog << "Adding facet: " << d_facets[i] << endl);
                node->add_node(collection);
            }
        }
    }
    return node;
}


// path must start with a '/'. By this class it will be interpreted as a
// starting at the CatalogDirectory instance's root directory. It may either
// end in a '/' or not.
//
// If it is not a directory - that is an error. (return null or throw?)
//
// Item names are relative
/**
 * @brief Get a CatalogNode for the given path in the current catalog
 *
 * This is similar to show_catalog() but returns a simpler response. The
 * \arg path must start with a slash and is used as a suffix to the Catalog's
 * root directory.
 *
 * @param path The pathname for the node; must start with a slash
 * @return A CatalogNode instance or null if there is no such path in the
 * current catalog.
 * @throw BESInternalError If the \arg path is not a directory
 * @throw BESForbiddenError If the \arg path is explicitly excluded by the
 * bes.conf file
 */
bes::CatalogNode *
CmrCatalog::get_node_OLD(const string &ppath) const
{
    string path = BESUtil::normalize_path(ppath,true, false);
    vector<string> path_elements = BESUtil::split(path);
    BESDEBUG(MODULE, prolog << "path: '" << path << "'   path_elements.size(): " << path_elements.size() << endl);

    string epoch_time = BESUtil::get_time(0,false);

    CmrApi cmrApi;
    bes::CatalogNode *node;

    if(path_elements.empty()){
        node = new CatalogNode("/");
        node->set_lmt(epoch_time);
        node->set_catalog_name(CMR_CATALOG_NAME);
        for(size_t i=0; i<d_collections.size() ; i++){
            CatalogItem *collection = new CatalogItem();
            collection->set_name(d_collections[i]);
            collection->set_type(CatalogItem::node);
            node->add_node(collection);
        }
    }
    else {
        string collection = path_elements[0];
        BESDEBUG(MODULE, prolog << "Checking for collection: " << collection << " d_collections.size(): " << d_collections.size() << endl);
        bool valid_collection = false;
        for(size_t i=0; i<d_collections.size() && !valid_collection ; i++){
            if(collection == d_collections[i])
                valid_collection = true;
        }
        if(!valid_collection){
            throw BESNotFoundError("The CMR catalog does not contain a collection named '"+collection+"'",__FILE__,__LINE__);
        }
        BESDEBUG(MODULE, prolog << "Collection " << collection << " is valid." << endl);
        if(path_elements.size() >1){
            string facet = path_elements[1];
            bool valid_facet = false;
            for(size_t i=0; i<d_facets.size() && !valid_facet ; i++){
                if(facet == d_facets[i])
                    valid_facet = true;
            }
            if(!valid_facet){
                throw BESNotFoundError("The CMR collection '"+collection+"' does not contain a facet named '"+facet+"'",__FILE__,__LINE__);
            }

            if(facet=="temporal"){
                BESDEBUG(MODULE, prolog << "Found Temporal Facet"<< endl);
                node = new CatalogNode(path);
                node->set_lmt(epoch_time);
                node->set_catalog_name(CMR_CATALOG_NAME);


                switch( path_elements.size()){
                case 2: // The path ends at temporal facet, so we need the years.
                {
                    vector<string> years;

                    BESDEBUG(MODULE, prolog << "Getting year nodes for collection: " << collection<< endl);
                    cmrApi.get_years(collection, years);
                    for(size_t i=0; i<years.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(years[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                    break;
                case 3:
                {
                    string year = path_elements[2];
                    string month("");
                    string day("");
                    vector<string> months;

                    BESDEBUG(MODULE, prolog << "Getting month nodes for collection: " << collection << " year: " << year << endl);
                    cmrApi.get_months(collection, year, months);
                    for(size_t i=0; i<months.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(months[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                    break;
                case 4:
                {
                    string year = path_elements[2];
                    string month = path_elements[3];
                    string day("");
                    vector<string> days;

                    BESDEBUG(MODULE, prolog << "Getting day nodes for collection: " << collection << " year: " << year << " month: " << month << endl);
                    cmrApi.get_days(collection, year, month, days);
                    for(size_t i=0; i<days.size() ; i++){
                        CatalogItem *collection = new CatalogItem();
                        collection->set_type(CatalogItem::node);
                        collection->set_name(days[i]);
                        collection->set_is_data(false);
                        collection->set_lmt(epoch_time);
                        collection->set_size(0);
                        node->add_node(collection);
                    }
                }
                    break;
                case 5:
                {
                    string year = path_elements[2];
                    string month = path_elements[3];
                    string day = path_elements[4];
                    BESDEBUG(MODULE, prolog << "Getting granule leaves for collection: " << collection << " year: " << year << " month: " << month <<  " day: " << day << endl);
                    vector<Granule *> granules;
                    cmrApi.get_granules(collection, year, month, day, granules);
                    for(size_t i=0; i<granules.size() ; i++){
                        node->add_leaf(granules[i]->getCatalogItem(get_catalog_utils()));
                    }
                }
                    break;
                default:
                    throw BESSyntaxUserError("CmrCatalog: The path '"+path+"' does not describe a valid temporal facet search.",__FILE__,__LINE__);
                    break;
                }
            }
            else {
                throw BESNotFoundError("The CMR catalog only supports temporal faceting.",__FILE__,__LINE__);
            }
        }
        else {
            BESDEBUG(MODULE, prolog << "Building facet list for collection: " << collection << endl);
            node = new CatalogNode(path);
            node->set_lmt(epoch_time);
            node->set_catalog_name(CMR_CATALOG_NAME);
            for(size_t i=0; i<d_facets.size() ; i++){
                CatalogItem *collection = new CatalogItem();
                collection->set_name(d_facets[i]);
                collection->set_type(CatalogItem::node);
                collection->set_lmt(epoch_time);
                BESDEBUG(MODULE, prolog << "Adding facet: " << d_facets[i] << endl);
                node->add_node(collection);
            }
        }
    }
    return node;
}

#if 0
bes::CatalogNode *
CmrCatalog::get_node(const string &path) const
{

    string rootdir = d_utils->get_root_dir();

    // This will throw the appropriate exception (Forbidden or Not Found).
    // Checks to make sure the different elements of the path are not
    // symbolic links if follow_sym_links is set to false, and checks to
    // make sure have permission to access node and the node exists.
    BESUtil::check_path(path, rootdir, d_utils->follow_sym_links());

    string fullpath = rootdir + path;

    DIR *dip = opendir(fullpath.c_str());
    if (!dip)
        throw BESInternalError(
            "A CMRCatalog can only return nodes for directory. The path '" + path
                + "' is not a directory for BESCatalog '" + get_catalog_name() + "'.", __FILE__, __LINE__);

    try {
        // The node is a directory

        // Based on other code (show_catalogs()), use BESCatalogUtils::exclude() on
        // a directory, but BESCatalogUtils::include() on a file.
        if (d_utils->exclude(path))
            throw BESForbiddenError(
                string("The path '") + path + "' is not included in the catalog '" + get_catalog_name() + "'.",
                __FILE__, __LINE__);

        CatalogNode *node = new CatalogNode(path);

        node->set_catalog_name(get_catalog_name());
        struct stat buf;
        int statret = stat(fullpath.c_str(), &buf);
        if (statret == 0 /* && S_ISDIR(buf.st_mode) */)
            node->set_lmt(get_time(buf.st_mtime));

        struct dirent *dit;
        while ((dit = readdir(dip)) != NULL) {
            string item = dit->d_name;
            if (item == "." || item == "..") continue;

            string item_path = fullpath + "/" + item;

            // TODO add a test in configure for the readdir macro(s) DT_REG, DT_LNK
            // and DT_DIR and use those, if present, to determine if the name is a
            // link, directory or regular file. These are not present on all systems.
            // Also, since we need mtime, this is not a huge time saver. But if we
            // decide not to use the mtime, using these macros could save lots of system
            // calls. jhrg 3/9/18

            // Skip this dir entry if it is a sym link and follow links is false
            if (d_utils->follow_sym_links() == false) {
                struct stat lbuf;
                (void) lstat(item_path.c_str(), &lbuf);
                if (S_ISLNK(lbuf.st_mode)) continue;
            }

            // Is this a directory or a file? Should it be excluded or included?
            statret = stat(item_path.c_str(), &buf);
            if (statret == 0 && S_ISDIR(buf.st_mode) && !d_utils->exclude(item)) {
#if 0
                // Add a new node; set the size to zero.
                node->add_item(new CatalogItem(item, 0, get_time(buf.st_mtime), CatalogItem::node));
#endif
                node->add_node(new CatalogItem(item, 0, get_time(buf.st_mtime), CatalogItem::node));
            }
            else if (statret == 0 && S_ISREG(buf.st_mode) && d_utils->include(item)) {
#if 0
                // Add a new leaf.
                node->add_item(new CatalogItem(item, buf.st_size, get_time(buf.st_mtime),
                    d_utils->is_data(item), CatalogItem::leaf));
#endif
                node->add_leaf(new CatalogItem(item, buf.st_size, get_time(buf.st_mtime),
                    d_utils->is_data(item), CatalogItem::leaf));
            }
            else {
                VERBOSE("Excluded the item '" << item_path << "' from the catalog '" << get_catalog_name() << "' node listing.");
            }
        } // end of the while loop

        closedir(dip);

        CatalogItem::CatalogItemAscending ordering;

        sort(node->nodes_begin(), node->nodes_end(), ordering);
        sort(node->leaves_begin(), node->leaves_end(), ordering);

        return node;
    }
    catch (...) {
        closedir(dip);
        throw;
    }
}
#endif


/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance along with information about
 * this catalog directory.
 *
 * @param strm C++ i/o stream to dump the information to
 */
void CmrCatalog::dump(ostream &strm) const
{
    strm << BESIndent::LMarg << prolog << "(" << (void *) this << ")" << endl;
    BESIndent::Indent();

    strm << BESIndent::LMarg << "catalog utilities: " << endl;
    BESIndent::Indent();
    get_catalog_utils()->dump(strm);
    BESIndent::UnIndent();
    BESIndent::UnIndent();
}

} // namespace cmr
