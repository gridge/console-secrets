/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: MultipleSourceIOSvc.cc
 Description: Implements support for multiple source management.
 Last Modified: $Id: MultipleSourceIOSvc.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include <algorithm>

#include "MultipleSourceIOSvc.h"
#include "ILog.h"
#include "IdManagerTool.h"

extern ILog *log;

using namespace std;


MultipleSourceIOSvc::MultipleSourceIOSvc(std::string pName) : IIOService(pName)
{
  SetOwner("SourceMgrSvc"); // we're meta-users.. don't need one :D
  //Create the IdManager tool -- shared by all the sources. Memory freed by base IIOService class destructor.
  m_idManagerTool = new IdManagerTool("IdManager");
}

MultipleSourceIOSvc::~MultipleSourceIOSvc()
{

}

void MultipleSourceIOSvc::SetIdManagerTool(IdManagerTool *mTool)
{
  *log << ILog::ERROR << "Tryed to change IdManagerTool of a MultipleSourceIOSvc. This cannot be done right now."
       << this << ILog::endmsg;
  m_statusCode = SC_WARNING;
}

vector<SourceURI> MultipleSourceIOSvc::GetManagedSources()
{
  //create list of sources from stored SingleSourceIOSvc instances
  vector<SourceURI> mgdSources;
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its)
    mgdSources.push_back((*its)->GetSource());

  return mgdSources;
}

bool MultipleSourceIOSvc::IsSourceManaged(SourceURI pSource)
{
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its)
    if ((*its)->GetSource() == pSource)
      return true;

  return false;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::SetSource(SourceURI pSource)
{
  //check managed sources for pSource
  if (!IsSourceManaged(pSource)) {
    *log << ILog::ERROR << "Trying to set a default resource not managed: " << pSource.GetURI() << this << ILog::endmsg;
    return m_statusCode;
  }
  m_source = pSource;
  return m_statusCode = SC_OK;
}

SingleSourceIOSvc* MultipleSourceIOSvc::GetSingleSource(SourceURI pSource)
{
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its)
    if ((*its)->GetSource() == pSource) {
      return (*its);
    }
  return 0; //source not found
}

