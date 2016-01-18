/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: SingleLocalSourceIOSvc.h
 Description: Implements IIOService for a single source.
 Last Modified: $Id: SingleSourceIOSvc.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include <string>
// whao.. we need to include it before GnuPGSecurityTool.h or weird compiler errors occur.. I don't know why (maybe gpgme?!)
#include <algorithm> 

#include "SingleSourceIOSvc.h"

//Specific tools used
#include "LocalFileStorageTool.h"
#include "FormatterPlainTextTool.h"
#include "GnuPGSecurityTool.h"
#include "IConfigurationService.h"

using namespace std;

// extern declaration for global instance (csm.h)
extern ILog *log;
extern IConfigurationService *cfgMgr;

SingleSourceIOSvc::SingleSourceIOSvc(string pName) : IIOService(pName)
{
  m_encrypt = false;
  m_zip = false;
  SetOwner("SingleSourceMgrSvc");
  m_storageTool = 0;
  m_formatterTool = 0;
  m_securityTool = 0;
}

SingleSourceIOSvc::~SingleSourceIOSvc()
{
  // free all records
  *log << ILog::INFO << "Freeing memory for source " << m_source.GetURI() << this << ILog::endmsg;
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr)
    delete *itr;
  if (m_storageTool)
    delete m_storageTool;
  if (m_formatterTool)
    delete m_formatterTool;
  if (m_securityTool)
    delete m_securityTool;
  if (m_idManagerTool)
    delete m_idManagerTool;
}

