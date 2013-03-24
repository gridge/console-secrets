/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalConfigSimpleTxt.cc
 Description: Simple implementation of ILocalConfigService using text files.
 Last Modified: $Id$
*/

#include "LocalConfigSimpleTxt.h"

#include "ILog.h"
#include "MiscUtils.h"

#include <algorithm>
#include <stdlib.h>

extern ILog *log;

using namespace std;

LocalConfigSimpleTxt::LocalConfigSimpleTxt(string pName) : ILocalConfigurationService(pName)
{
  m_keyValSeparator = '=';  
  m_commentChar = '#';
}

LocalConfigSimpleTxt::~LocalConfigSimpleTxt()
{

}

IErrorHandler::StatusCode LocalConfigSimpleTxt::LoadConfig()
{
  *log << ILog::INFO << "Loading configuration from: " << m_source << this << ILog::endmsg;
  if (not m_cfgFile.is_open()) {
    m_cfgFile.open(m_source.c_str(), fstream::in);
    if (!m_cfgFile.is_open()) {
      //Error opening input file
      log->say(ILog::ERROR, string("Error loading data from file ") + m_source, this);
      return SC_ERROR;
    } 
  }
  
  //read data
  string bufStr;
  string key;
  vector<string> values;
  try {    
    while (getline(m_cfgFile, bufStr)) {
      if (bufStr.empty()) continue;
      bufStr = CSMUtils::TrimStr(bufStr);
      if (bufStr[0] == m_commentChar) continue; //comment
      //now parse next key/value pair
      key.clear();
      values.clear();
      size_t idxKeyValSeparator;
      idxKeyValSeparator = bufStr.find(m_keyValSeparator);
      if (idxKeyValSeparator == string::npos) {
	*log << ILog::WARNING << "Malformed line in config file: " << bufStr << this << ILog::endmsg;
	continue; //skip line
      }
      key = bufStr.substr(0, idxKeyValSeparator);
      key = CSMUtils::TrimStr(key);
      string valuesStr = bufStr.substr(idxKeyValSeparator+1);
      values = CSMUtils::SplitListStr(valuesStr, string(","));
      if (SetLocalVar(key, values) != SC_OK) {
	*log << ILog::WARNING << "Uknown parameter or malformed line in config file: " << bufStr << this << ILog::endmsg;
	continue;
      }
    }
  } catch (char *str) {
    //Error I/O
    log->say(ILog::ERROR, string("I/O Error while reading input file ") + m_source, this);
    return m_statusCode = SC_ERROR;
  }
  
  *log << ILog::VERBOSE << "Finished reading config file" << this << ILog::endmsg;
  m_cfgFile.close();
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::StoreConfig()
{
  return SC_NOT_IMPLEMENTED;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::SetLocalVar(string key, vector<string>& values)
{
  //now the ugly gigantic "if" statements to set local variables
  //@todo -> switch to a map key/values?
  m_statusCode=SC_OK;
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  if (key == "bruteforce") {
    m_statusCode = GetKeyValue(bruteForce, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << bruteForce << ILog::endmsg;
  } else if (key == "inputuri") {
    resolveEnvVariables(values);
    m_statusCode = GetKeyValue(inputURI, values);
    *log << ILog::VERBOSE << "Set " << key << " to: ";
    for (vector<string>::iterator itV = inputURI.begin(); itV != inputURI.end(); ++itV) {
      *log << *itV;
      if (itV+1 != inputURI.end()) *log << ", ";
    }
    *log << this << ILog::endmsg;
  } else if (key == "minrunsecuritylevel") {
    m_statusCode = GetKeyValue(minRunSecurityLevel, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << minRunSecurityLevel << this << ILog::endmsg;
  } else if (key == "searchtype") {
    m_statusCode = GetKeyValue(searchType, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << searchType << this << ILog::endmsg;
  } else if (key == "defaultdatatype") {
    m_statusCode = GetKeyValue(defaultDataType, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << defaultDataType << this << ILog::endmsg;
  } else if (key == "defaultname") {
    m_statusCode = GetKeyValue(defaultName, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << defaultName << this << ILog::endmsg;
  } else if (key == "defaultdataformat") {
    m_statusCode = GetKeyValue(defaultDataFormat, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << defaultDataFormat << this << ILog::endmsg;
  } else if (key == "logmessagedetails") {
    m_statusCode = GetKeyValue(logMessagesDetails, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << values[0] << this << ILog::endmsg; //avoid wrong parsing by ILog::<< operator
  } else if (key == "accountfieldnamesize") {
    m_statusCode = GetKeyValue(accountFieldNameSize, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << accountFieldNameSize << this << ILog::endmsg;
  } else if (key == "username") {
    resolveEnvVariables(values);
    m_statusCode = GetKeyValue(userName, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << userName << this << ILog::endmsg;
  } else if (key == "userkey") {    
    m_statusCode = GetKeyValue(userKey, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << userKey << this << ILog::endmsg;
  } else if (key == "predefinedaccounttypes") {
    //special syntax here.. the first one is the predefined account name, then the fields
    if (values.size() < 2) {
      *log << ILog::WARNING << "Malformed predefined account";
      for (vector<string>::iterator itV = values.begin(); itV != values.end(); ++itV) {
	*log << *itV;
	if (itV+1 != values.end()) *log << ", ";
      }
      *log << this << ILog::endmsg;
    } else {
      //get predefined account name
      string accName = values[0];
      //now build the config string for predefined account types, starting from values.begin()+1
      string accFormStr;
      for (vector<string>::iterator itV = values.begin()+1; itV != values.end(); ++itV) {
	accFormStr+=*itV;
	if (itV+1 != values.end()) accFormStr += ", ";
      }
      //finally add the the list
      predefinedAccountTypes.push_back(ARecord(accName, accFormStr));
      *log << ILog::VERBOSE << "New predefined account type: " << accName << " -> " << accFormStr << this << ILog::endmsg;
    }
  } else if (key == "topmessage") {
    m_statusCode = GetKeyValue(topMessage, values);
    *log << ILog::VERBOSE << "Set " << key << " to: " << topMessage << this << ILog::endmsg;
  }

  if (m_statusCode != SC_OK) {
    *log << m_statusCode << "Problems with key: " << key << this << ILog::endmsg;
  }
  return m_statusCode;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(bool& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  string boolStr = input[0];
  boolStr = CSMUtils::TrimStr(boolStr);
  std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);
  if (boolStr == "false") target=false;
  else if (boolStr == "true") target=true;
  else {
    *log << ILog::WARNING << "Invalid value: " << input[0] << this << ILog::endmsg;
    return SC_WARNING;
  }
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(vector<string>& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  target = input;
  for (vector<string>::iterator itS = target.begin(); itS != target.end(); ++itS) {
    //trim the string
    *itS = CSMUtils::TrimStr(*itS);
  }
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(int& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  stringstream intStream;
  int intVal;
  intStream.str(CSMUtils::TrimStr(input[0]));
  try {
    intStream >> intVal;
  } catch (char *str){
    *log << ILog::WARNING << "Invalid integer value: " << input[0] << this << ILog::endmsg;
    return SC_WARNING;
  }
  target = intVal;
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(SearchRequest::SearchType& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  string searchTypeStr = input[0];
  searchTypeStr = CSMUtils::TrimStr(searchTypeStr);
  std::transform(searchTypeStr.begin(), searchTypeStr.end(), searchTypeStr.begin(), ::toupper);
  if (searchTypeStr == "TXT") target=SearchRequest::TXT;
  else if (searchTypeStr == "EXACT") target = SearchRequest::EXACT;
  else if (searchTypeStr == "REGEX") target = SearchRequest::REGEX;
  else {
    *log << ILog::WARNING << "Invalid SearchType value: " << input[0] << this << ILog::endmsg;
    return SC_WARNING;
  }
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(string& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  string targetStr = input[0];
  targetStr = CSMUtils::TrimStr(targetStr);
  if (targetStr.size() == 0) return SC_OK;
  target = targetStr;
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::GetKeyValue(ILog::ILogMsgType& target, vector<string> input)
{
  if (input.size() == 0) return SC_OK; //leave value unchanged
  ILog::ILogMsgType m;
  if (log->GetMsgTypeFromStr(m, CSMUtils::TrimStr(input[0])) != SC_OK) {
    *log << ILog::WARNING << "Invalid message type: " << input[0] << this << ILog::endmsg;
    return SC_WARNING;
  }
  logMessagesDetails=m;
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::resolveEnvVariables(std::vector<std::string>& values)
{
  for (vector<string>::iterator itV = values.begin(); itV != values.end(); ++itV) {
    resolveEnvVariables(*itV);
  }
  return SC_OK;
}

IErrorHandler::StatusCode LocalConfigSimpleTxt::resolveEnvVariables(std::string& value)
{
  size_t idxStartEnvV = 0;
  size_t idxEndEnvV   = 0;
  while (true) {
    idxStartEnvV = value.find("${", idxEndEnvV);
    if (idxStartEnvV == string::npos)
      break;
    idxEndEnvV = value.find("}", idxStartEnvV);
    if (idxEndEnvV == string::npos) {
      *log << ILog::WARNING << "Un-terminated environment variable. Assume it's not an environment variable at all: " << value << this << ILog::endmsg;
      break;
    }
    size_t envVLength=idxEndEnvV-idxStartEnvV+1;
    string envV = value.substr(idxStartEnvV+2, envVLength-3); //take out "${" and "}"
    string envVValue(getenv(envV.c_str()));
    if (envVValue.empty())
      *log << ILog::WARNING << "Replacing with empty environment variable: " << envV << this <<ILog::endmsg;
    value.replace(idxStartEnvV, envVLength, envVValue);
    idxEndEnvV = idxStartEnvV+envVValue.size(); //set new starting point for next search
  }
  return SC_OK;
}
