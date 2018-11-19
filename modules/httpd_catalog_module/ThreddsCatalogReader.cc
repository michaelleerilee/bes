// HttpdDirScraper.cc
// -*- mode: c++; c-basic-offset:4 -*-
//
// This file is part of httpd_catalog_module, A C++ module that can be loaded in to
// the OPeNDAP Back-End Server (BES) and is able to handle remote requests.
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>     /* atol */
#include <string.h>
#include <ctype.h> /* isalpha and isdigit */
#include <time.h> /* mktime */

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <util.h>
#include <BESDebug.h>
#include <BESUtil.h>
#include <BESRegex.h>
#include <BESXMLUtils.h>
#include <BESSyntaxUserError.h>

#include <BESCatalogList.h>
#include <BESCatalogUtils.h>
#include <CatalogItem.h>

#include "RemoteHttpResource.h"
#include "HttpdCatalogNames.h"

#include "ThreddsCatalogReader.h"

using namespace std;
using bes::CatalogItem;

#define prolog std::string("ThreddsCatalogReader::").append(__func__).append("() - ")

#define CATALOG "catalog"
#define DATASET "dataset"
#define SERVICE "service"
#define METADATA "metadata"
#define CATALOG_REF "catalogRef"
#define DATA_SIZE "dataSize"
#define DATE "date"
#define ACCESS "access"
#define UNITS "units"
#define SERVICE_NAME "serviceName"
#define URL_PATH "urlPath"
#define DATA_FORMAT "dataFormat"
#define TYPE "type"
#define HREF "href"
#define TITLE "title"
#define ID "ID"
#define SERVICE_TYPE "serviceType"
#define BASE "base"
#define NAME "name"

namespace httpd_catalog {

ThreddsAccess::ThreddsAccess(xmlNodePtr accessElement, ThreddsService *defaultService):d_access(accessElement)
{
    // Find File and DAP services.
    string aname(""), avalue("");
    map<string, string> attrs;
    map<string, string>::iterator attrIt;
    BESXMLUtils::GetNodeInfo(accessElement, aname, avalue, attrs);
    // thredds:access children
    // ./dataSize
    // @urlPath
    // @serviceName
    // @dataFormat

    urlPath = dataFormat = "";
    attrIt = attrs.find(URL_PATH);
    if(attrIt!= attrs.end()){
        urlPath = attrIt->second;
    }

    string serviceName;
    attrIt = attrs.find(SERVICE_NAME);
    if(attrIt!= attrs.end()){
        serviceName = attrIt->second;
    }

    if(serviceName.empty() && defaultService){
        service = defaultService;
    }
    else {

    }




    attrIt = attrs.find(DATA_FORMAT);
    if(attrIt!= attrs.end()){
        dataFormat = attrIt->second;
    }
}
string ThreddsAccess::getAccessUrl(){
    if(service->serviceType == threddsServiceType::Compound);

    BESUtil::pathConcat(service->base, urlPath);
}



ThreddsCatalogReader::ThreddsCatalogReader()
{
}


std::string xmlNodeToString(xmlNode *node){
    std::stringstream ss;
    string name, value;
    map<string, string> attributes;
    BESXMLUtils::GetNodeInfo(node, name, value, attributes);
    ss << "xmlNode*: "<< node << " name: '" << name << "' value: '" << value  << "' attributes ";
    map<string, string>::const_iterator it = attributes.begin();
    while(it != attributes.end()){
        ss << it->first << "=\"" << it->second << "\" ";
        it++;
    }
    return ss.str();
}

std::string xmlNodeDetailToString(xmlNode *node){
    std::stringstream ss;
    ss << xmlNodeToString(node);
    ss << endl;
    ss << "Parent: " << node->parent << endl;
    ss << "Children: ";
    xmlNode *child_node = node->children;
    if(child_node){
        int i=0;
        while (child_node) {
            child_node = child_node->next;
            i++;
        }
        ss << i << " Found," << endl ;
    }
    else {
        ss << "None Found," << endl ;
    }

    return ss.str();
}




std::string showProperties(const map<string, string> &eProps){
    stringstream ss;
    map<string, string>::const_iterator it = eProps.begin();
    while(it != eProps.end()){
        ss << it->first << "=\"" << it->second << "\" ";
        it++;
    }
    return ss.str();
}

CatalogItem *get_catalog_ref_node(map<string, string> &eProps){
    std::map<string,string>::iterator pit;
    string id(""), href(""), name(""), title(""), type("");
    // name: catalogRef value: attrs: ID="/opendap/hyrax/nwa_catalog/" href="nwa_catalog/catalog.xml" name="nwa_catalog" title="nwa_catalog" type="simple"
    pit = eProps.find(ID);
    if(pit!= eProps.end()){
        id = pit->second;
    }
    pit = eProps.find(HREF);
    if(pit!= eProps.end()){
        href = pit->second;
    }
    pit = eProps.find(NAME);
    if(pit!= eProps.end()){
        name = pit->second;
    }
    pit = eProps.find(TITLE);
    if(pit!= eProps.end()){
        title = pit->second;
    }
    pit = eProps.find(TYPE);
    if(pit!= eProps.end()){
        type = pit->second;
    }
    BESDEBUG(MODULE, prolog << "Processing catalogRef (node). id: '" << id << "' "<< "href: '" << href << "' "<< "name: '" << name << "' "<< "title: '" << title << "' "<< "type: '" << type << "' "<< endl);

    CatalogItem *catalogRef_node = new CatalogItem();
    catalogRef_node->set_type(CatalogItem::node);
    catalogRef_node->set_name(href);

    // FIXME: Find the Last Modified date? Head??
    catalogRef_node->set_lmt(BESUtil::get_time(true));

    return catalogRef_node;
}

#if 0
/**
 * From the example expressed here:
 *     http://xmlsoft.org/tutorial/ar01s05.html
 *
 * And the associated source code from Appendix D:
 *     http://xmlsoft.org/tutorial/apd.html
 *
 */
xmlXPathObjectPtr get_node_set (xmlDocPtr doc, xmlNodePtr current_node,  xmlChar *xpath, xmlChar *namespace_list)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;


