/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IIOService.cc
 Description: Implement Interface for IOService
 Last Modified: $Id: IIOService.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "IIOService.h"

// extern declaration for global instance (csm.h)
extern ILog *log;

#include <sstream>

using namespace std;

// --- Implement SourceURI class members

IIOService::IIOService(string pName) : IErrorHandler(pName)
{
  m_localIdMgrInstance = false;
}

IIOService::IIOService(string pName, string owner) : IErrorHandler(pName)
{
  m_owner = owner;
  m_localIdMgrInstance = false;
}

IIOService::~IIOService()
{
  //free IdManagerTool only if it's a local instance (not shared by other IIOService instances)
  if (m_localIdMgrInstance && m_idManagerTool)
    delete m_idManagerTool;
}

IErrorHandler::StatusCode IIOService::Load(SourceURI pSource)
{
  SetSource(pSource);
  if (m_statusCode >= SC_ERROR)
    return m_statusCode;
  return Load();
}

IErrorHandler::StatusCode IIOService::SourceExists()
{
  //default: unable to check
  return SC_NOT_IMPLEMENTED;
}

IErrorHandler::StatusCode IIOService::Store(SourceURI pSource)
{
  SetSource(pSource);
  if (m_statusCode >= SC_ERROR)
    return m_statusCode;
  return Store();
}

IErrorHandler::StatusCode IIOService::Add(vector<ARecord*> pARecords, bool flushBuffer)
{
  StatusCode intermediateSC = SC_OK;
  for (vector<ARecord*>::iterator r=pARecords.begin(); r!=pARecords.end(); ++r) {
    StatusCode curSC = Add(*r, flushBuffer);
    if (curSC != SC_OK) {
      ostringstream msg;
      msg << "Unable to insert record " << (*r)->GetAccountName() << "(accountId = " << (*r)->GetAccountId() << ")";
      log->say(ILog::ERROR, msg.str().c_str());
    }
    intermediateSC = curSC;
  }
  return intermediateSC;
}

IErrorHandler::StatusCode IIOService::Remove(ARecord *pRecord)
{
  //just call the method Remove(unsigned long pAccountId)
  *log << ILog::DEBUG << "Requested remove of record with account Id: " << pRecord->GetAccountId() << this << ILog::endmsg;
  return Remove(pRecord->GetAccountId());
}

void IIOService::SetOwner(std::string pOwner)
{
  if (!pOwner.empty())
    m_owner = pOwner;
  else
    log->say(ILog::ERROR, "Tried to assign empty owner to IIOService.", this);
}

std::string IIOService::GetOwner()
{
  return m_owner;
}

void IIOService::SetKey(std::string pKey)
{
  if (!pKey.empty())
    m_key = pKey;
  else
    log->say(ILog::ERROR, "Tried to assign empty key to IIOService.", this);
}

std::string IIOService::GetKey()
{
  return m_key;
}

IErrorHandler::StatusCode IIOService::SetSource(SourceURI pSource)
{
  m_source = pSource;
  return SC_OK;
}

SourceURI IIOService::GetSource()
{
  return m_source;
}

void IIOService::SetIdManagerTool(IdManagerTool *mTool)
{
  m_idManagerTool = mTool;
}

