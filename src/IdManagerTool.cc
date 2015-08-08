/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IdManagerTool.cc
 Description: Implements Interface IIdManagerTool
 Last Modified: $Id: IdManagerTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "IdManagerTool.h"
#include "ILog.h"

extern ILog *log;

using namespace std;

IdManagerTool::IdManagerTool(string pName) : IErrorHandler(pName)
{

}

IdManagerTool::~IdManagerTool()
{

}

unsigned long IdManagerTool::GetNewId(std::string pSource)
{
  // for now use plain incremental Ids, start from 1.
  unsigned long newId=0; //zero is reserved, and indicates an error
  newId = m_idData.size()+1;
  m_idData[newId] = pSource;

  *log << ILog::DEBUG << "New Id requested: " << newId << " for source: " << pSource << this << ILog::endmsg;

  return newId;
}

string IdManagerTool::GetSource(unsigned long pId)
{
  string pS;
  idDataType::iterator idx = m_idData.find(pId);

  //debug
  *log << ILog::DEBUG << "List of accounts in Id Manager." << this << ILog::endmsg;
  for (idDataType::iterator itId = m_idData.begin(); itId != m_idData.end(); ++itId) {
    *log << ILog::DEBUG << itId->first << " - " << itId->second << this << ILog::endmsg;
  }


  if (idx != m_idData.end()) {
    //found
    pS = idx->second;
    *log << ILog::DEBUG << "Requested source for Id: " << pId << ". Found it: " << pS << this << ILog::endmsg;
  }
  return pS;
}

vector<unsigned long> IdManagerTool::GetIdList(std::string pSource)
{
  vector<unsigned long> rIds;
  for (idDataType::iterator idx = m_idData.begin(); idx != m_idData.end(); ++idx) {
    if (idx->second == pSource)
      rIds.push_back(idx->first);
  }
  return rIds;
}

IErrorHandler::StatusCode IdManagerTool::FreeId(unsigned long pId)
{
  idDataType::iterator idx = m_idData.find(pId);
  if (idx == m_idData.end()) {
    //not found
    log->say(ILog::WARNING, string("Requested to free un-managed Id."));
    return SC_WARNING;
  }
  m_idData.erase(idx);
  return SC_OK;
}
  
IErrorHandler::StatusCode IdManagerTool::FreeIdBySource(std::string pSource)
{
  for (idDataType::iterator idx = m_idData.begin(); idx != m_idData.end(); ++idx) {
    if (idx->second == pSource) {
      m_idData.erase(idx); 
    }
  }
  return SC_OK;
}