    BESDEBUG(MODULE, prolog << "BEGIN - current node is " << xmlNodeDetailToString(current_node) << endl;);


    context = xmlXPathNewContext(doc);
    if (context == NULL) {
        BESDEBUG(MODULE, prolog << "Error in xmlXPathNewContext!" << endl;);
        return NULL;
    }

    if(namespace_list){
        int result = BESXMLUtils::register_namespaces(context, namespace_list);
        if(result != 0){
            char buffer[50], *b;
            char *s =  reinterpret_cast<char *>(namespace_list);
            std::string list(s);
            throw BESInternalError("Failed to register names spaces with provided context. namespace_list: " + list,__FILE__,__LINE__);
        }
    }

    context->node = current_node;
    context->here = current_node;
    result = xmlXPathEvalExpression(xpath, context);


    xmlXPathFreeContext(context);
    if (result == NULL) {
        BESDEBUG(MODULE, prolog << "Error in xmlXPathEvalExpression!" << endl);
        return NULL;
    }
    xmlXPathObjectType type;
    switch(result->type){
    case XPATH_UNDEFINED:
        BESDEBUG(MODULE, prolog << "The value of result->type is XPATH_UNDEFINED." << endl);
       break;
    case XPATH_NODESET:
    {
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_NODESET. result->nodesetval: '"<< result->nodesetval << "'"<< endl);
        xmlNodeSetPtr node_set = result->nodesetval;
        if(node_set == NULL){
            BESDEBUG(MODULE, prolog <<  "xpath: '" << xpath << "' OUCH! The returned node_set (result->nodesetval) was NULL. " << endl);
            return NULL;
        }
        else {
            BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' node_set->nodeNr: "<< node_set->nodeNr << endl);
            BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' xmlXPathNodeSetGetLength(node_set): "<< xmlXPathNodeSetGetLength(node_set) << endl);
            BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' xmlXPathNodeSetIsEmpty(node_set): "<< (xmlXPathNodeSetIsEmpty(node_set)?"true":"false") << endl);
        }
    }
       break;
    case XPATH_BOOLEAN:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_BOOLEAN. result->boolval: '"<< result->boolval << "'"<< endl);
        break;
    case XPATH_NUMBER:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_NUMBER. result->floatval: '"<< result->floatval << "'"<< endl);
        break;
   case XPATH_STRING:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_STRING. result->stringval: '"<< result->stringval << "'"<< endl);
        break;
    case XPATH_POINT:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_POINT."<< endl);
        break;
    case XPATH_RANGE:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_RANGE."<< endl);
        break;
    case XPATH_LOCATIONSET:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_LOCATIONSET."<< endl);
        break;
    case XPATH_USERS:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_USERS."<< endl);
        break;
    case XPATH_XSLT_TREE:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type is XPATH_XSLT_TREE."<< endl);
        break;
    default:
        BESDEBUG(MODULE, prolog << "xpath: '" << xpath << "' The value of result->type (" << result->type << ") has no known association."<< endl);
        break;
    }
    // xmlXPathFreeObject(result);
    return result;
}
#endif


bool p1_serviceName_location(xmlNodePtr dataset, string &serviceName){
    BESDEBUG(MODULE, prolog << "BEGIN   dataset: '" << xmlNodeToString(dataset) << "'"<< endl);

    string value;
    map<string,string> attributes;
    xmlNodePtr serviceNameElement = BESXMLUtils::GetChild(dataset,SERVICE_NAME,value,attributes);
    if(serviceNameElement){
        BESDEBUG(MODULE, prolog << "Found ./serviceName element. value: '" << value << "'"<< endl);
        serviceName = value;
        return true;
    }
    return false;
}

