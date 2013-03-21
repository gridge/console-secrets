/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IConfigurationService
 Description: Configuration Service Interface
 Commnent: Implement all persistent and transient configuration settings
           Methods Load(),LoadConfig(),Store() and StoreConfig() will need to be overloaded
 Last Modified: $Id: IConfigurationService.h 54 2013-03-20 21:54:57Z gridge $
*/
#ifndef __ICONFIGURATION_SERVICE__
#define __ICONFIGURATION_SERVICE__

#include "ILog.h"
#include "ISearchTool.h"
#include "ARecord.h"
#include <string>
#include <vector>

/** Interface for Configuration Service.
 * Defines basic methods for accessing/retrieving all
 * the settings of CSM program.
 * Need to overload Load(),LoadConfig() and Store(),StoreConfig() functions.
*/
class IConfigurationService : public IErrorHandler {
 protected:
  // ----------------------------------------
  // --- General IConfigurationService settings
  // ----------------------------------------
  std::string m_source;

 protected:
  /** Implements actual loading of configuration (virtual pure).
   * Pure virtual function to be overloaded with a specific config parser 
   */
  virtual StatusCode LoadConfig() = 0;

  /** Implements actual storing of configuration (virtual pure).
   * Pure virtual function to be overloaded with a specific config parser 
   */
  virtual StatusCode StoreConfig() = 0;

 public:
  /** Set default setting values. */
  IConfigurationService(std::string pName);
  ~IConfigurationService();

  /** Load configuration from a source. 
   * Uses LoadConfig() for the actual de-coding. To be overloaded. 
   */
  virtual StatusCode Load(std::string pCfgSource="");
  /** Store configuration to a destination. 
   * To be overloaded. Use StoreConfig() for the actual writing. To be overloaded. 
   */
  virtual StatusCode Store(std::string pCfgDestination="");

 public:
  /// Restore default settings
  void InitValues();

  // ----------------------------------------
  // --- Definition of all the settings
  // ----------------------------------------

  // -- CSM settings

  std::string version; ///< CSM version
  bool bruteForce; ///< Force operations (user must really know what (s)he's doing)
  std::string topMessage;

  // -- IIOService settings
  std::vector<std::string> inputURI; ///< Source(s) for password retrieval/storage  

  // -- Running Security Service settings

  int minRunSecurityLevel; ///< Minimum Running Security Level required. See IRunningSecurityService

  // -- Search Tool settings
  SearchRequest::SearchType searchType; ///< Default search type, see SearchRequest::SearchOptions for details

  // -- SourceURI settings
  std::string defaultDataType; ///< See SourceURI::SourceFields
  std::string defaultName; ///< See SourceURI::SourceFields
  std::string defaultDataFormat; ///< See SourceURI::SourceFields

  // -- Log service
  ILog::ILogMsgType logMessagesDetails; ///< Set details of logging. See ILog::ILogMsgType

  // -- Account settings
  std::vector<ARecord> predefinedAccountTypes; ///< Define standard fields for given account types
  // - viewer settings for account windows
  int accountFieldNameSize;

  // -- Source manager settings
  /// default user name used to handle the source
  std::string userName;
  /// default user key used to encrypt/descrypt the source
  std::string userKey;

 public:  
  // ----------------------------------------
  // --- Accessors, used for safe settings of properties
  // ----------------------------------------

  // -- CSM settings
  std::string GetCSMVersion();
  StatusCode SetCSMVersion(std::string pVersion);
  bool GetBruteForce();
  StatusCode SetBruteForce(bool flag);
  std::string GetTopMessage();
  StatusCode SetTopMessage(std::string msg);

  // -- Running Security Service settings
  int GetMinRunSecurityLevel(); ///< Set m_minRunSecurityLevel
  int SetMinRunSecurityLevel(int pSecurityLevel); ///< Get m_minRunSecurityLevel

  // -- Search Tool settings
  SearchRequest::SearchType GetSearchType();  
  StatusCode SetSearchType(SearchRequest::SearchType pSType);

  // -- SourceURI settings  
  StatusCode SetDefaultDataType(std::string pDataType);
  std::string GetDefaultDataType();
  StatusCode SetDefaultDataFormat(std::string pDataFormat);
  std::string GetDefaultDataFormat();
  StatusCode SetDefaultName(std::string pName);
  std::string GetDefaultName();

  // -- Log service
  ILog::ILogMsgType GetLogMessagesDetails();
  StatusCode SetLogMessagesDetails(ILog::ILogMsgType pLogMessagesDetails);

  // -- Account settings
  std::vector<ARecord> &GetPredefinedAccountTypes();
  StatusCode SetPredefinedAccountTypes(std::vector<ARecord> pAccTypes);
  StatusCode AddPredefinedAccountType(ARecord pAccType);
  ARecord GetPredefinedAccountTypes(std::string pName); ///< return TPredefAccType corresponding to pName. First found is returned.
  
  int GetAccountFieldNameSize(); ///< Get accountFieldNameSize
  StatusCode SetAccountFieldNameSize(int pSize); ///< Set size of field name for account display

  // -- Source manager settings
  std::string GetUserName();
  StatusCode SetUserName(std::string pName);
  std::string GetUserKey(); ///< get userKey
  StatusCode SetUserKey(std::string pKey); ///< Set userKey

};

#endif

