/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: IErrorHandler.cc
 Description: Base class for handling return codes and error messages.
 Last Modified: $Id: IErrorHandler.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "IErrorHandler.h"

#include <string>

using namespace std;

IErrorHandler::IErrorHandler(std::string pName)
{
  m_name = pName;
  m_statusCode = SC_OK;
}

IErrorHandler::~IErrorHandler()
{
  m_statusCode = SC_OK;
}

IErrorHandler::StatusCode IErrorHandler::GetErrorMsg(std::string &pMsg)
{
  pMsg = m_errorMsg;
  m_errorMsg.clear();
  StatusCode sc = m_statusCode;
  m_statusCode = SC_OK; //clear status code
  return sc;
}

string IErrorHandler::GetErrorMsg()
{
  string tmpMsg = m_errorMsg;
  m_errorMsg.clear();  
  m_statusCode = SC_OK; //clear status code
  return tmpMsg;
}

