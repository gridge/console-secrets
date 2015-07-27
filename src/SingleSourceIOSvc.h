/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: SingleLocalSourceIOSvc.h
 Description: Implements IIOService for a single source.
 Last Modified: $Id: SingleSourceIOSvc.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __SINGLESOURCEIO_SERVICE__
#define __SINGLESOURCEIO_SERVICE__

#include <vector>

#include "IIOService.h"
#include "ARecord.h"
#include "IStorageTool.h"
#include "IFormatterTool.h"
#include "ISecurityTool.h"

/** Implements IIOService for a single source.
 * Load all data in a transient vector.
 * Store data as soon as any change occurs -- it's still possible to create a cache overloading Find()
 * Use external tool to read/write data, allowing multiple sources to be handled.
 */
class SingleSourceIOSvc : public IIOService {
 protected:
  /// define basic transient storage for data.
  std::vector<ARecord*> m_data;

  /// define Storage Tool to be used to physically load/store data
  IStorageTool* m_storageTool;
  /// define Formatter Tool to be used to format I/O data
  IFormatterTool* m_formatterTool;
  /// define ISecurity Tool to be used to encrypt/decrypt data, if needed
  ISecurityTool* m_securityTool;

  // --- Settings
  /// Regulate if encyption has to be used. Decided based on file type.
  bool m_encrypt; 
  /// Regulate if content needs to be gzipped before encryption. Decidec base on file type.
  bool m_zip;

  /// Search helper
  bool SMatch(std::string pPattern, std::string pField, SearchRequest::SearchType pSType=SearchRequest::TXT);
  /// Add ARecord to a list, if it does not exists there already.
  StatusCode addUniqueRecord(ARecord *newRecord, std::vector<ARecord*> &resultList);

  /*Tool loader.
   * Load appropriate tools depending on m_source.
   */
  StatusCode LoadTools();

 public:
  SingleSourceIOSvc(std::string pName);
  ~SingleSourceIOSvc();

  /// Force (re-)loading of data.
  virtual StatusCode Load();

  /// Check if source exists
  virtual StatusCode SourceExists();

  /// Flush data to physical source
  virtual StatusCode Store();

  /** Add one record to the local collection
   * @copydoc IIOService::Add(ARecord*, bool)
   */
  virtual StatusCode Add(ARecord *pARecord, bool flushBuffer=true);

  /// @copydoc IIOService::Find()
  virtual std::vector<ARecord*> Find(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);
  /// @copydoc IIOService::FindByAccountName()
  virtual std::vector<ARecord*> FindByAccountName(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);
  /// @copydoc IIOService::FindByLabel()
  virtual std::vector<ARecord*> FindByLabel(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);    
  /// @copydoc IIOService::FindByAccountId()
  virtual ARecord* FindByAccountId(unsigned long pAccountId);  
  /// @copydoc IIOService::GetAllAccounts()
  virtual std::vector<ARecord*> GetAllAccounts();

  /** @copydoc IIOService::GetLabels()
   * Loop over all elements to find out which are defined labels.
   * @todo at some point we may want to switch to a label manager
   */
  virtual std::vector<std::string> GetLabels();
  
  /// Remove a given record
  virtual StatusCode Remove(unsigned long pAccountId);

  /** Set Source.
   * Overloaded to enforce only one SetSource per instance.
   * If you want another source, create a new instance.
   */
  virtual StatusCode SetSource(SourceURI pSource); ///< see m_source  

  
};

#endif
