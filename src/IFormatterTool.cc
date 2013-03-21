/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IFormatterTool.cc
 Description: Interface for data formatter tools
 Last Modified: $Id: IFormatterTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "IFormatterTool.h"

// extern declaration for global instance (csm.h)
extern ILog *log;

using namespace std;

IFormatterTool::IFormatterTool(string pName) : IErrorHandler(pName)
{
  *log << ILog::DEBUG << "Creating new formatter tool: " << pName << this << ILog::endmsg;
  m_format = "";
}

IFormatterTool::IFormatterTool(string pName, string pFormat) : IErrorHandler(pName)
{
  *log << ILog::DEBUG << "Creating new formatter tool: " << pName << "; Format = " << pFormat << this << ILog::endmsg;
  m_format = pFormat;
}

IFormatterTool::~IFormatterTool()
{
  *log << ILog::DEBUG << "Destroying FormatterTool: " << m_name << this << ILog::endmsg;
}

std::string IFormatterTool::GetFormat()
{
  return m_format;
}
