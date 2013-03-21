/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LogLocalFile.cc
 Description: Implements ILog for logging to a local file
 Last Modified: $Id: LogLocalFile.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "LogLocalFile.h"

extern ILog *log;

using namespace std;

LogLocalFile::LogLocalFile(string pName) : ILog(pName)
{

}

LogLocalFile::~LogLocalFile()
{
  //Close log file
  if (m_localFile.is_open()) {
    m_localFile.flush();
    m_localFile.close();
  }
}

IErrorHandler::StatusCode LogLocalFile::SetLogDetail(ILogMsgType pLogDetail) {
  log->say(INFO, "Changing log details to: "+m_msgTypeStr[pLogDetail]);
  return ILog::SetLogDetail(pLogDetail);
}

string LogLocalFile::GetLocalFileName() 
{
  return m_localFileName;
}

void LogLocalFile::SetLocalFileName(std::string pFileName)
{
  m_localFileName = pFileName;
}

void LogLocalFile::Init()
{
  if (!m_localFileName.empty()) {
    m_localFile.open(m_localFileName.c_str());
    if (!m_localFile.is_open()) {
      //We're deperate.. use cerr to comunicate the error
      cerr << "[FATAL] (LogLocalFile): Unable to open log file: " << m_localFileName << endl;
      //this is a fatal error, throw exception
      throw;
    }
    log->say(ILog::INFO, string("Starting log file: ") + m_localFileName, this);
  } else {
    //no log file provided. Mute logging
    Mute();
  }
}

void LogLocalFile::say(ILogMsgType msgType, std::string msg, std::string callerObj)
{
  //Provide basic output to m_localFileName
  //First check if we're supposed to log the message
  if (msgType > m_logDetail || m_logDetail == endmsg)
    return;

  if (!m_localFile.is_open()) {
    //we've a serious error here.. tell me please!
    cerr << "[FATAL] (LogLocalFile): Cannot write on log file: " << m_localFileName << endl;
    throw;
  }

  //Log the event
  m_localFile << "[" << m_msgTypeStr[msgType] << "] ";
  for (int i=m_msgTypeStr[msgType].size(); i<= 12; i++)
    m_localFile << " "; //fill space
  if (!callerObj.empty()) 
    m_localFile << "(" << callerObj.substr(0,13) << ") "; //limit to 12 chars
  for (int i=callerObj.size(); i<= 12; i++)
    m_localFile << " "; //fill space
  m_localFile << msg << flush << endl;
}

