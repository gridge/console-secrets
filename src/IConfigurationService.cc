/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IConfigurationService.cc
 Description: Configuration Service Interface
 Last Modified: $Id: IConfigurationService.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "IConfigurationService.h"
#include "IRunningSecurityService.h"

#include "TuiSvc.h"
#include "ILog.h"

extern TuiSvc *tuiSvc;
extern ILog *log;


using namespace std;

IConfigurationService::IConfigurationService(string pName) : IErrorHandler(pName)
{
  InitValues();
}

void IConfigurationService::InitValues()
{
  version="0.0.1";
  bruteForce=false;
  // -- Running Security Service minimum requirement: strongest checks!
  minRunSecurityLevel = IRunningSecurityService::SAFE;
  // -- Search Tool
  searchType = SearchRequest::TXT;
  // -- SourceURI
  defaultDataType = "file"; // local file
  //defaultDataFormat = "czx"; // crypted zipeed xml
  defaultDataFormat = "ct"; // crypted text file
  defaultName = "accounts";
  // -- Log service
  logMessagesDetails = ILog::INFO;
  // -- Accounts
  //Set predefined account types. 
  //Note that if you want some predefined values they can be added as content of the respective field in the ARecord instance.
  // we end with an '*' fields wich have to be marked as 'Essentials'
  //We now define these in the configuration file
  //predefinedAccountTypes.push_back(ARecord("Login", "User*,Password*,Host,Description,Expiration"));
  //predefinedAccountTypes.push_back(ARecord("Credit card", "Number*,Expiration*,CVV*,Name,Description"));
  //predefinedAccountTypes.push_back(ARecord("Security Question", "Question*,Answer*,Service,Description"));
  // View settings
  accountFieldNameSize = 20;
}

IConfigurationService::~IConfigurationService()
{

}

// ----------------------------------------
// Internal classes
// ----------------------------------------
IErrorHandler::StatusCode IConfigurationService::Load(std::string pCfgSource)
{
  if (not pCfgSource.empty())
    m_source = pCfgSource;
  return LoadConfig();
}

IErrorHandler::StatusCode IConfigurationService::Store(std::string pCfgDestination)
{
  if (not pCfgDestination.empty())
    m_source = pCfgDestination;
  return StoreConfig();
}
// ----------------------------------------
// ACCESSORS
// ----------------------------------------

// ----------------------------------------
// CSM settings

string IConfigurationService::GetCSMVersion()
{
  return version;
}

IErrorHandler::StatusCode IConfigurationService::SetCSMVersion(std::string pVersion)
{
  version = pVersion;
  return SC_OK;
}

bool IConfigurationService::GetBruteForce()
{
  return bruteForce;
}

IErrorHandler::StatusCode IConfigurationService::SetBruteForce(bool flag)
{
  bruteForce = flag;
  return SC_OK;
}

string IConfigurationService::GetTopMessage()
{
  return topMessage;
}

IErrorHandler::StatusCode IConfigurationService::SetTopMessage(string msg)
{
  topMessage = msg;
  return SC_OK;
}

// ----------------------------------------
// Running security Service settings

int IConfigurationService::GetMinRunSecurityLevel()
{
  return minRunSecurityLevel;
}

int IConfigurationService::SetMinRunSecurityLevel(int pSecurityLevel)
{
  //check boundaries and set value
  if (pSecurityLevel >=0 && pSecurityLevel <= IRunningSecurityService::NSECURITYLEVELS)
    minRunSecurityLevel = pSecurityLevel;
  return pSecurityLevel;
}

// ----------------------------------------
// Search Tool settings

SearchRequest::SearchType IConfigurationService::GetSearchType()
{
  return searchType;
}

IErrorHandler::StatusCode IConfigurationService::SetSearchType(SearchRequest::SearchType pSType)
{
  searchType = pSType;
  return SC_OK;
}

// ----------------------------------------
// SourceURI settings

IErrorHandler::StatusCode IConfigurationService::SetDefaultDataType(std::string pDataType)
{
  defaultDataType = pDataType;
  return SC_OK;
}

std::string IConfigurationService::GetDefaultDataType()
{
  return defaultDataType;
}

IErrorHandler::StatusCode IConfigurationService::SetDefaultDataFormat(std::string pDataFormat)
{
  defaultDataFormat = pDataFormat;
  return SC_OK;
}

std::string IConfigurationService::GetDefaultDataFormat()
{
  return defaultDataFormat;
}

IErrorHandler::StatusCode IConfigurationService::SetDefaultName(std::string pName)
{
  defaultName = pName;
  return SC_OK;
}

std::string IConfigurationService::GetDefaultName()
{
  return defaultName;
}

// ----------------------------------------
// Log settings

ILog::ILogMsgType IConfigurationService::GetLogMessagesDetails()
{
  return logMessagesDetails;
}

IErrorHandler::StatusCode IConfigurationService::SetLogMessagesDetails(ILog::ILogMsgType pLogMessagesDetails)
{
  logMessagesDetails = pLogMessagesDetails;
  return SC_OK;
}

// ----------------------------------------
// Account settings

vector<ARecord>& IConfigurationService::GetPredefinedAccountTypes()
{
  return predefinedAccountTypes;
}

IErrorHandler::StatusCode IConfigurationService::SetPredefinedAccountTypes(vector<ARecord> pAccTypes)
{
  predefinedAccountTypes = pAccTypes;
  return SC_OK;
}

IErrorHandler::StatusCode IConfigurationService::AddPredefinedAccountType(ARecord pAccType)
{  
  predefinedAccountTypes.push_back(pAccType);
  return SC_OK;
}

ARecord IConfigurationService::GetPredefinedAccountTypes(string pName)
{
  for (vector<ARecord>::iterator it = predefinedAccountTypes.begin(); it != predefinedAccountTypes.end(); ++it) {
    if (it->GetAccountName() == pName)
      return *it;
  }
  return ARecord(); //return null element
}

int IConfigurationService::GetAccountFieldNameSize()
{
  return accountFieldNameSize;
}

IErrorHandler::StatusCode IConfigurationService::SetAccountFieldNameSize(int pSize)
{
  //if TuiSvc is defined, just do a quick sanity check
  if (tuiSvc) {
    if (pSize > tuiSvc->GetScreenRows()) {
      if (log)
	*log << ILog::ERROR << "Field size exceeds screen size!" << this << ILog::endmsg;
      return m_statusCode = SC_ERROR;
    }
  }
  accountFieldNameSize = pSize;  
  return m_statusCode = SC_OK;
}

string IConfigurationService::GetUserName()
{
  return userName;
}

IErrorHandler::StatusCode IConfigurationService::SetUserName(std::string pName)
{
  userName = pName;
  if (log) *log << ILog::INFO << "userName set to: " << pName << this << ILog::endmsg;
  return SC_OK;
}

std::string IConfigurationService::GetUserKey()
{
  return userKey;
}

IErrorHandler::StatusCode IConfigurationService::SetUserKey(std::string pKey)
{
  userKey = pKey;
  if (log) *log << ILog::INFO << "userKey set to: " << pKey << this << ILog::endmsg;  
  return SC_OK;
}
