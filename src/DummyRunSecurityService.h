/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: DummyRunSecurityService.h
 Description: Dummy implementation if IRunningSecurityService
 Last Modified: $Id: DummyRunSecurityService.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __DUMMYRUNSECURITY_SERVICE__
#define __DUMMYRUNSECURITY_SERVICE__

#include "IRunningSecurityService.h"

/** Dummy implementation of IRunningSecurityService.
 * Always return IRunningSecurityService::SAFE
 */
class DummyRunSecurityService : public IRunningSecurityService {
 public:
  DummyRunSecurityService(std::string pName);
  ~DummyRunSecurityService();

  /// Perform dummy security check. Log result.
  virtual RunSecurityResult Run();  
};

#endif