bool p2_serviceName_location(xmlNodePtr datasetElement, string &serviceName){
    BESDEBUG(MODULE, prolog << "BEGIN   dataset: '" << xmlNodeToString(datasetElement) << "'"<< endl);

    string value;
    map<string,string> attributes;
    xmlNodePtr metadataElement = BESXMLUtils::GetChild(datasetElement,METADATA,value,attributes);
    if(metadataElement){
        xmlNodePtr serviceNameElement = BESXMLUtils::GetChild(metadataElement,SERVICE_NAME,value,attributes);
        if(serviceNameElement){
            BESDEBUG(MODULE, prolog << "Found ./metadata/serviceName element. value: '" << value << "'"<< endl);
            serviceName = value;
            return true;
        }
    }
    return false;
}

bool p3_serviceName_location(xmlNodePtr dataset, string &serviceName){
    BESDEBUG(MODULE, prolog << "BEGIN   dataset: '" << xmlNodeToString(dataset) << "'"<< endl);

    string name(""), value("");
    map<string,string> attributes;
    BESXMLUtils::GetNodeInfo(dataset, name, value, attributes);

    map<string,string>:: iterator it = attributes.find(SERVICE_NAME);
    if(it!=attributes.end()){
        serviceName = it->second;
        BESDEBUG(MODULE, prolog << "Found ./@serviceName attribute. value '" << serviceName << "'"<< endl);
        return true;
    }
    return false;
}

bool p4_serviceName_location(xmlNodePtr datasetElement, string &serviceName){
    BESDEBUG(MODULE, prolog << "BEGIN   dataset: '" << xmlNodeToString(datasetElement) << "'"<< endl);
    string name(""), value("");
    map<string,string> attributes;

    xmlNodePtr parentElement = datasetElement->parent;

    if(parentElement){
        xmlNodePtr metadataElement = BESXMLUtils::GetChild(parentElement,METADATA,value,attributes);
        if(metadataElement){
            xmlNodePtr serviceNameElement = BESXMLUtils::GetChild(metadataElement,SERVICE_NAME,value,attributes);
            if(serviceNameElement){
                BESDEBUG(MODULE, prolog << "Found ./metadata/serviceName element. value: '" << value << "'"<< endl);
                serviceName = value;
                return true;
            }
        }
        // keep looking up the tree!
        return p4_serviceName_location(parentElement, serviceName);
    }
    return false;
}

bool get_default_service_name(xmlNodePtr dataset_node, string &serviceName){
    bool result;

    result = p1_serviceName_location(dataset_node, serviceName);
    BESDEBUG(MODULE, prolog << "p1_service_name() returned: " <<  (result?"true":"false")  << " value: '" << serviceName << "'"<< endl);
    if(result) return true;

    serviceName.clear();
    result = p2_serviceName_location(dataset_node, serviceName);
    BESDEBUG(MODULE, prolog << "p2_service_name() returned: " <<  (result?"true":"false")  << " value: '" << serviceName << "'"<< endl);
    if(result) return true;

    serviceName.clear();
    result = p3_serviceName_location(dataset_node, serviceName);
    BESDEBUG(MODULE, prolog << "p3_service_name() returned: " <<  (result?"true":"false")  << " value: '" << serviceName << "'"<< endl);
    if(result) return true;

    serviceName.clear();
    result = p4_serviceName_location(dataset_node, serviceName);
    BESDEBUG(MODULE, prolog << "p4_service_name() returned: " <<  (result?"true":"false")  << " value: '" << serviceName << "'"<< endl);
    if(result) return true;

    return false;
}

/**
 * There are a number of ways a service element can be referenced by a dataset.
 * When multiple references come into play for a given dataset, the following is
 * the precedence for deciding on the default service to use with access methods:
 * -# A child serviceName element (XPath: "./serviceName").
 * -# A child serviceName element of a child metadata element (XPath: "./metadata/serviceName")
 * -# [DEPRECATED] A dataset element's serviceName attribute (XPath: "@serviceName") [Deprecated: use a child serviceName element instead.]
 * -# The serviceName element in an inherited metadata element of the closest ancestor dataset (XPath: the first item in the set given by "ancestor::dataset/metadata[@inherited=true]/serviceName")
 * The service with the highest precedence is the "default" service for that dataset element.
 *
 */
ThreddsService *get_default_service(
    xmlNodePtr dataset_node,
    std::map<std::string, ThreddsService *> &services
    ){

    string defaultServiceName;
    bool foundDefaultServiceName = get_default_service_name(dataset_node,defaultServiceName);
    if(foundDefaultServiceName){
        map<string,ThreddsService*>::iterator it = services.find(defaultServiceName);
        if(it!=services.end())
            return it->second;
    }
    return NULL;

}









