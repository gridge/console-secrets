/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ISearchTool.h
 Description: Implement Interface for search request and tools
 Last Modified: $Id: ISearchTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ISearchTool.h"
#include "IConfigurationService.h"

extern IConfigurationService *cfgMgr;

using namespace std;

SearchRequest::SearchRequest()
{
  searchTypePattern = cfgMgr->searchType;
  searchTypeFields = cfgMgr->searchType; 
}

SearchRequest::~SearchRequest()
{

}

ISearchTool::ISearchTool(string pName) : IErrorHandler(pName)
{

}

ISearchTool::~ISearchTool()
{

}

std::vector<ARecord*> ISearchTool::FindAccount(std::string pAccountName, std::vector<ARecord*> pData)
{
  SearchRequest thisSearch;
  thisSearch.fields = "ARecord::AccountName";
  thisSearch.pattern = pAccountName;
  return Find(thisSearch, pData);
}
