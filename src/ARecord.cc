/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ARecord.cc
 Description: Account Record, data model.
 Last Modified: $Id: ARecord.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ARecord.h"
#include "ILog.h"
#include "MiscUtils.h"

#include <sstream>
#include <algorithm>
//#include <locale>
#include <stdio.h>
#include <iomanip>

//external definitions
extern ILog *log;

using namespace std;

ARecord::ARecord()
{
  m_accountId = 0;
  //set creation time to current system time
  SetCreationTime();
  SetLock(UNLOCKED);
}

ARecord::~ARecord()
{
}

ARecord::ARecord(const ARecord& pARecord)
{
  m_accountName = pARecord.m_accountName;
  m_fields = pARecord.m_fields;
  m_labels = pARecord.m_labels;
  m_essentials = pARecord.m_essentials;
  m_accountId = pARecord.m_accountId;
  m_creationTime = pARecord.m_creationTime;
  m_lastModificationTime = pARecord.m_lastModificationTime;
  SetLock(UNLOCKED); // New record UNLOCKED by default
}

ARecord::ARecord(string pAccountName, string pFields, string pDelim) : 
  m_accountName(pAccountName)
{
  if (pDelim == "*") {
    //whooo.. did you really have to choose this one?
    if (log)
      log->say(ILog::FIXME, "Delimiter for fields initialization is '*'. Wrong choice!", "ARecord");
  }
  //parse pFields. it's a pDelim-separated list of fields
  string str = pFields;
  size_t cutAt;
  while( (cutAt = str.find_first_of(pDelim)) != str.npos ) {
    if(cutAt > 0) {
      string toadd = str.substr(0,cutAt);
      if (toadd[toadd.size()-1] == '*') {
	//field to be marked as essential
	toadd = toadd.substr(0,toadd.size()-1);
	m_essentials.push_back(toadd);
      }
      m_fields.push_back(make_pair<string, string>(toadd, string("")));
    }
    str = str.substr(cutAt+pDelim.size());
  }
  if(str.length() > 0) {
    m_fields.push_back(make_pair<string, string>(str, string("")));
  }  
}

void ARecord::SetAccountName(string pAccountName)
{
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
    return;
  }
  //does not allow empty names
  if (!pAccountName.empty())
    m_accountName = pAccountName;
  else {
    string msgWarning;
    msgWarning = "Tried to set null name to ARecord ";
    msgWarning += m_accountName;
    msgWarning += ".";
    if (log)
      log->say(ILog::WARNING, msgWarning, "ARecord");
  }
}

string ARecord::GetAccountName()
{
  return m_accountName;
}

void ARecord::AddDebugField(std::string pTitle, std::string pContent)
{
  string newContent = pContent;
  AddField(pTitle, newContent);
}

void ARecord::AddField(string pTitle, string &pContent)
{
  string title= CSMUtils::TrimStr( pTitle );
  string content = CSMUtils::TrimStr( pContent );
  if (log)
    log->say(ILog::DEBUG, string("Adding new field:")+title
	     + string(" to record ") + m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to add field of a locked ARecord ") + m_accountName, "ARecord");
    return;
  }
  if (!title.empty())
    m_fields.push_back( make_pair<string, string>(title, content));
  else
    if (log)
      log->say(ILog::WARNING, string("Trying to add empty element to ARecord") + m_accountName, "ARecord");
}

void ARecord::EraseFields()
{
  if (log)
    log->say(ILog::DEBUG, string("Clearing fields of record:")+ m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
  } else {
    m_fields.clear();
  }
}

void ARecord::EraseField(string pTitle)
{
  if (log)
    log->say(ILog::DEBUG, string("Clearing field '")+pTitle+string("' of record:")+ m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
    return;
  }
  TFieldsIterator recordToDelete;
  for (recordToDelete = m_fields.begin(); recordToDelete != m_fields.end(); ++recordToDelete) {
    if (recordToDelete->first == pTitle) {
      if (log)
	log->say(ILog::VERBOSE, string("Removed field ") + pTitle + string(" from record ") + m_accountName, "ARecord");
      m_fields.erase(recordToDelete);
    }
  }
}

void ARecord::EraseField(TFieldsIterator pField)
{
  m_fields.erase(pField);
  if (log)
    log->say(ILog::DEBUG, string("Erased field: ") + pField->first, "ARecord");
}