/**
 *

 name: 'dataset' value: '' attrs: 'ID="/opendap/hyrax/test-304.html" href="test/catalog.xml" name="test-304.html" title="test" type="simple" '
 Processing dataset...
 name: 'dataSize' value: '343' attrs: 'units="bytes" '
 name: 'date' value: '2005-07-13T19:32:26' attrs: 'type="modified" units="bytes" '
 name: 'access' value: '2005-07-13T19:32:26' attrs: 'serviceName="file" type="modified" units="bytes" urlPath="/test-304.html" '
 name: 'access' value: '2005-07-13T19:32:26' attrs: 'serviceName="file" type="modified" units="bytes" urlPath="/test-304.html" '
 name: 'access' value: '2005-07-13T19:32:26' attrs: 'serviceName="file" type="modified" units="bytes" urlPath="/test-304.html" '
 *
 */
CatalogItem *get_dataset_leaf(xmlNode *datasetElement, std::map<std::string, ThreddsService *> &services){
    string eName(""), eValue("");
    map<string, string> attributes;
    BESXMLUtils::GetNodeInfo(datasetElement, eName, eValue, attributes);

    std::map<string,string>::iterator pit;
    string id(""), href(""), name(""), title(""), type(""), urlPath("");
    // name: catalogRef value: attrs: ID="/opendap/hyrax/nwa_catalog/" href="nwa_catalog/catalog.xml" name="nwa_catalog" title="nwa_catalog" type="simple"
    pit = attributes.find(ID);
    if(pit!= attributes.end()){
        id = pit->second;
    }
    pit = attributes.find(HREF);
    if(pit!= attributes.end()){
        href = pit->second;
    }
    pit = attributes.find(NAME);
    if(pit!= attributes.end()){
        name = pit->second;
    }
    pit = attributes.find(TITLE);
    if(pit!= attributes.end()){
        title = pit->second;
    }
    pit = attributes.find(TYPE);
    if(pit!= attributes.end()){
        type = pit->second;
    }
    pit = attributes.find(URL_PATH);
    if(pit!= attributes.end()){
        urlPath = pit->second;
    }

    // --------------------------------------------------------------
    // Process thredds::dataSize element if present in the dataset
    size_t dataset_size;
    map<string, string> size_attrs;
    string size_value("");
    xmlNodePtr dataSizeElement= BESXMLUtils::GetChild(datasetElement,DATA_SIZE,size_value,size_attrs);


    // If the dataSize element is present we need to process it's @units attribute
    // so that the size (in bytes) can be calculated.
    if(dataSizeElement){
        // Attribute units is required.
        // Allowed units values are {"bytes", "Kbytes", "Mbytes", "Gbytes", "Tbytes"}
        BESUtil::removeLeadingAndTrailingBlanks(size_value);
        dataset_size = atol(size_value.c_str());
        map<string, string>::iterator it;
        it = size_attrs.find(UNITS);
        if(it != size_attrs.end()){
            string units = it->second;
            size_t scale=1;
            string lunits = BESUtil::lowercase(units);

            switch(lunits[0]){

            case 'k':
                scale = 1024;
                break;

            case 'm':
                scale = 1024 * 1024;
                break;

            case 'g':
                scale = 1024 * 1024 * 1024;
                break;

            case 't':
                scale = 1024 * 1024 * 1024 * 1024;
                break;

            case 'b':
            default:
                scale = 1;
                break;

            }
            BESDEBUG(MODULE, prolog << "Found " << UNITS << ": " << units << " scale: " << scale << endl);
            dataset_size *= scale;
        }
    }



    // --------------------------------------------------------------
    // Process thredds::access elements if present in the dataset
    //
    vector<xmlNodePtr> accessElements;
    BESXMLUtils::GetChildren(datasetElement, ACCESS, accessElements);

    BESDEBUG(MODULE, prolog << "Found " << accessElements.size() << " access elements." << endl);

    ThreddsService *defaultService = get_default_service(datasetElement, services);
    BESDEBUG(MODULE, prolog << "Dataset default THREDDS Service:: " << (defaultService?defaultService->show():"null") << endl);

    vector<xmlNodePtr>::iterator ait = accessElements.begin();
    bool done = false;
    ThreddsService *dapService=0, *fileService=0;
    while(ait!=accessElements.end()){
        xmlNodePtr accessElement = *ait;
        ThreddsAccess tAccess(accessElement, defaultService);
        if(tAccess.service->serviceType == OpenDAP){
            dapService = tAccess.service;
        }
        else if(tAccess.service->serviceType == HTTPServer){
            fileService = tAccess.service;
        }
        ait++;
    }

    BESDEBUG(MODULE, prolog << "Dataset attributes. "
        << "id: '" << id << "' "
        << "href: '" << href << "' "
        << "name: '" << name << "' "
        << "title: '" << title << "' "
        << "type: '" << type << "' "
        << "urlPath: '" << urlPath << "' "
        << "dataset_size: '" << dataset_size << "' "
        << "defaultService: '" << (defaultService?defaultService->name:"null") << "' "
        << "DAP Service: '" << (dapService?dapService->name:"null") << "' "
        << "File Service: '" << (fileService?fileService->name:"null") << "' "
        << endl);

    CatalogItem *dataset_leaf = new CatalogItem();
    dataset_leaf->set_type(CatalogItem::leaf);
    dataset_leaf->set_name(href);
    dataset_leaf->set_size(dataset_size);

    // FIXME: Find the Last Modified Time! Probably a thredds:dataset/thredds:date
    dataset_leaf->set_lmt(BESUtil::get_time(true));

    return dataset_leaf;
}

