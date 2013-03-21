/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: DummyRunSecurityService.cc
 Description: Dummy implementation if IRunningSecurityService
 Last Modified: $Id: DummyRunSecurityService.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "DummyRunSecurityService.h"
#include "ILog.h"

//external definitions
extern ILog *log;

using namespace std;

DummyRunSecurityService::DummyRunSecurityService(string pName) : IRunningSecurityService(pName)
{

}

DummyRunSecurityService::~DummyRunSecurityService()
{

}

DummyRunSecurityService::RunSecurityResult DummyRunSecurityService::Run()
{  
  log->say(ILog::INFO, "Running security checks: Passed.", this);
  return SAFE;
}
