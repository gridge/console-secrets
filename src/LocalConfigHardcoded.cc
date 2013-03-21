/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalConfigHardcoded.cc
 Description: Dummy implementation of ILocalConfigService. Just uses hard-coded values
 Last Modified: $Id: LocalConfigHardcoded.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "LocalConfigHardcoded.h"
#include <stdlib.h>

using namespace std;

LocalConfigHardcoded::LocalConfigHardcoded(string pName) : ILocalConfigurationService(pName)
{
  
}

LocalConfigHardcoded::~LocalConfigHardcoded()
{

}

IErrorHandler::StatusCode LocalConfigHardcoded::LoadConfig()
{
  // IMPORTANT: Modify here only essential things which make no
  //  sense to have a default for. See IConfigurationService::IConfigurationService()
  //  for other defaults.

  //set default file for password storage  
  string csmDefaultPwFile;
  csmDefaultPwFile="file://";
  const char *envHome=0;
  envHome = getenv("HOME");
  if (envHome == NULL) {
    csmDefaultPwFile+=".";
  } else {
    csmDefaultPwFile+=envHome;
  }
  csmDefaultPwFile+="/";
  csmDefaultPwFile+="csmAccounts.txt.ct";
  inputURI.push_back(csmDefaultPwFile);

  //make log more verbose
  logMessagesDetails = ILog::DEBUG;

  //set default userKey
  userKey="C37DBF71";//"Simone Pagan Griso (Master Account)"; -> this also work but slightly less safe
 

  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigHardcoded::StoreConfig()
{
  return SC_NOT_IMPLEMENTED;
}