/**
 * Process a dataset element
 */
void traverse_dataset(
    xmlDocPtr catalog_doc,
    xmlNodePtr datasetElement,
    std::map<std::string, ThreddsService *> &services,
    std::map<std::string, bes::CatalogItem *> &items
    ){
    string eName, eValue;
    map<string, string> attributes;
    BESDEBUG(MODULE, prolog << "BEGIN Processing " << xmlNodeToString(datasetElement) << endl);


    vector<xmlNode *> child_datasets;
    BESXMLUtils::GetChildren(datasetElement, DATASET, child_datasets);
    if(child_datasets.empty()){
        BESDEBUG(MODULE, prolog << "Processing leaf dataset: " << xmlNodeToString(datasetElement) << endl);
        // No child datasets so this is a leaf dataset.
        CatalogItem *dataset_leaf = get_dataset_leaf(datasetElement,services);
        items.insert(pair<string,CatalogItem*>(dataset_leaf->get_name(),dataset_leaf));
        return;
    }

    std::map<string,string>::iterator pit;
    xmlNode *lastElement = datasetElement;
    eValue.clear();
    attributes.clear();
    xmlNodePtr childElement = BESXMLUtils::GetFirstChild(datasetElement, eName, eValue, attributes);
    if(childElement && childElement != lastElement){
        while(childElement && childElement != lastElement){
            BESDEBUG(MODULE, prolog << "Processing Child " << xmlNodeToString(childElement) << endl);
            if(eName == DATASET){
                BESDEBUG(MODULE, prolog << "Found child dataset. Making recursive call for dataset traversal. child dataset: " << childElement << endl);
                traverse_dataset(catalog_doc, childElement, services, items);
            }
            else if(eName == CATALOG_REF){
                BESDEBUG(MODULE, prolog << "Found catalogRef" << endl);
                CatalogItem *catalogRef_node = get_catalog_ref_node(attributes);
                items.insert(pair<string,CatalogItem*>(catalogRef_node->get_name(),catalogRef_node));
            }
            else {
                BESDEBUG(MODULE, prolog << "Found " << eName << endl);

            }
            lastElement = childElement;
            eName.clear();
            eValue.clear();
            attributes.clear();
            childElement = BESXMLUtils::GetNextChild(lastElement, eName, eValue, attributes);
        }
    }

}

#if 0
 // Commented out 10/8/18 may come back...
/**
 * Process a dataset element
 */
void traverse_dataset(
    xmlDocPtr catalog_doc,
    xmlNodePtr datasetElement,
    std::map<std::string, ThreddsService *> &services,
    std::map<std::string, bes::CatalogItem *> &items
    ){
    string eName, eValue;
    map<string, string> attributes;
    std::map<string,string>::iterator pit;

    //
    eName = DATASET;
    xmlNodePtr childElement = BESXMLUtils::GetChild(datasetElement, eName, eValue, attributes);
    if(childElement != NULL){
        // We have child datasets so we do that.
        xmlNode *lastElement = datasetElement;
        attributes.clear();
        childElement = BESXMLUtils::GetFirstChild(datasetElement, eName, eValue, attributes);
        while(childElement && childElement != lastElement){
            BESDEBUG(MODULE, prolog << "Processing " << xmlNodeToString(childElement) << endl);

            if(eName == DATASET){
                BESDEBUG(MODULE, prolog << "Recursive call for dataset traversal: " << endl);
                traverse_dataset(catalog_doc, childElement, services, items);
            }
            else if(eName == CATALOG_REF){
               CatalogItem *catalogRef_node = get_catalog_ref_node(attributes);
                items.insert(pair<string,CatalogItem*>(catalogRef_node->get_name(),catalogRef_node));
            }



            lastElement = childElement;
            eName.clear();
            eValue.clear();
            attributes.clear();
            childElement = BESXMLUtils::GetNextChild(lastElement, eName, eValue, attributes);
        }
    }
    else {
        BESDEBUG(MODULE, prolog << "Processing leaf dataset: " << xmlNodeToString(childElement) << endl);
        // No child datasets so this is a leaf dataset.
        ThreddsService *service = get_default_service(catalog_doc, datasetElement, services);
        CatalogItem *dataset_leaf = get_dataset_leaf(datasetElement);
        items.insert(pair<string,CatalogItem*>(dataset_leaf->get_name(),dataset_leaf));
    }

}
#endif