ARecord::TFieldsIterator ARecord::GetFieldsIterBegin()
{
  return m_fields.begin();
}

ARecord::TFieldsIterator ARecord::GetFieldsIterEnd()
{
  return m_fields.end();
}


string ARecord::GetField(string pTitle)
{
  if (log)
    log->say(ILog::DEBUG, string("Requesting field'")+pTitle+string("' of record:")+ m_accountName,"ARecord");
  TFieldsIterator recordToFind;
  for (recordToFind = m_fields.begin(); recordToFind != m_fields.end(); ++recordToFind)
    if (recordToFind->first == pTitle)
      return recordToFind->second;
  if (log)
    log->say(ILog::WARNING, string("Field ") + pTitle + string(" not found in ARecord ") + m_accountName, "ARecord");
  return string(""); // Field not found
}

vector<string> ARecord::GetFieldNameList()
{
  vector<string> rList;
  for (TFieldsIterator its = m_fields.begin(); its != m_fields.end(); ++its) 
    rList.push_back(its->first);
  return rList;
}

size_t ARecord::GetNumberOfFields()
{
  return m_fields.size();
}

bool ARecord::HasField(std::string pTitle)
{
  for (TFieldsIterator its = m_fields.begin(); its != m_fields.end(); ++its) 
    if (its->first == pTitle)
      return true;
  return false;
}

void ARecord::AddLabels(vector<string> pLabels)
{
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");    
  } else {
    for (vector<string>::iterator itl = pLabels.begin(); itl != pLabels.end(); ++itl)
      AddLabel(*itl);
  }
}

void ARecord::AddLabel(string pLabel)
{
  string lab = CSMUtils::TrimStr(pLabel); //remove spaces
  if (log)
    log->say(ILog::DEBUG, string("Adding label '")+lab+string("' in record:")+ m_accountName,"ARecord");
  if (lab.empty())
    return; //nothing to add
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
    return;
  }
  if (!lab.empty()) 
    m_labels.push_back(lab);
  else 
    if (log)
      log->say(ILog::WARNING, "Tried to add an empty label to ARecord object.", "ARecord");
}

void ARecord::ClearLabels()
{
  if (log)
    log->say(ILog::DEBUG, string("Clearing labels in record:")+ m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
  } else {
    //delete list of labels
    m_labels.clear();
  }
}

vector<string> ARecord::GetLabels()
{
  return m_labels;
}

ARecord::TLabelsIterator ARecord::GetLabelsIterBegin()
{
  return m_labels.begin();
}

ARecord::TLabelsIterator ARecord::GetLabelsIterEnd()
{
  return m_labels.end();
}

bool ARecord::HasLabel(string pLabel)
{
  if (find(m_labels.begin(), m_labels.end(), pLabel) != m_labels.end())
    return true;
  else
    return false;
}

void ARecord::AddEssentials(vector<string> pEssentials, bool pForce)
{
  for (vector<string>::iterator ite = pEssentials.begin(); ite != pEssentials.end(); ++ite)
    AddEssential(*ite, pForce);
}

void ARecord::AddEssential(string pEssential, bool pForce)
{
  string ess = CSMUtils::TrimStr(pEssential);
  if (log)
    log->say(ILog::DEBUG, string("Adding essential '")+ess+string("' in record:")+ m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
    return;
  } 
  if (!ess.empty()) {
    //check if essential is a valid field or pForce is true
    if (pForce) {
	m_essentials.push_back(ess);    
	return;
    }
    for (TFieldsIterator itf = m_fields.begin(); itf != m_fields.end(); ++itf)
      if (itf->first == ess) {
	m_essentials.push_back(ess);    
	return;
      }
    if (log)
      log->say(ILog::ERROR, string("Essential field not valid: ") + ess, "ARecord");
  } else {
    if (log)
      log->say(ILog::WARNING, "Tried to add an empty essential to ARecord object.", "ARecord");
  }
}

void ARecord::ClearEssentials()
{
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
  } else {
    //delete list of essentials
    m_essentials.clear();
  }
}

vector<string> ARecord::GetEssentials()
{
  return m_essentials;
}

ARecord::TEssentialsIterator ARecord::GetEssentialsIterBegin()
{
  return m_essentials.begin();
}

