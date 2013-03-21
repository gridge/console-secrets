/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IStorageTool.cc
 Description: Interface for physical load/storage of data
 Last Modified: $Id: IStorageTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ILog.h"
#include "IStorageTool.h"

// extern declaration for global instance (csm.h)
extern ILog *log;

using namespace std;

IStorageTool::IStorageTool(string pName, SourceURI pSource) : IErrorHandler(pName)
{
  *log << ILog::DEBUG << "Creating new storage tool: " << pName << "; for source: " << pSource.GetFullURI() << this << ILog::endmsg;
  m_source = pSource;
}

IStorageTool::~IStorageTool()
{
  *log << ILog::DEBUG << "Destroying StorageTool: " << m_name << this << ILog::endmsg;  
}

SourceURI IStorageTool::GetSource()
{
  return m_source;
}

IErrorHandler::StatusCode IStorageTool::StorageExists()
{
  //if we can't check, resource does not exists
  return SC_NOT_IMPLEMENTED;
}