#if 0
void getDatasetsAsItem(xmlNode *startElement, vector<CatalogItem *> dataset_items){

    vector<xmlNode *>::iterator xnIt;


    BESDEBUG(MODULE, prolog << "BEGIN - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"<< endl);
    vector<xmlNode *> datasets;
    BESXMLUtils::GetDescendants(startElement,DATASET,datasets);
    xnIt = datasets.begin();
    while(xnIt!=datasets.end()){
        xmlNode *node = *xnIt;
        BESDEBUG(MODULE, prolog << xmlNodeToString(node) << endl);
        vector<xmlNode *>child_datasets;
        BESXMLUtils::GetChildren(node,DATASET,child_datasets);
        if(child_datasets.empty()){
            // Simple dataset...
            string element_name;
            string value;
            map<string, string> attributes;
            std::map<string,string>::iterator ait;
            BESXMLUtils::GetNodeInfo(node, element_name, value, attributes);

            string id(""), name(""), urlPath("");

            ait = attributes.find(ID);
            if(ait!= attributes.end()){
                id = ait->second;
            }
            ait = attributes.find(NAME);
            if(ait!= attributes.end()){
                name = ait->second;
            }
            ait = attributes.find(URL_PATH);
            if(ait!= attributes.end()){
                urlPath = ait->second;
            }

            CatalogItem *item = new CatalogItem();
            item->set_type(CatalogItem::leaf);
            item->set_name(name);





        }
        else {
            BESDEBUG(MODULE, prolog << "SKIPPING compound dataset (It's in the descendants list) " << xmlNodeToString(node)  << endl);
        }
        xnIt++;
    }
    BESDEBUG(MODULE, prolog << "END - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"<< endl);

}
#endif


void ThreddsCatalogReader::getCatalogServices(xmlNode *startElement, std::map<std::string, ThreddsService *> &services)
{
    vector<xmlNode *> child_services;
    BESXMLUtils::GetChildren(startElement,SERVICE,child_services);

    vector<xmlNode *>::iterator vit = child_services.begin();
    while(vit!=child_services.end()){
        xmlNode *srvc = *vit;
        vector<xmlNode *> cmpnd_srvcs;
        ThreddsService *thredds_srvc =  new ThreddsService();

        BESXMLUtils::GetChildren(srvc,SERVICE,cmpnd_srvcs);
        string name, value;
        map<string, string> attributes;
        map<string,string>::iterator mit;

        BESXMLUtils::GetNodeInfo(srvc, name, value, attributes);
        mit = attributes.find(NAME);
        if (mit == attributes.end()){
            string err = (string) "THREDDS service declaration is missing the required " + NAME + " attribute.";
            BESDEBUG(MODULE, prolog << err << endl);
            throw BESInternalError(err, __FILE__, __LINE__);
        }
        thredds_srvc->name = mit->second;

        mit = attributes.find(SERVICE_TYPE);
        if (mit == attributes.end()){
            string err = (string) "THREDDS service declaration is missing the required " + SERVICE_TYPE + " attribute.";
            BESDEBUG(MODULE, prolog << err << endl);
            throw BESInternalError(err, __FILE__, __LINE__);
        }
        thredds_srvc->serviceTypeStr = mit->second;
        string srvc_name_normalized = BESUtil::lowercase(thredds_srvc->serviceTypeStr);
        if( srvc_name_normalized == "opendap"){
            thredds_srvc->serviceType = OpenDAP;
        }
        else if ( srvc_name_normalized == "dods"){
            thredds_srvc->serviceType = OpenDAP;
        }
        else if (srvc_name_normalized == "file"){
            thredds_srvc->serviceType = HTTPServer;
        }
        else if ( srvc_name_normalized == "httpserver"){
            thredds_srvc->serviceType = HTTPServer;
        }
        else if ( srvc_name_normalized == "compound"){
            thredds_srvc->serviceType = Compound;
        }


        if(cmpnd_srvcs.size() != 0 && thredds_srvc->serviceType == threddsServiceType::Compound){
            // This is a compound service
            getCatalogServices(srvc,thredds_srvc->child_services);
            services.insert(thredds_srvc->child_services.begin(),thredds_srvc->child_services.end());
            services.insert(pair<string, ThreddsService *>(thredds_srvc->name,thredds_srvc));
        }
        else {
            // It's just a simple service...
            mit = attributes.find(BASE);
            if (mit == attributes.end()){
                string err = (string) "THREDDS service declaration is missing the required " + BASE + " attribute.";
                BESDEBUG(MODULE, prolog << err << endl);
                throw BESInternalError(err, __FILE__, __LINE__);
            }
            thredds_srvc->base = mit->second;
            services.insert(pair<string, ThreddsService *>(thredds_srvc->name,thredds_srvc));
        }
        vit++;
    }
}