IErrorHandler::StatusCode MultipleSourceIOSvc::NewSource(SourceURI pSource, std::string pOwner, std::string pKey)
{
  m_statusCode = SC_OK;

  // Create a new SingleSourceIOSvc instance
  string ioSvcName = "IO";
  ioSvcName += pSource.GetField(SourceURI::NAME);
  SingleSourceIOSvc *newIOSvc = new SingleSourceIOSvc(ioSvcName);
  
  // Assign the source, IdManager and properties
  m_statusCode = newIOSvc->SetSource(pSource);
  newIOSvc->SetIdManagerTool(m_idManagerTool); //share the parent IdManager
  if (!pOwner.empty()) {
    newIOSvc->SetOwner(pOwner);
  } else if (!m_owner.empty()) {
    //inherit from parent
    newIOSvc->SetOwner(m_owner);
  }
  //now set the key. Note: we will not enforce an inheritance of keys here.
  // This is done only in Load() if no source exists, but is left otherwise to the user.
  // Also remember that security tool could auto-determine the right key, in some circumstances
  if (!pKey.empty()) {
    newIOSvc->SetKey(pKey);
  } 

  //append to the list of existing sources
  m_sourceList.push_back(newIOSvc); 

  //set as default source
  m_source = pSource; 

  *log << ILog::VERBOSE << "Added new managed source: " << m_source.GetFullURI() << this << ILog::endmsg;

  return m_statusCode;  
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Load(SourceURI pSource)
{
  SourceURI tmpSrc = m_source; //store temporary
  m_source = pSource;
  m_statusCode = Load(); //call main method
  if (m_statusCode == SC_ERROR) {
    // put back original source, something went wrong
    m_source = tmpSrc;
  }
  return m_statusCode;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Store(SourceURI pSource)
{
  SourceURI tmpSrc = m_source; //store temporary
  m_source = pSource;
  m_statusCode = Store(); //call main method
  if (m_statusCode == SC_ERROR) {
    // put back original source, something went wrong
    m_source = tmpSrc;
  }
  return m_statusCode;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Load()
{
  m_statusCode = SC_OK;
  
  if (m_source.GetURI().empty()) {
    *log << ILog::ERROR << "Loading from empty source." << this << ILog::endmsg;
    return m_statusCode;
  }

  SingleSourceIOSvc *currentSouce = 0;
  if (!IsSourceManaged(m_source)) {
    // Create new empty source. Inherit key and owner from parent
    m_statusCode = NewSource(m_source, m_owner, m_key);
    if (m_statusCode >= SC_ERROR) {
      *log << ILog::ERROR << "Error in creating new object for hosting source " << m_source.GetURI()
	   << this << ILog::endmsg;
    }
  }

  currentSouce = GetSingleSource(m_source);
  if (!currentSouce) {
    //would mean that is managed but we cannot find it :)
    *log << ILog::FIXME << "Strange error occurred. Need to debug." << this << ILog::endmsg;
    return m_statusCode;
  }
  
  // Load Source
  m_statusCode = currentSouce->Load();
  if (m_statusCode >= SC_ERROR) {
    *log << ILog::ERROR << "Error loading source " << m_source.GetURI() << this << ILog::endmsg;
    return m_statusCode;
  }

  return m_statusCode;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::SourceExists(SourceURI pSource)
{
  //just create a temporary single source and check existence
  StatusCode sc = SC_ERROR;
  SingleSourceIOSvc *tmpSource = 0;
  if (pSource.Empty())
    return SC_ERROR;
  tmpSource = new SingleSourceIOSvc(pSource.GetField(SourceURI::NAME));
  tmpSource->SetSource(pSource);
  tmpSource->SetOwner(m_owner);
  tmpSource->SetKey(m_key);
  sc = tmpSource->SourceExists();
  delete tmpSource;
  return sc;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Store()
{
  m_statusCode = SC_OK;
  // not actually sure why we need this
  SingleSourceIOSvc *currentSource = 0;
  currentSource = GetSingleSource(m_source);
  if (!currentSource) {
    *log << ILog::ERROR << "Cannot find source to store: " << m_source.GetURI() << this << ILog::endmsg;
    return SC_ERROR;
  }

  //check if key is already associated with the SingleSourceIOSvc instance
  if (currentSource->GetKey().empty()) {
    if (!m_key.empty()) {
      currentSource->SetKey(m_key);
    }
    //otherwise let's suppose we do not need one 
  }

  m_statusCode = currentSource->Store();

  return m_statusCode;
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Add(ARecord *pARecord, bool flushBuffer)
{
  SingleSourceIOSvc *currentIOSvc = 0;
  //if it has a valid accountId, add it in the correct source
  if (pARecord->GetAccountId() > 0) {
    //ok, this record belongs to some existing source
    string cSource;
    cSource = m_idManagerTool->GetSource(pARecord->GetAccountId());
    if (cSource.empty()) {
      *log << ILog::ERROR << "Record to be added (Account Id = " << pARecord->GetAccountId()
	   << ") is not valid. It does not belong to any source according to IdManagerTool." 
	   << this << ILog::endmsg;
      return m_statusCode;
    }
    if (!IsSourceManaged(cSource)) {
      *log << ILog::ERROR << "Lost track of a source! Defined in Id Manager (Account Id = " << pARecord->GetAccountId()
	   << ", Source = " << cSource << ")" << this << ILog::endmsg;
      return m_statusCode;
    }
    currentIOSvc = GetSingleSource(cSource);
  } else {
    //otherwise add it to the default source
    currentIOSvc = GetSingleSource(m_source);
    if (!currentIOSvc) {
      //no (valid) default source? Raise an error then!
      *log << ILog::ERROR << "Default source is not valid: " << m_source.GetURI() 
	   << ". If a new source, you need to create it before storing anything." 
	   << this << ILog::endmsg;
      return m_statusCode;
    }
  }

  if (!currentIOSvc) {
    //It will never happen.. right?
    *log << ILog::FIXME << "Source is not valid. FIXME please." << m_source.GetURI() << this << ILog::endmsg;
    return m_statusCode;
  }

  m_statusCode = currentIOSvc->Add(pARecord, flushBuffer);

  return m_statusCode;
}

vector<ARecord*> MultipleSourceIOSvc::Find(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  ///@todo Decide if we want to add an extra-label to the record to identify which source it belongs to (ot store it inside the ARecord class).
  vector<ARecord*> sResult;
  vector<ARecord*> tmpResult;
  //loop over sources and add results together
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its) {
    tmpResult = (*its)->Find(pSearch, pTypeOfSearch);
    sResult.insert(sResult.end(), tmpResult.begin(), tmpResult.end());
  }

  return sResult;
}

vector<ARecord*> MultipleSourceIOSvc::FindByAccountName(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  vector<ARecord*> sResult;
  vector<ARecord*> tmpResult;
  //loop over sources and add results together
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its) {
    tmpResult = (*its)->FindByAccountName(pSearch, pTypeOfSearch);
    sResult.insert(sResult.end(), tmpResult.begin(), tmpResult.end());
  }

  return sResult;
}

vector<ARecord*> MultipleSourceIOSvc::FindByLabel(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  vector<ARecord*> sResult;
  vector<ARecord*> tmpResult;
  //loop over sources and add results together
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its) {
    tmpResult = (*its)->FindByLabel(pSearch, pTypeOfSearch);
    sResult.insert(sResult.end(), tmpResult.begin(), tmpResult.end());
  }

  return sResult;
}

ARecord* MultipleSourceIOSvc::FindByAccountId(unsigned long pAccountId)
{
  string cSource;
  cSource = m_idManagerTool->GetSource(pAccountId);
  SingleSourceIOSvc *cSrc;
  cSrc = GetSingleSource(cSource);
  return cSrc->FindByAccountId(pAccountId);
}

vector<string> MultipleSourceIOSvc::GetLabels()
{
  // loop over sources and build a global label list
  // very similar to SingleSourceIOSvc::GetLabels()
  vector<string> labelList;  
  for (vector<SingleSourceIOSvc*>::iterator its = m_sourceList.begin(); its != m_sourceList.end(); ++its) {
    vector<string> addToLabels = (*its)->GetLabels();
    labelList.insert(labelList.end(), addToLabels.begin(), addToLabels.end());
  }  
  //now make the list unique, and sorted
  sort(labelList.begin(), labelList.end());
  vector<string>::iterator newSize = unique(labelList.begin(), labelList.end());
  labelList.resize( newSize - labelList.begin() );

  return labelList;  
}

IErrorHandler::StatusCode MultipleSourceIOSvc::Remove(unsigned long pAccountId)
{
  string cSource;
  cSource = m_idManagerTool->GetSource(pAccountId);
  SingleSourceIOSvc *cSrc;
  cSrc = GetSingleSource(cSource);
  if (!cSrc) {
    *log << ILog::ERROR << "Record not found. Accout id = " << pAccountId
	 << this << ILog::endmsg;
    return SC_NOT_FOUND;
  } 
  return cSrc->Remove(pAccountId);      
}
