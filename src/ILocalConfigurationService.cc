/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ILocalConfigurationService.cc
 Description: Implement local-file configuration management
 Last Modified: $Id: ILocalConfigurationService.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ILocalConfigurationService.h"

/* Contains external C-style utilities. */
namespace externalCUtils {
#define MIN(a,b) ((a)>(b)?(b):(a))
#include <unistd.h>
#include <stdio.h>
}

using namespace std;

//Init ILocalConfigurationService specific settings, then call IConfigurationService constructor
ILocalConfigurationService::ILocalConfigurationService(string pName) : IConfigurationService(pName),
								       m_cfgDefaultFileName("console-secrets.conf") 
{
  //general class defaults
  m_source = string("./") + m_cfgDefaultFileName;
}

//Free ILocalConfigurationService-specific stuffs
ILocalConfigurationService::~ILocalConfigurationService() 
{

}

//Return CSM directory -- used to locate configuration file
// Right now it's a straight C- and linux-way of doing so
string ILocalConfigurationService::getCSMDirectory()
{
  using namespace externalCUtils;
  char szTmp[32];
  int len = 1024;
  char *pBuf; 
  pBuf = new char[len]; //Yes, I know.. I just hope it's enough or it will get truncated
  sprintf(szTmp, "/proc/%d/exe", getpid());
  int bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
  if(bytes >= 0)
    pBuf[bytes] = '\0';
  return string(pBuf);
}

//Set m_source. Priority: function parameter, default configuration files
IErrorHandler::StatusCode ILocalConfigurationService::Load(std::string pCfgSource)
{
  if (!pCfgSource.empty()) {
    m_source = pCfgSource;
  } else {
    //Default: ${program_dir}/console-secrets.conf
    string programDir = getCSMDirectory();
    if (programDir.empty())
      programDir = "./"; //try with local directory
    m_source = programDir + m_cfgDefaultFileName;
  }

  //It's responsibility of overloaded LoadConfig() to check if file exists and is readable
  return LoadConfig();
}

//Set m_cfgDefaultFileName and call StoreConfig()
IErrorHandler::StatusCode ILocalConfigurationService::Store(std::string pCfgDestination) 
{
  if (!pCfgDestination.empty()) {
    m_source = pCfgDestination;
  } 

  // Store configuration setting into m_cfgFileName
  // It's responsibility of overloaded StoreConfig() to check writing permissions
  return StoreConfig();
}