xmlDocPtr parseXmlDoc(string xml_doc_str){
    // set the default error function to my own
    vector<string> parseerrors;
    xmlSetGenericErrorFunc((void *) &parseerrors, BESXMLUtils::XMLErrorFunc);

    // XML_PARSE_NONET
    xmlDocPtr doc = xmlReadMemory(xml_doc_str.c_str(), xml_doc_str.size(), "" /* base URL */,
                        NULL /* encoding */, XML_PARSE_NONET /* xmlParserOption */);

    if (doc == NULL) {
        string err = "Problem parsing the request xml document:\n";
        bool isfirst = true;
        vector<string>::const_iterator i = parseerrors.begin();
        vector<string>::const_iterator e = parseerrors.end();
        for (; i != e; i++) {
            if (!isfirst && (*i).compare(0, 6, "Entity") == 0) {
                err += "\n";
            }
            err += (*i);
            isfirst = false;
        }
        throw BESSyntaxUserError(err, __FILE__, __LINE__);
    }

    // get the root element and make sure it exists and is called request
    xmlNodePtr root_element = xmlDocGetRootElement(doc);
    if (!root_element)
        throw BESSyntaxUserError("There is no root element in the xml document", __FILE__, __LINE__);
    return doc;
}


/**
 * @brief Converts an THREDDS catalog into a collection of bes::CatalogItems.
 *
 * If one considers each THREDDS catalog to be equivalent to
 * a bes::CatalogNode then this method examines the contents of the THREDDS catalog and
 * builds child node and leaf bes:CatalogItems based on what it finds.
 *
 * isData: The besCatalogItem objects that are leaves are evaluated against the BES_DEFAULT_CATALOG
 * TypeMatch by retrieving the BES_DEFAULT_CATALOG's BESCatalogUtils and then calling
 * BESCatalogUtils::is_data(leaf_name) This why this catalog does not utilize it's own
 * TypeMatch string.
 *
 * @param url The url of the source httpd directory.
 * @param items The map (list sorted by href) of catalog Items generated by the scrape processes. The map's
 * key is the bes::CatalogItem::name().
 */