IErrorHandler::StatusCode SingleSourceIOSvc::LoadTools()
{
  // --- Setup correct Storage Tool, if not already done
  bool createNewStorageTool = false;
  if (m_storageTool == 0) {    
    createNewStorageTool = true;
  } else {
    // Check if current source if the same handled by the storage tool, otherwise delete and create a new one
    if (m_storageTool->GetSource() != m_source) {
      delete m_storageTool;
      createNewStorageTool = true;
    }
  }
  if (createNewStorageTool) {
    //decide which type of storage tool we need
    log->say(ILog::VERBOSE, "Creating a new Storage Tool", this);        
    string type = m_source.GetField(SourceURI::TYPE);
    if (type == "file") {
      m_storageTool = new LocalFileStorageTool("StorageTool", m_source);
    } else {
      *log << ILog::ERROR << "Unknown source type: " << type << this << ILog::endmsg;
      return m_statusCode = SC_NOT_IMPLEMENTED;
    }
  }
  // --- Setup correct Formatter Tool, if not already done
  bool createNewFormatterTool = false;
  if (m_formatterTool == 0) {
    createNewFormatterTool = true;
  } else {
    if (m_formatterTool->GetFormat() != m_source.GetField(SourceURI::FORMAT)) {
      delete m_formatterTool;
      createNewFormatterTool = true;
    }
  }
  if (createNewFormatterTool) {    
    log->say(ILog::VERBOSE, "Creating a new Formatter Tool", this);
    //decide which type we need based on FORMAT field of m_source
    std::string format = m_source.GetField(SourceURI::FORMAT);
    if (format == "czx") { // Cripted-Zipped-Xml
      //m_formatterTool = new FormatterXMLTool("FormatterXMLTool", format); // XML format
      //m_encrypt = true;
      //m_zip = true;
      log->say(ILog::ERROR, "Requested czx format, but not implemented yet.", this);
      return m_statusCode = SC_NOT_IMPLEMENTED; //tool to be implemented
    } else if (format == "ct") { //Cripted-Text
      m_formatterTool = new FormatterPlainTextTool("FormatterPlainTextTool", format); //plain text
      m_encrypt = true;
      m_zip = false;
    } else if (format == "t") { //plain Text
      m_formatterTool = new FormatterPlainTextTool("FormatterPlainTextTool", format); //plain text
      m_encrypt = false;
      m_zip = false;
      log->say(ILog::WARNING, string("Using a non-encrypted data source: ") + m_source.GetURI(), this);
    } else {
      *log << ILog::ERROR << "Unknown source format: " << format << this << ILog::endmsg;
      return m_statusCode = SC_NOT_IMPLEMENTED;
    }
  }
  // --- Setup ISecurity Tool, if not already done
  bool createNewSecurityTool = false;
  if (m_securityTool == 0) {
    createNewSecurityTool = true;
  } else {
    if (m_securityTool->GetKey() != m_key) {
      delete m_securityTool;
      createNewSecurityTool = true;
    }
  }
  if (createNewSecurityTool) {
    log->say(ILog::VERBOSE, "Creating new Security Tool", this);
    m_securityTool = new GnuPGSecurityTool("GnuPGSecurityTool");
    if (!m_key.empty()) // note if we first decrypt, a key will be selected automatically
      m_securityTool->SetKey(m_key);
  }
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode SingleSourceIOSvc::Load()
{
  StatusCode sc = SC_OK;
  *log << ILog::INFO << "Loading data from source: " << m_source.GetURI() << this << ILog::endmsg;
  // --- Load tools
  sc = LoadTools();
  if (sc != SC_OK) {
    if (sc == SC_NOT_IMPLEMENTED)
      *log << ILog::ERROR << "Error loading tools: functionality not implemented for source " 
	   << m_source.GetFullURI() << this << ILog::endmsg;
    else 
      log->say(ILog::ERROR, "Generic error loading tools.", this);
    return sc;
  }
  // --- Ok, now load, decode and store data in memory
  // -- Read data
  string bufStr;
  StatusCode scLocal;
  ///@todo implement a recovery utility which uses IStorageTool::Load and IFormatterTool:Decode with pBruteForce=true
  scLocal = m_storageTool->Load(bufStr);
  if (scLocal == SC_WARNING) {
    log->say(ILog::WARNING, string("Warning in loading source ") + m_source.GetURI() + 
	     string(": ") + m_storageTool->GetErrorMsg());
  } else if (scLocal >= SC_ERROR) {
    log->say(ILog::ERROR, string("Error while loading source ") + m_source.GetURI() + 
	     string(": ") + m_storageTool->GetErrorMsg());
    return sc = scLocal;
  }

  // -- If data is encrypted, now decrypt
  if (m_encrypt) {
    string plainText;
    if (m_securityTool->Decrypt(bufStr, plainText) >= SC_ERROR) {
      // abort only if SC_ERROR, with OK or WARNING go on..
      *log << ILog::ERROR << "Error decrypting source: " 
	   << m_storageTool->GetErrorMsg() << this << ILog::endmsg;
      return m_statusCode = SC_ERROR;
    }
    // put on buffer
    ISecurityTool::ClearString(bufStr);
    bufStr = plainText;
    ISecurityTool::ClearString(plainText);
    // save the determined key for later storage
    m_key = m_securityTool->GetKey();
  }
  // -- If data is zipped, now unzip
  if (m_zip)
    return SC_NOT_IMPLEMENTED;
  // -- Decode data into transient vector
  vector<ARecord *> recordsToAdd;
  scLocal = m_formatterTool->Decode(bufStr, recordsToAdd, cfgMgr->GetBruteForce());
  if (scLocal == SC_WARNING) {
    log->say(ILog::WARNING, string("Warning in decoding source ") + m_source.GetURI() + 
	     string(": ") + m_formatterTool->GetErrorMsg());
  } else if (scLocal >= SC_ERROR) {
    log->say(ILog::ERROR, string("Error while decoding source ") + m_source.GetURI() + 
	     string(": ") + m_formatterTool->GetErrorMsg());
    return sc = scLocal;
  }  
  // -- Add new records to the list
  for (vector<ARecord *>::iterator itRec = recordsToAdd.begin(); itRec != recordsToAdd.end(); ++itRec) {
    scLocal = Add(*itRec, false); //no flush on disk, we're loading :)
    if (scLocal != SC_OK)
      return m_statusCode = scLocal;
  }
  ISecurityTool::ClearString(bufStr);
  *log << ILog::DEBUG << "Finished loading from source: " << m_source.str() << this << ILog::endmsg;
  return m_statusCode = sc;
}

IErrorHandler::StatusCode SingleSourceIOSvc::SourceExists()
{
  //only check if source exists, don't try to read or decrypt
  StatusCode sc = SC_ERROR;
  *log << ILog::INFO << "Testing source: " << m_source.GetURI() << this << ILog::endmsg;
  // --- Load tools
  sc = LoadTools();
  if (sc != SC_OK) {
    if (sc == SC_NOT_IMPLEMENTED)
      *log << ILog::ERROR << "Error loading tools: functionality not implemented for source " 
	   << m_source.GetFullURI() << this << ILog::endmsg;
    else 
      log->say(ILog::ERROR, "Generic error loading tools.", this);
    //return always that a check was not possible
    return SC_ERROR;
  }
  // --- Ok, now try to see if source exists
  sc = m_storageTool->StorageExists();
  *log << ILog::DEBUG << "Source exists: " << sc << this << ILog::endmsg;
  return sc;
}

IErrorHandler::StatusCode SingleSourceIOSvc::Store()
{
  StatusCode sc = SC_OK;
  m_statusCode = sc;
  *log << ILog::INFO << "Storing data to source: " << m_source.GetFullURI() << this << ILog::endmsg;
  // --- Load tools
  LoadTools();
  if (m_statusCode != SC_OK) {
    log->say(ILog::ERROR, "Error loading tools.", this);
    return m_statusCode;
  }
  // --- Ok, now store data
  // -- First code the information
  string bufStr;
  sc = m_formatterTool->Code(m_data, bufStr);
  if (sc == SC_WARNING) {
    log->say(ILog::WARNING, string("Warning in coding source ") + m_source.GetURI() + 
	     string(": ") + m_formatterTool->GetErrorMsg());
    m_statusCode = sc;
    m_errorMsg = "See above.";
  } else if (m_statusCode >= SC_ERROR) {
    log->say(ILog::ERROR, string("Error while coding source ") + m_source.GetURI() + 
	     string(": ") + m_formatterTool->GetErrorMsg());
    m_errorMsg = "See above.";
    return m_statusCode = sc;
  }  
  // -- If zip/cryptation is requested, apply
  if (m_zip)
    return SC_NOT_IMPLEMENTED;
  if (m_encrypt) {
    string chiperText;
    if (m_securityTool->Encrypt(bufStr, chiperText) >= SC_ERROR) {
      // abort only if SC_ERROR, with OK or WARNING go on..
      *log << ILog::ERROR << "Error encrypting source: " 
	   << m_storageTool->GetErrorMsg() << this << ILog::endmsg;
      return m_statusCode = SC_ERROR;
    }
    bufStr.clear();
    bufStr = chiperText;
  }
  sc = m_storageTool->Store(bufStr);
  if (sc == SC_WARNING) {
    log->say(ILog::WARNING, string("Warning while writing to source ") + m_source.GetURI() + 
	     string(": ") + m_storageTool->GetErrorMsg());
    m_errorMsg = "See above.";
    m_statusCode = sc;
  } else if (sc >= SC_ERROR) {
    log->say(ILog::ERROR, string("Error while writing to source ") + m_source.GetURI() + 
	     string(": ") + m_storageTool->GetErrorMsg());
    m_errorMsg = "See above.";
    m_statusCode = sc;
    return m_statusCode;
  }

  return m_statusCode;
}

IErrorHandler::StatusCode SingleSourceIOSvc::Add(ARecord *pARecord, bool flushBuffer)
{
  m_statusCode = SC_OK;

  // -- Assigning an unique accountId to each record and lock the information
  //retrieve new available global ID (needed for multiple source handling)
  int newId = -1;
  if (m_idManagerTool == 0) {
    m_idManagerTool = new IdManagerTool("IdManager");
    m_localIdMgrInstance = true; //flag so that can be freed by IIOService destructor -- ugly
  } 
  newId = m_idManagerTool->GetNewId(m_source.GetURI());
  if (newId == -1) {
    // ID Manager Tool not available.. error.
    log->say(ILog::ERROR, "Error in requesting new ID.", this);
    return m_statusCode = SC_ERROR;
  }
  pARecord->m_accountId = newId;
  pARecord->SetLock(ARecord::LOCKED);

  // -- Now append to the list of records
  m_data.insert(m_data.end(), pARecord);

  // -- Flush to media
  if (flushBuffer)
    m_statusCode = Store();
  
  return m_statusCode;
}

bool SingleSourceIOSvc::SMatch(string pPattern, string pField, SearchRequest::SearchType pSType)
{
  switch (pSType) {
  case SearchRequest::TXT:
    {
      string pField_toUpper = pField;
      string pPattern_toUpper = pPattern;
      std::transform(pField.begin(), pField.end(),pField_toUpper.begin(), ::toupper);
      std::transform(pPattern.begin(), pPattern.end(), pPattern_toUpper.begin(), ::toupper);
      if (pField_toUpper.find(pPattern_toUpper) != string::npos)
	return true;
      return false;
      break;
    }
  case SearchRequest::EXACT:
    if (pField == pPattern)
      return true;
    return false;
    break;
  case SearchRequest::REGEX:
    //not implemented yet
    break;
  }

  //not implemented
  log->say(ILog::ERROR, ("Search type not implemented while searching for") + pPattern + 
	   string(" in " ) + pField, this);
  return false;
}

vector<ARecord*> SingleSourceIOSvc::Find(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  *log << ILog::VERBOSE << "Performing search of pattern: " << pSearch << ", with type = " << pTypeOfSearch << this << ILog::endmsg;
  if ((m_data.size() == 0) && !m_source.GetURI().empty()) {
    //try to load source first
    log->say(ILog::WARNING, string("Data empty. Trying to load from source: ") + m_source.GetURI(), this);
    Load();
  }
  vector<ARecord*> sRes;
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    //search for account name
    if (SMatch(pSearch, (*itr)->GetAccountName(), pTypeOfSearch)) {
      *log << ILog::DEBUG << "Acount name matched search for record Id: " << (*itr)->GetAccountId() << this << ILog::endmsg;
      addUniqueRecord(*itr, sRes);
      continue;
    }
    //now for labels
    vector<string> labels = (*itr)->GetLabels();
    bool found=false;
    for (vector<string>::iterator itL = labels.begin(); itL != labels.end(); ++itL)
      if (SMatch(pSearch, *itL, pTypeOfSearch)) {
	*log << ILog::DEBUG << "Acount labels matched search for record Id: " << (*itr)->GetAccountId() << this << ILog::endmsg;
	addUniqueRecord(*itr, sRes);
	found=true;
	break; //exit labels loop
      }
    if (found)
      continue; //next record
    //finally for all other fields names and values (m_essentials will match with field names anyway)
    for (ARecord::TFieldsIterator itf = (*itr)->GetFieldsIterBegin(); itf != (*itr)->GetFieldsIterEnd(); ++itf) {
      if (SMatch(pSearch, itf->first, pTypeOfSearch) || SMatch(pSearch, itf->second, pTypeOfSearch)) {
	*log << ILog::DEBUG << "Acount field " << itf->first << " matched search for record Id: " << (*itr)->GetAccountId() << this << ILog::endmsg;
	addUniqueRecord(*itr, sRes);
	found = true;
	break;
      }      
    }
    if (found)
      continue; //next record -- useless now, but in case we add other checks
  } // loop over records
  return sRes;
}

