/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ILocalConfigurationService.h
 Description: Implements local-file configuration management
 Last Modified: $Id: ILocalConfigurationService.h 54 2013-03-20 21:54:57Z gridge $
*/
#ifndef __ILOCALCONFIGURATION_SERVICE__
#define __ILOCALCONFIGURATION_SERVICE__

#include "IConfigurationService.h"

#include <string>

/** Implement linux-like local access to configuration files.
 * Leave LoadConfig() and StoreConfig() to be overloaded
 */
class ILocalConfigurationService : public IConfigurationService {
 protected:
  // --- General ILocalConfigurationService settings
  const std::string m_cfgDefaultFileName; ///< Configuration default file name

 protected:
  // --- Specific implementation in derived classes
  virtual StatusCode LoadConfig() = 0;
  virtual StatusCode StoreConfig() = 0;

 protected:
  // --- Helper functions
  virtual std::string getCSMDirectory(); ///< Get CSM executable directory.

 public:
  ILocalConfigurationService(std::string pName);
  ~ILocalConfigurationService();

  /** Load configuration from file. 
   * If pCfgFileName is not provided, check m_cfgFileName or defaults filenames/locations.
   */
  virtual StatusCode Load(std::string pCfgSource="");
  /** Store configuration to a file. 
   * If pCfgFileName use default
   */
  virtual StatusCode Store(std::string pCfgDestination="");

};

#endif

