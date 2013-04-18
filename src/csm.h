/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: csm.h
 Description: Main CSM program header
 Last Modified: $Id: csm.h -1   $
*/

#ifndef __CSM__
#define __CSM__

#include <string>

//Include services and tools
#include "IConfigurationService.h"
#include "LocalConfigSimpleTxt.h"

#include "ILog.h"
#include "LogLocalFile.h"

#include "IRunningSecurityService.h"
#include "DummyRunSecurityService.h"

#include "IIOService.h"
#include "MultipleSourceIOSvc.h"

#include "TuiSvc.h"

#include "GnuPGSecurityTool.h"

//Store pointers to instances the services and tools needed by CSM
IConfigurationService *cfgMgr; ///< Configuration Manager Service
ILog *log; ///< Logging Service
IRunningSecurityService *runningSecurityChecks; ///< Running Security Service
MultipleSourceIOSvc *ioSvc; ///< IO service
TuiSvc *tuiSvc; ///< TUI service

//CSM return codes
const int CSM_OK=0;
const int CSM_SECURITY=-1;
const int CSM_ACTION_ERROR=2;
const int CSM_WRONG_CONFIG=3;

//CSM command line actions
enum CmdLineActions {
  act_startGui=0,
  act_quickSearch,
  act_createSource,
  act_nActions
};

//CSM useful global info
std::string logFileName; ///< log file location

#endif