ARecord::TEssentialsIterator ARecord::GetEssentialsIterEnd()
{
  return m_essentials.end();
}

bool ARecord::HasEssential(string pEssential)
{
  if (find(m_essentials.begin(), m_essentials.end(), pEssential) != m_essentials.end())
    return true;
  else
    return false;
}

void ARecord::SetAccountId(unsigned long pAccountId)
{
  if (log)
    log->say(ILog::DEBUG, string("Setting new account ID in record:")+ m_accountName,"ARecord");
  if (GetLockStatus() != UNLOCKED) {
    if (log)
      log->say(ILog::ERROR, string("Tried to change account name of locked ARecord ") + m_accountName, "ARecord");
  } else {
    if (pAccountId > 0)
      m_accountId = pAccountId;
    else if (log)      
      log->say(ILog::ERROR, string("Trying to set zero or negarive m_accountId into ARecord ") + m_accountName, "ARecord");
  }
}

unsigned long ARecord::GetAccountId()
{
  return m_accountId;
}

ARecord::LockStatus ARecord::SetLock(LockStatus pLock)
{
  if (pLock >= UNLOCKED && pLock < NLOCKSTATES)
    m_lock = pLock;
  return m_lock;
}

ARecord::LockStatus ARecord::GetLockStatus()
{
  return m_lock;
}

int ARecord::SetCreationTime(std::string pTimeStr)
{
  bool hasValidTime(false);
  if (not pTimeStr.empty()) {
    //attempt to validate input time
    tm myTime;
    if (CSMUtils::parseStrDate(pTimeStr, &myTime) == 0) {
      m_creationTime = mktime(&myTime);
      m_lastModificationTime = m_creationTime;
      if (m_creationTime != -1)
	hasValidTime=true;
    }
  }
  if (not hasValidTime) {
    //get system time
    m_creationTime = time(&m_creationTime);
    m_lastModificationTime = m_creationTime;
  }
  return 0;
}

int ARecord::SetCreationTime(time_t pTime)
{
  if (pTime == 0) {
    m_creationTime = time(&m_creationTime);
    m_lastModificationTime = m_creationTime;
  } else {
    m_creationTime = pTime;
    m_lastModificationTime = pTime;
  }
  return 0;
}

time_t ARecord::GetCreationTime()
{
  return m_creationTime;
}

std::string ARecord::GetCreationTimeStr()
{
  struct tm* tmpCreationTime = localtime(&m_creationTime);  
  if (!tmpCreationTime) {
    log->say(ILog::FIXME, "Invalid date stored in ARecord", "ARecord");
    return std::string("00/00/0000");
  }
  ostringstream oss;
  oss << tmpCreationTime->tm_mday << "/"
      << tmpCreationTime->tm_mon+1 << "/" 
      << tmpCreationTime->tm_year+1900;
  return oss.str();
  //return asctime(tm);
}

int ARecord::SetModificationTime(std::string pTimeStr)
{
  bool hasValidTime(false);
  if (not pTimeStr.empty()) {
    //attempt to validate input time
    tm myTime;
    if (CSMUtils::parseStrDate(pTimeStr, &myTime) == 0) {
      m_lastModificationTime = mktime(&myTime);
      if (m_lastModificationTime != -1)
	hasValidTime=true;
    }
  }
  if (not hasValidTime) {
    //get system time
    m_lastModificationTime = time(&m_lastModificationTime);
  }
  return 0;
}

int ARecord::SetModificationTime(time_t pTime)
{
  if (pTime == 0) {
    m_lastModificationTime = time(&m_lastModificationTime);
  } else {
    m_lastModificationTime = pTime;
  }
  return 0;
}


time_t ARecord::GetModificationTime()
{
  return m_lastModificationTime;
}

std::string ARecord::GetModificationTimeStr()
{
  struct tm* tmpModTime = localtime(&m_lastModificationTime);  
  if (!tmpModTime) {
    log->say(ILog::FIXME, "Invalid date stored in ARecord", "ARecord");
    return std::string("00/00/0000");
  }
  ostringstream oss;
  oss << tmpModTime->tm_mday << "/"
      << tmpModTime->tm_mon+1 << "/" 
      << tmpModTime->tm_year+1900;
  return oss.str();
  //return asctime(tm);
}

