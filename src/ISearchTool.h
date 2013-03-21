/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ISearchTool.h
 Description: Implement Interface for search request and tools
 Last Modified: $Id: ISearchTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ISEARCH_TOOL__
#define __ISEARCH_TOOL__

#include <string>
#include <vector>

#include "ILog.h"
#include "ARecord.h"


/** Define parameters to specify a search request for ISearchTool.
 * Base class specifying search options. Derived classes can
 * add specific options that will be used in particuar services.
 * We specify different options for matching fields and content (pattern).
 * There are few field name with special meaning:
 *   - ARecord::Label search only in ARecord::m_labels content
 *   - ARecord::AccountName search only in ARecord::m_accountName content
 *   - ARecord::AccountID search only in ARecord::m_accountId content
 */
class SearchRequest {
 public:
  /** Define search type. */
  enum SearchType {
    TXT=1, ///< default, find substrings
    EXACT=2, ///< find exact match only
    REGEX=4 ///< use regular expressions
  };
 public:
  std::string pattern; ///< pattern to search for
  std::string fields; ///< fields to search in
  SearchType searchTypePattern; ///< search options for pattern
  SearchType searchTypeFields; ///< search options for fields

  SearchRequest();
  ~SearchRequest();
};


/** Interface to search Tool.
 * Search ARecord objects matching specific criteria from a given dataset of ARecord objects.
 * Can be used by IIOService to search whitthin stored data or externally to refine searches.
 * @todo This class is not used at the moment. We'll come back later, if needed.
 */
class ISearchTool : public IErrorHandler {
public:
  ISearchTool(std::string);
  ~ISearchTool();

  /** Retrieve a given set of records from given dataset.    
   * Only fields requested are guaranteed to be filled in the output records.
   * WARNING: when refining a search you are guaranteed only using in the pattern a subset of the requested fields.
   * @param pSearch specify search options in the form of a string. Syntax defined in derived classes.
   * @param pData specify data source where performing the search
   * @return Pointer to (locked) records that match the search criteria.
   */
  virtual std::vector<ARecord*> Find(std::string pSearch, std::vector<ARecord*> pData) = 0;
  /** Retrieve a given set of records.
   * Implements a more explicit syntax and wrap a call to Find(std::string, std::vector<ARecord*>) 
   * @param pOptions options for searches. See SearchType
   * @param pData specify data source where performing the search
   * @return Pointer to (locked) records that match the search criteria.
   */
  virtual std::vector<ARecord*> Find(SearchRequest pOptions, std::vector<ARecord*> pData) = 0;
  /** Retrieve a given set of records.
   * Implements a simplified syntax and wrap a call to Find(std::string, std::vector<ARecord*>) 
   * @param pAccountName account name to be searched for
   * @param pData specify data source where performing the search
   * @return Pointer to (locked) records that match the search criteria.
   */
  virtual std::vector<ARecord*> FindAccount(std::string pAccountName, std::vector<ARecord*> pData);

};


#endif
