/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalConfigHardcoded.h
 Description: Dummy implementation of ILocalConfigService. Just uses hard-coded values
 Last Modified: $Id: LocalConfigHardcoded.h 54 2013-03-20 21:54:57Z gridge $
*/
#ifndef __LOCALCONFIGHARDCODED_
#define __LOCALCONFIGHARDCODED_

#include "ILocalConfigurationService.h"

/** Dummy implementation of ILocalConfigurationService.
 * Implement void LoadConfig() and StoreConfig(). All the configuration is hard-coded in default constructor of IConfigurationService
 */
class LocalConfigHardcoded : public ILocalConfigurationService {
 protected:
  virtual StatusCode LoadConfig(); ///< Do nothing.
  virtual StatusCode StoreConfig(); ///< Do nothing
  
 public:
  LocalConfigHardcoded(std::string pName);
  ~LocalConfigHardcoded();
};

#endif
