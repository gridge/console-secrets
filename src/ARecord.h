/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ARecord.h
 Description: Account Record, data model.
 Last Modified: $Id: ARecord.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ARECORD__
#define __ARECORD__

class IIOService;
class SingleSourceIOSvc;

#include <string>
#include <vector>
#include <time.h>

/** Define the basic data model for an account record.
 * This is the core of the informations that will be saved.
 * Tried to be generic but we assume all std::string members and no duplicates.
 */
class ARecord {
 protected:
  // --- Data-Model fields -- these are the persistent members
  std::string m_accountName; ///< Unique name of the account
  time_t m_creationTime; ///< Time of record creation
  time_t m_lastModificationTime; ///< Time of last change
  std::vector<std::pair<std::string, std::string> >m_fields; ///< Informations associated with the record, in the form of <Title, Content>
  std::vector<std::string> m_labels; ///< Labels associated to this record
  std::vector<std::string> m_essentials; ///< Store list of m_fields.first which are marked as essentials

  // --- Data-Model fields -- these are the transient members
  unsigned long m_accountId; ///< Stores unique account ID to be eventually used by IIOService for identification

 public:
  //Public type reference
  typedef std::vector<std::pair<std::string, std::string> > TFieldsType;
  typedef std::vector<std::pair<std::string, std::string> >::iterator TFieldsIterator;
  typedef std::vector<std::string> TLabelsType;
  typedef std::vector<std::string>::iterator TLabelsIterator;
  typedef std::vector<std::string> TEssentialsType;
  typedef std::vector<std::string>::iterator TEssentialsIterator;

  // --- Locking of data values
 public:  

 /// Define lock allowed states
  enum LockStatus {
    UNLOCKED, /// Data can be modified
    LOCKED, /// Data can be modified
    NLOCKSTATES /// In case we need to add more lock status in the future
  };
 protected:
  /// Control il object data are locked and cannot be modified
  LockStatus m_lock;
  /** Set specific lock status. 
   * Only private members and IIOService classes can call it.
   */
  LockStatus SetLock(LockStatus pLock);

 public:
  /// Check lock status
  LockStatus GetLockStatus();

 protected:  
  // --- Other properties of ARecord class
  /// Friend class for easy access of method for loading/storing
  friend class IIOService;
  friend class SingleSourceIOSvc;
  
 public:
  //----------------------------------------  
  // --- Constructors
  //----------------------------------------

  /// Default constructor
  ARecord();
  /** copy constructor.
   * Create a copy of the record. The new record is not locked!
   */
  ARecord(const ARecord& pARecord); ///< define a copy constructor to unlock the copied object data

  /** init record with name and fields.
   * Create a new ARecord object and fills name and list of fields from a pDelim-separated string
   * @param pAccountName fills m_accountName
   * @param pFields contains a list of pDelim-separated words which are the name of the fields. If it ends with an '*', that field is also marked as Essential. Each field value is set to empty string. Therefore pDelim must be different from '*'
   * @param pDelim sets the delimiter for each field in pFields
   */
  ARecord(std::string pAccountName, std::string pFields, std::string pDelim=",");

  /// Default destructor
  ~ARecord();

  //----------------------------------------
  // --- accessors
  //----------------------------------------

  // m_accountName
  void SetAccountName(std::string pAccountName);
  std::string GetAccountName();

  // m_fields
  void AddField(std::string pTitle, std::string &pContent); ///< Add <pTitle, pContent> to m_fields
  void AddDebugField(std::string pTitle, std::string pContent); ///< Add <pTitle, pContent> to m_fields
  void EraseFields(); ///< Erase all the content of m_fields
  void EraseField(std::string pTitle); ///< erase elements with title pTitle from m_fields
  void EraseField(TFieldsIterator pField); ///< erase elements 
  TFieldsIterator GetFieldsIterBegin(); ///< Return iterator for m_fields at the begin of the map
  TFieldsIterator GetFieldsIterEnd(); ///<  Return iterator for m_fields at the end of the map
  std::string GetField(std::string pTitle); ///< Get Content of m_fields element with key pTitle
  std::vector<std::string> GetFieldNameList(); ///< return a list of field names stored in ARecord::m_fields
  size_t GetNumberOfFields(); ///< return the number of fields
  bool HasField(std::string pTitle); ///< returns if field pTitle exists

  // m_labels
  void AddLabels(std::vector<std::string> pLabels); ///< Append to m_labels
  void AddLabel(std::string pLabel); ///< add single label to the list m_labels
  void ClearLabels(); ///< clear m_labels list
  std::vector<std::string> GetLabels(); ///< get all m_labels list
  TLabelsIterator GetLabelsIterBegin(); ///< Return iterator for m_labels at the begin of the vector
  TLabelsIterator GetLabelsIterEnd(); ///< Return iterator for m_labels at the end of the vector
  bool HasLabel(std::string pLabel); ///< check if this ARecord has pLabel set

  // m_essentials
  void AddEssentials(std::vector<std::string> pEssentials, bool pForce=false); ///< Append to m_essentials
  void AddEssential(std::string pEssential, bool pForce=false); ///< add single pEssential to the list m_essentials, pForce disable check of valid field existing
  void ClearEssentials(); ///< clear m_essentials list
  std::vector<std::string> GetEssentials(); ///< get all m_essentials list
  TEssentialsIterator GetEssentialsIterBegin(); ///< Return iterator for m_essentials at the begin of the vector
  TEssentialsIterator GetEssentialsIterEnd(); ///< Return iterator for m_essentials at the end of the vector
  bool HasEssential(std::string pEssential); ///< check if this ARecord has pEssential set

  // m_accountId
  void SetAccountId(unsigned long pAccountId); ///< set m_accountId
  unsigned long GetAccountId(); ///< Get m_accountId

  // creation/modification time
  /** Set creation (and last modification) time. 
   * Validates input as well.
   * @par timeStr if null or input not valid, use current system time.
   * @return zero on success
   */
  int SetCreationTime(std::string timeStr);
  /// Retrieve creation time as string (only days)
  std::string GetCreationTime();
  /** Set last modification time
   * Validates input as well.
   * @par timeStr if null or input not valid, use current system time.
   * @return zero on success
   */
  int SetModificationTime(std::string timeStr);
  /// Retrieve last modification time as string (only days)
  std::string GetModificationTime();

};


#endif