IErrorHandler::StatusCode SingleSourceIOSvc::addUniqueRecord(ARecord *newRecord, std::vector<ARecord*> &resultList)
{
  if (!newRecord) {
    *log << ILog::DEBUG << "Can't add empty record." << this << ILog::endmsg;
    return SC_ERROR; //invalid record
  }
  for (vector<ARecord*>::iterator itR = resultList.begin(); itR != resultList.end(); ++itR) {
    if ((*itR)->GetAccountId() == newRecord->GetAccountId()) {
      return SC_WARNING; //record exists already
    }
  }
  resultList.push_back(newRecord);
  return SC_OK;
}
 
vector<ARecord*> SingleSourceIOSvc::FindByAccountName(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  if ((m_data.size() == 0) && !m_source.GetURI().empty()) {
    //try to load source first
    log->say(ILog::WARNING, string("Data empty. Trying to load from source: ") + m_source.GetURI());
    Load();
  }
  vector<ARecord*> sRes;
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    //search for account name
    if (SMatch(pSearch, (*itr)->GetAccountName(), pTypeOfSearch)) {
      sRes.push_back(*itr);
      continue;
    }
  }
  return sRes;
}
 
vector<ARecord*> SingleSourceIOSvc::FindByLabel(std::string pSearch, SearchRequest::SearchType pTypeOfSearch)
{
  if ((m_data.size() == 0) && !m_source.GetURI().empty()) {
    //try to load source first
    log->say(ILog::WARNING, string("Data empty. Trying to load from source: ") + m_source.GetURI());
    Load();
  }
  vector<ARecord*> sRes;
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    vector<string> labels = (*itr)->GetLabels();
    for (vector<string>::iterator itL = labels.begin(); itL != labels.end(); ++itL)
      if (SMatch(pSearch, *itL, pTypeOfSearch)) {
	sRes.push_back(*itr);
	break; //exit labels loop
      }
  }
  return sRes;
}    

