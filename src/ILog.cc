/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ILog.cc
 Description: Interface for logging and error reporting utility
 Last Modified: $Id: ILog.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "ILog.h"

#include <algorithm>

//#include <typeinfo>

using namespace std;

ILog::ILog(string pName) : IErrorHandler(pName)
{
  InitValues();
}

void ILog::InitValues()
{
  m_msgTypeStr[FATAL] = "FATAL";
  m_msgTypeStr[FIXME] = "FIXME";
  m_msgTypeStr[ERROR] = "ERROR";
  m_msgTypeStr[WARNING] = "WARNING";
  m_msgTypeStr[INFO] = "INFO";
  m_msgTypeStr[VERBOSE] = "VERBOSE";
  m_msgTypeStr[DEBUG] = "DEBUG";
  // default: mute log messages. None displayed
  Mute();

  //internal usage
  m_bufferMsgType = FATAL; //this default is never used if ILog is properly called
  m_bufferMsgCaller = "Unknown"; //this default is never used if ILog properly called
}

ILog::~ILog()
{

}

void ILog::Init()
{

}

IErrorHandler::StatusCode ILog::SetLogDetail(ILogMsgType pLogDetail)
{
  //check boundaries, 'endmsg' is allowed as no-logging
  if (pLogDetail >= -1 && pLogDetail < endmsg) {
    m_logDetail = pLogDetail;
    return SC_OK;
  }
  return SC_ERROR;
}

IErrorHandler::StatusCode ILog::Mute()
{
  return SetLogDetail(endmsg);
}

IErrorHandler::StatusCode ILog::GetStatusCodeFromMsg(ILogMsgType msg)
{
  switch (msg) {
  case FATAL:
  case FIXME:
    return SC_FATAL;
  case ERROR:
    return SC_ERROR;
  case WARNING:
    return SC_WARNING;
  case INFO:
  case VERBOSE:
  case DEBUG:
    return SC_OK;
  default:
    return SC_ERROR;
  }
  return SC_ERROR;
}

void ILog::say(ILogMsgType msgType, std::string msg, IErrorHandler* callerObj)
{
  if (callerObj != 0) {
    // update also status code only if it's a more serious comunication than what already stored
    if (GetStatusCodeFromMsg(msgType) >= callerObj->m_statusCode) {
      callerObj->m_errorMsg = msg;
      callerObj->m_statusCode = GetStatusCodeFromMsg(msgType);
    }
    say(msgType, msg, callerObj->m_name);
  } else {
    say(msgType, msg);
  }
}

ILog &ILog::operator<<(const std::string pMsg)
{
  //just add to the buffer message
  m_bufferMsg << pMsg;
  return *this;
}

ILog &ILog::operator<<(const char* pMsg)
{
  m_bufferMsg << pMsg;
  return *this;
}

ILog &ILog::operator<<(const int pMsg)
{
  m_bufferMsg << pMsg;
  return *this;
}

ILog &ILog::operator<<(const unsigned long pMsg)
{
  m_bufferMsg << pMsg;
  return *this;
}

ILog &ILog::operator<<(const double pMsg)
{
  m_bufferMsg << pMsg;
  return *this;
}

ILog &ILog::operator<<(const ILogMsgType pMsgType)
{
  if (pMsgType != endmsg) {
    //change message type and create new output
    if (!m_bufferMsg.str().empty()) {
      //a previous message is still here.. flush it!
      if (m_bufferMsgCallerPtr)
	say(m_bufferMsgType, m_bufferMsg.str(), m_bufferMsgCallerPtr);
      else
	say(m_bufferMsgType, m_bufferMsg.str(), m_bufferMsgCaller);
    }
    m_bufferMsgType = pMsgType; 
  } else {
    //end-of-message detected, it's time to log it
    if (m_bufferMsgCallerPtr)
      say(m_bufferMsgType, m_bufferMsg.str(), m_bufferMsgCallerPtr); //no check on content.. user responsibility    
    else
      say(m_bufferMsgType, m_bufferMsg.str(), m_bufferMsgCaller); //no check on content.. user responsibility          
  }
  //in any case a new log message is starting
  //set default values
  m_bufferMsgCaller = string("Unknown");
  m_bufferMsgCallerPtr = 0;
  m_bufferMsg.str("");
  return *this;
}

ILog &ILog::operator<<(IErrorHandler *pCaller)
{
  m_bufferMsgCaller = pCaller->m_name; //we're friends :)
  m_bufferMsgCallerPtr = pCaller;
  return *this;
}

ILog &ILog::operator<<(IErrorHandler &pCaller)
{
  m_bufferMsgCaller = pCaller.m_name; //we're friends :)
  m_bufferMsgCallerPtr = &pCaller;
  return *this;
}

IErrorHandler::StatusCode ILog::GetMsgTypeFromStr(ILogMsgType &out, string msgTypeStr)
{
  string inUp = msgTypeStr;
  std::transform(msgTypeStr.begin(), msgTypeStr.end(), inUp.begin(), ::toupper);
  for (int imsg=FATAL; imsg < endmsg; imsg++) {
    string cmpUp = m_msgTypeStr[imsg];
    std::transform(m_msgTypeStr[imsg].begin(), m_msgTypeStr[imsg].end(), cmpUp.begin(), ::toupper);
    if (inUp == cmpUp) {
      out = (ILogMsgType)imsg;
      return SC_OK;
    }
  }
  return SC_ERROR;
}
