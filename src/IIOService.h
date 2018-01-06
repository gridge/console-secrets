/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IIOService.h
 Description: Implement Interface for IOService
 Last Modified: $Id: IIOService.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __IIO_SERVICE__
#define __IIO_SERVICE__

#include <vector>

#include "ILog.h"
#include "ARecord.h"
#include "SourceURI.h"
#include "IdManagerTool.h"
#include "ISearchTool.h"

/** Manager layer for loading/storing passwords.
 * This is the logical core of CSM. Use external tools to perform specific operations. 
 * Here we define the main methods, while the logic is implemented in derived classes.
 * In particular no data structure is defined in this interface for ARecord,  
 * to allow, for example, implementing an IIOService for managing several sources (IIOService).
 * Tips: In implementation of IIOService:
 *   - to include handle m_source use an IStorageTool.
 *   - to format data after/before I/O use a IFormatterTool
 *   - to encrypt/decrypt data use a ISecurityTool
 */
class IIOService : public IErrorHandler {
 protected:
  // We define here the general properties of this service
  std::string m_owner; ///< Owner of the service [optional]
  std::string m_key; ///< Encryption key to be used by the ISecurityTool instance(s)
  SourceURI m_source; ///< Fully identifier of the data source

  /// General-tool for source <--> record Id management
  IdManagerTool *m_idManagerTool;
  /// Flag to understand the current instance should be freed in the end
  bool m_localIdMgrInstance;

 public:
  enum AccountsSorting{
    ACCOUNTS_SORT_NOSORT = 0,
    ACCOUNTS_SORT_BYNAME = 1,
    ACCOUNTS_SORT_BYDATE = 2
  };

 public:
  IIOService(std::string pName);
  IIOService(std::string pName, std::string owner);
  virtual ~IIOService();

  // --- Implement methods that need to be defined in an IIOService implementation
  /** Load data from source.
   * Used to force data (re-)loading. Find and add should check data is available, otherwise call Load().
   */
  virtual StatusCode Load() = 0;

  /** Load data from source.
   * Used to force data (re-)loading. It also sets the current source before loading.
   */
  virtual StatusCode Load(SourceURI pSource);

  /** Test if source exists already.
   * Used to determine if a new source can be made with no overwriting.
   * e.g. a user-owned file but trying to access with wrong key.
   * @return SC_OK if source exists
   * @return SC_ERROR if an error occurred while checking
   * @return SC_NOT_FOUND if the source does not exists
   * @return SC_NOT_IMPLEMENTED if can't tell
   */
  virtual StatusCode SourceExists();

  /** Write data to the source.
   * This method is called to flush data to the physical source.
   */
  virtual StatusCode Store() = 0;

  /** Write data to the source.
   * This method is called to flush data to the physical source. it sets also the current source before storing.
   */
  virtual StatusCode Store(SourceURI pSource);

  /** Store/Modify a new/existing record.
   * If another record with the same ARecord::m_accountId exists, modify it,
   * otherwise store a new one.
   * @param pARecord record to be added
   * @param if true, flush to media, otherwise just add in memory
   * @return status of operation
   */
  virtual StatusCode Add(ARecord *pARecord, bool flushBuffer=true) = 0;

  /** Store od mofify a set of records 
   * @param pARecord record to be added
   * @param if true, flush to media, otherwise just add in memory
   * @return status of operation
   */ 
  virtual StatusCode Add(std::vector<ARecord*> pARecords, bool flushBuffer=true); 

  /** Retrieve a given set of records.
   * Perform a search of records matching the criteria in any field of stored data (including Labels).
   * @param pSearch defines the search pattern
   * @param pTypeOfSearch defines the type of search as described in SearchType
   */
  virtual std::vector<ARecord*> Find(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT) = 0;

  /** Retrieve a given set of records.
   * Perform a search by account name only
   * @param pSearch defines the search pattern
   * @param pTypeOfSearch defines the type of search as described in SearchType  
   */
  virtual std::vector<ARecord*> FindByAccountName(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT) = 0;

  /** Retrieve a given set of records.
   * Perform a search by Labels only
   * @param pSearch defines the search pattern
   * @param pTypeOfSearch defines the type of search as described in SearchType  
   */
  virtual std::vector<ARecord*> FindByLabel(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT) = 0;  

  /** Retrieve a given record.
   * Perform a search by account Id. Exact matched is returned. No duplicates are assumed.
   * @param pAccountId defines the account Id
   * @return Pointer to the (locked) record found.
   */
  virtual ARecord* FindByAccountId(unsigned long pAccountId) = 0;

  /** Retrieve all accounts. 
   * @param sort Sort accounts
   */
  virtual std::vector<ARecord*> GetAllAccounts(int sort=ACCOUNTS_SORT_NOSORT) = 0;

  /** Retrieve all labels list.
   */
  virtual std::vector<std::string> GetLabels() = 0;

  /** Delete a record.
   * Remove the given record from the data list.
   * Use m_accountId to uniquely identify the record and call Remove(unsigned long pAccountId) method
   * @param pRecord record to be removed.
   * @return status of the operation
   */
  virtual StatusCode Remove(ARecord *pRecord);

  /** Delete a record.
   * Remove the given record.
   * @param pAccountId identify the record to be removed
   * @return status of the operation.
   */
  virtual StatusCode Remove(unsigned long pAccountId) = 0;

  ///@todo: Probably we want to move the followings outside IIOService
  /** Backup utility.
   * Uses m_backupTool to make a backup copy of the informations.
   * @param pDestination is backup destination
   * @return StatusCode the state of the operation
   */
  //virtual StatusCode Backup(std::string pDestination) = 0;

  /** Import utility.
   * Uses m_importTool to import data.
   */
  //virtual StatusCode Import(std::string pSource) = 0;
  
  /** Syncronization utility.
   * Syncronize data with pExternalSource
   */
  //virtual StatusCode Syncronize(std::string pExternalSource) = 0;

  // --- Accessors
  virtual void SetOwner(std::string pOwner); ///< see m_owner
  virtual std::string GetOwner(); ///< see m_owner
  virtual void SetKey(std::string pKey); ///< see m_key
  virtual std::string GetKey(); ///< see m_key
  virtual StatusCode SetSource(SourceURI pSource); ///< see m_source
  virtual SourceURI GetSource(); ///< see m_source

  // --- Tools accessors
  virtual void SetIdManagerTool(IdManagerTool *mTool);

};

#endif
