/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IRunningSecurityService.h
 Description: Interface for running security checks. Implements dummy check.
 Last Modified: $Id: IRunningSecurityService.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __IRUNNINGSECURITY_SERVICE__
#define __IRUNNINGSECURITY_SERVICE__

#include "ILog.h"

/** Running security service interface.
 * Defines interface for performing checks on the running environment of CSM.
 */
class IRunningSecurityService : public IErrorHandler {
 public:
  /// Define results of the checks performed
  enum RunSecurityResult {
    FATAL=0, ///< Highly discouraged to continue.
    WARNING, ///< Possible attacks to the CSM programs.
    SAFE, ///< As far as I can tell.. it's safe.
    NSECURITYLEVELS 
  };
 public:
 IRunningSecurityService(std::string pName) : IErrorHandler(pName) {};
  ~IRunningSecurityService() {};

  /** Methods which performs all security checks.
   * Returns IRunningSecurityService::SAFE if succesfull. Returns others for reduced security.
   */
  virtual RunSecurityResult Run() = 0;
};

#endif
