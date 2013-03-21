/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalConfigSimpleTxt.h
 Description: Simple implementation of ILocalConfigService using text files.
 Last Modified: $Id$
*/
#ifndef __LOCALCONFIGSIMPLETXT__
#define __LOCALCONFIGSIMPLETXT__

#include "ILocalConfigurationService.h"
#include "ISearchTool.h"

#include <fstream>
#include <string>

/** Implementation of ILocalConfigurationService using text files.
 *  Comments are lines starting with '#'
 *  Key/value pairs given as 'Key = Value' (spaces allowed but not mandatory)
 *  Multiple values using comma-separated list
 */
class LocalConfigSimpleTxt : public ILocalConfigurationService {
 protected:
  ///File
  std::fstream m_cfgFile;

  ///Set Key/Value separator
  char m_keyValSeparator;

  ///Set the char to delimit comment lines
  char m_commentChar;

  ///Load config from file.
  virtual StatusCode LoadConfig();

  ///This won't be implemented for now
  virtual StatusCode StoreConfig();

  ///Get key/value and set proper variable
  StatusCode SetLocalVar(std::string key, std::vector<std::string>& values);

  ///Helpers to get keys
  StatusCode GetKeyValue(bool& target, std::vector<std::string> input);
  StatusCode GetKeyValue(std::string& target, std::vector<std::string> input);
  StatusCode GetKeyValue(std::vector<std::string>& target, std::vector<std::string> input);
  StatusCode GetKeyValue(int &target, std::vector<std::string> input);
  StatusCode GetKeyValue(SearchRequest::SearchType& target, std::vector<std::string> input);
  StatusCode GetKeyValue(ILog::ILogMsgType& target, std::vector<std::string> input);

  //Translate environment variables
  StatusCode resolveEnvVariables(std::vector<std::string>& values);
  StatusCode resolveEnvVariables(std::string& value);
  
 public:
  LocalConfigSimpleTxt(std::string pName);
  ~LocalConfigSimpleTxt();
};

#endif
