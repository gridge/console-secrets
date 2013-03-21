/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ISecurityTool.cc
 Description: Interface for cryptography and password strenght checks
 Last Modified: $Id: ISecurityTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ISecurityTool.h"

// extern declaration for global instance (csm.h)
#include "ILog.h"
extern ILog *log;

ISecurityTool::ISecurityTool(std::string pName) : IErrorHandler(pName)
{
  *log << ILog::DEBUG << "Creating new SecurityTool: " << pName << this << ILog::endmsg;
}

ISecurityTool::~ISecurityTool()
{
  *log << ILog::DEBUG << "Destroying SecurityTool: " << m_name << this << ILog::endmsg;
}

IErrorHandler::StatusCode ISecurityTool::SetKey(std::string pKey)
{
  m_key = pKey;
  return SC_OK;
}


std::string ISecurityTool::GetKey()
{
  return m_key;
}

void ISecurityTool::ClearString(std::string& str)
{
  std::fill(str.begin(), str.end(), '\0');
  str.clear();
}

void ISecurityTool::ClearStrBuffer(std::istringstream& str)
{
  //@todo should overwrite the (remaining) content in memory somehow, need to think a bit more about it
  str.str(std::string());
}
