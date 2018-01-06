/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: MultipleSourceIOSvc.h
 Description: Implements support for multiple source management.
 Last Modified: $Id: MultipleSourceIOSvc.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __MULTIPLESOURCEIO_SERVICE__
#define __MULTIPLESOURCEIO_SERVICE__

#include "SingleSourceIOSvc.h"
#include "SourceURI.h"

#include <string>
#include <vector>

/** Implements support for multiple source management.
 * Class to manage multiple SingleSourceIOSvc instances.
 * Keep trace of source currently used providing few new members respect to SingleSourceIOSvc.
 * Then overloads pure virtual members calling the corresponding source.
 * If inserting a new record can either use dedicated method to specify the source
 * or set a default destination source with SetSource method.
 * Similar for loading a new source.
 * Search and remove functions will automatically determine the right source (or use multiple).
 */
class MultipleSourceIOSvc : public IIOService {
 protected:

  /// Data member storing the managed SingleSourceIOSvc instances
  std::vector<SingleSourceIOSvc*> m_sourceList;

  /// Get pointer to given source
  SingleSourceIOSvc* GetSingleSource(SourceURI pSource);
 public:
  MultipleSourceIOSvc(std::string pName);
  virtual ~MultipleSourceIOSvc();

  ///(Re-)Load source
  virtual StatusCode Load();

  /**(Re-)Load source
   * Need to override since SetSource now accepts only already manages sources.
   * Here we can also load un-managed resources (which will most often the case).
   */
  virtual StatusCode Load(SourceURI pSource);

  /** Check if a given source exists.       
   * Uses SingleSourceIOSvc::SourceExists()
   * Do not load the source.
   */
  virtual StatusCode SourceExists(SourceURI pSource);

  /// Store source
  virtual StatusCode Store();
  
  /// Store source
  virtual StatusCode Store(SourceURI pSource);

  /** Create a new source.
   * This method create a new source.
   * It can be used if source does not exists yet (if Load returns SC_NOT_FOUND, for example)
   * @param pSource destination URI of the new source
   * @param pOwner Owner of the Source 
   * @param pKey encryption key to be used, if needed
   * @return status of operation
   */
  virtual StatusCode NewSource(SourceURI pSource, std::string pOwner="", std::string pKey="");

  /** Add or update existing record.
   * Decide which is the appropriate source. Otherwise use default m_source.
   * @copydoc IIOService::Add(ARecord*, bool)
   */
  virtual StatusCode Add(ARecord *pARecord, bool flushBuffer=true);

  /** Retrieve a given set of records.
   * Loop over all sources managed.
   * @copydoc IIOService::Find(std::string, SearchType pTypeOfSearch)
   */
  virtual std::vector<ARecord*> Find(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);

  /** Retrieve a given set of records.
   * Loop over all sources managed.
   * @copydoc IIOService::FindByAccountName(std::string, SearchType pTypeOfSearch)   
   */
  virtual std::vector<ARecord*> FindByAccountName(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);

  /** Retrieve a given set of records.
   * Loop over all sources managed.
   * @copydoc IIOService::FindByLabel(std::string, SearchType pTypeOfSearch)   
   */
  virtual std::vector<ARecord*> FindByLabel(std::string pSearch, SearchRequest::SearchType pTypeOfSearch=SearchRequest::TXT);  

  /** Retrieve a given record.
   * @copydoc IIOService::FindByAccountId(unsigned long)   
   */
  virtual ARecord* FindByAccountId(unsigned long pAccountId);

  /** @copydoc IIOService::GetAllAccounts() 
      Loop over existing sources and return a new vector with their complete list.
   */
  virtual std::vector<ARecord*> GetAllAccounts(int sort);
  

  /** @copydoc IIOService::GetLabels()
   * Loop over existing sources and build a global label list.
   */
  virtual std::vector<std::string> GetLabels();

  /** Delete a record.
   * @copydoc IIOService::Remove(unsigned long)   
   */
  virtual StatusCode Remove(unsigned long pAccountId);

  //Accessors
  /** Set default source.
   * Override normal behavior to use m_source to set a default destination source, where applicable.
   * Used only when inserting a new record or loading and not specifying a destination source.
   * Fails if pSource is not currently managed.
   * @param pSource needs to be a currently loaded source
   * @return status of operation
   */
  StatusCode SetSource(SourceURI pSource);

  /// Get list of managed sources.
  std::vector<SourceURI> GetManagedSources();

  /// Check if the source is managed.
  bool IsSourceManaged(SourceURI pSource);

  /// Overloaded to deny it; only one instance of MultipleSourceIOSvc is supposed to exist.
  virtual void SetIdManagerTool(IdManagerTool *mTool);
  
};

#endif