ARecord* SingleSourceIOSvc::FindByAccountId(unsigned long pAccountId)
{
  if ((m_data.size() == 0) && !m_source.GetURI().empty()) {
    //try to load source first
    log->say(ILog::WARNING, string("Data empty. Trying to load from source: ") + m_source.GetURI());
    Load();
  }
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    if ((*itr)->GetAccountId() == pAccountId) {
      // record found. Assume no duplicates
      return *itr;
    }
  }
  //record not found
  *log << ILog::WARNING << "Record not found. AccountId = " << pAccountId << this << ILog::endmsg;
  return 0; // return null pointer
}

vector<ARecord*> SingleSourceIOSvc::GetAllAccounts()
{
  if ((m_data.size() == 0) && !m_source.GetURI().empty()) {
    //try to load source first
    log->say(ILog::WARNING, string("Data empty. Trying to load from source: ") + m_source.GetURI());
    Load();
  }
  return m_data;
}

vector<string> SingleSourceIOSvc::GetLabels()
{
  vector<string> labelList;
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    //add to the list
    labelList.insert(labelList.end(), (*itr)->GetLabelsIterBegin(), (*itr)->GetLabelsIterEnd());
  }
  //now make the list unique, and sorted
  sort(labelList.begin(), labelList.end());
  vector<string>::iterator newSize = unique(labelList.begin(), labelList.end());
  labelList.resize( newSize - labelList.begin() );

  return labelList;
}