void ThreddsCatalogReader::ingestThreddsCatalog(std::string url, std::map<std::string, bes::CatalogItem *> &items)
{
    const BESCatalogUtils *cat_utils = BESCatalogList::TheCatalogList()->find_catalog(BES_DEFAULT_CATALOG)->get_catalog_utils();

    // Go get the text from the remote resource
    RemoteHttpResource rhr(url);
    rhr.retrieveResource();
    ifstream t(rhr.getCacheFileName().c_str());
    stringstream buffer;
    buffer << t.rdbuf();
    string catalog_doc_str = buffer.str();
    BESDEBUG(MODULE, prolog << "Read catalog: " << endl << catalog_doc_str << endl);


    xmlDoc *catalog_doc = NULL;
    xmlNode *root_element = NULL;
    xmlNode *current_node = NULL;
    try {
        // XML_PARSE_NONET
        catalog_doc = parseXmlDoc(catalog_doc_str);
        // get the root element and we know it exists because parseXmlDoc() checks.
        root_element = xmlDocGetRootElement(catalog_doc);

        string eName;
        string eValue;
        map<string, string> attributes;
        BESXMLUtils::GetNodeInfo(root_element, eName, eValue, attributes);
        if (eName != CATALOG)
            throw BESSyntaxUserError(
                string("The root element should be a thredds:catalog element, found: ").append((char *) root_element->name),
                __FILE__, __LINE__);

#if 0
        // This is not really needed, because, who cares?
        if (!root_val.empty())
            throw BESSyntaxUserError(string("The request element must not contain a text value, ").append(root_val),
            __FILE__, __LINE__);
#endif
        xmlNode *lastElement = root_element;
        attributes.clear();
        xmlNode *thisElement = BESXMLUtils::GetFirstChild(root_element, eName, eValue, attributes);

        BESDEBUG(MODULE, prolog << "FIRST CHILD ELEMENT - " << xmlNodeToString(thisElement) << endl);
       if(thisElement == root_element){
            throw BESSyntaxUserError("THREDDS Catalog must actually have content.", __FILE__, __LINE__);
        }


       // std::map<std::string, ThreddsService *> services;
       std::map<std::string, ThreddsService *>::iterator sit;
       getCatalogServices(root_element,d_catalog_services);
       sit = d_catalog_services.begin();
       while(sit != d_catalog_services.end()){
           ThreddsService *tsrvc = sit->second;
           BESDEBUG(MODULE, prolog << tsrvc->show() << endl);
           sit++;
       }



       while(thisElement && (thisElement != lastElement)){
           BESDEBUG(MODULE, prolog << "Processing " << xmlNodeToString(thisElement) << endl);
           if(eName == DATASET){
               traverse_dataset(catalog_doc, thisElement, d_catalog_services, items);
           }
           else if(eName == CATALOG_REF){
               CatalogItem *catalogRef_node = get_catalog_ref_node(attributes);
               items.insert(pair<string,CatalogItem*>(catalogRef_node->get_name(),catalogRef_node));
           }
           else if(eName == SERVICE){
               BESDEBUG(MODULE, prolog << "Skipping " << SERVICE << ", already processed." << endl);
           }
           else {
               BESDEBUG(MODULE, prolog << "Skipping " << eName << " element. No processing rule." << endl);
           }
           lastElement = thisElement;
           eName.clear();
           eValue.clear();
           attributes.clear();
           thisElement = BESXMLUtils::GetNextChild(lastElement, eName, eValue, attributes);
       }
#if 0

       vector<xmlNode *>::iterator xnIt;


       BESDEBUG(MODULE, prolog << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"<< endl);
       vector<xmlNode *> datasets;
       BESXMLUtils::GetDescendants(root_element,DATASET,datasets);
       xnIt = datasets.begin();
       while(xnIt!=datasets.end()){
           BESDEBUG(MODULE, prolog << xmlNodeToString(*xnIt) << endl);
           xnIt++;
       }

       BESDEBUG(MODULE, prolog << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"<< endl);
       vector<xmlNode *> catalog_refs;
       BESXMLUtils::GetDescendants(root_element,CATALOG_REF,catalog_refs);
       xnIt = catalog_refs.begin();
       while(xnIt!=catalog_refs.end()){
           BESDEBUG(MODULE, prolog << xmlNodeToString(*xnIt) << endl);
           xnIt++;
       }
       BESDEBUG(MODULE, prolog << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"<< endl);

        while(lastElement != thisElement){
            lastElement = thisElement;
            thisElement = BESXMLUtils::GetNextChild(lastElement, eName, eValue, eProps);
            BESDEBUG(MODULE, prolog << "name: " << eName << " value:" << eValue  << " attrs: " << showProperties(eProps) << endl);
            if(eName == DATASET){
                traverse_dataset(thisElement, eName, eValue, eProps, items);
            }
            else if(eName == CATALOG_REF){
                CatalogItem *catalogRef_node = get_catalog_ref_node(eProps);
                items.insert(pair<string,CatalogItem*>(catalogRef_node->get_name(),catalogRef_node));
            }

        }
#endif
    }
    catch (...) {
        xmlFreeDoc(catalog_doc);
        xmlCleanupParser();
        throw;
    }

    xmlFreeDoc(catalog_doc);

    // Removed since the docs indicate it's not needed and it might be
    // contributing to memory issues flagged by valgrind. 2/25/09 jhrg
    //
    // Added this back in. It seems to the the cause of BES-40 - where
    // When certain tests are run, the order of <Dimension..> elements
    // in a DMR for a server function result is different when the BESDEBUG
    // output is on versus when it is not. This was true only when the
    // BESDEBUG context was 'besxml' or timing,' which lead me here.
    // Making this call removes the errant behavior. I've run tests using
    // valgrind and I see no memory problems from this call. jhrg 9/25/15
    xmlCleanupParser();

}


/*
 * @brief Returns the catalog node represented by the httpd directory page returned
 * by dereferencing the passed url.
 * @param url The url of the Apache httpd directory to process.
 * @param path The path prefix that associates the location of this generated CatalogNode with it's
 * correct position in the local service path.
 */
bes::CatalogNode *ThreddsCatalogReader::get_node(const string &url, const string &path)
{
    BESDEBUG(MODULE, prolog << "Processing url: '" << url << "'"<< endl);
    bes::CatalogNode *node = new bes::CatalogNode(path);

    if (BESUtil::endsWith(url, "catalog.xml") || BESUtil::endsWith(url, "/") ) {
        // All thredds catalogs must end in "catalog.xml" or in "/"
        map<string, bes::CatalogItem *> items;
        ingestThreddsCatalog(url, items);

        BESDEBUG(MODULE, prolog << "Found " << items.size() << " items." << endl);
        map<string, bes::CatalogItem *>::iterator it;
        it = items.begin();
        while (it != items.end()) {
            bes::CatalogItem *item = it->second;
            BESDEBUG(MODULE, prolog << "Adding item: '" << item->get_name() << "'"<< endl);
            if(item->get_type() == CatalogItem::node )
                node->add_node(item);
            else
                node->add_leaf(item);
            it++;
        }
    }
    else {
        // Otherwise we figure it's a leaf aka "item" response.
        const BESCatalogUtils *cat_utils = BESCatalogList::TheCatalogList()->find_catalog(BES_DEFAULT_CATALOG)->get_catalog_utils();
        std::vector<std::string> url_parts = BESUtil::split(url,'/',true);
        string leaf_name = url_parts.back();

        CatalogItem *item = new CatalogItem();
        item->set_type(CatalogItem::leaf);
        item->set_name(leaf_name);
        item->set_is_data(cat_utils->is_data(leaf_name));

        // FIXME: Find the Last Modified date? Head??
        item->set_lmt(BESUtil::get_time(true));

        // FIXME: Determine size of this thing? Do we "HEAD" all the leaves?
        item->set_size(1);

        node->set_leaf(item);
    }
    return node;
}

} // namespace httpd_catalog