IErrorHandler::StatusCode SingleSourceIOSvc::Remove(unsigned long pAccountId)
{
  m_statusCode = SC_OK;

  bool record_found=false;
  //look for the record
  for (vector<ARecord*>::iterator itr = m_data.begin(); itr != m_data.end(); ++itr) {
    if ((*itr)->GetAccountId() == pAccountId) {
      //print warning
      *log << ILog::INFO << "Removing record with accountId = " << pAccountId << this << ILog::endmsg;
      //remove this record and free its Id
      m_data.erase(itr);
      m_idManagerTool->FreeId(pAccountId);
      record_found = true;
      break;
    }
  }
  if (!record_found) {
    *log << ILog::WARNING << "Record to be removed not found. accountId = " << pAccountId << this << ILog::endmsg;
    return m_statusCode = SC_NOT_FOUND;
  }
  // Flush buffer
  m_statusCode = Store();

  return m_statusCode;
}

IErrorHandler::StatusCode SingleSourceIOSvc::SetSource(SourceURI pSource)
{
  if (!m_source.GetURI().empty()) {
    *log << ILog::ERROR << "Source already assigned to this instance: " << m_source.GetURI() << "(tried: " 
	 << pSource.GetURI() << ")" << this << ILog::endmsg;
    m_statusCode = SC_ERROR;
    return m_statusCode;
  }
  m_source = pSource;
  return m_statusCode = SC_OK;
}
