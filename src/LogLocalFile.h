/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LogLocalFile.h
 Description: Implements ILog for logging to a local file
 Last Modified: $Id: LogLocalFile.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __LOGLOCALFILE_SERVICE__
#define __LOGLOCALFILE_SERVICE__

#include "ILog.h"

#include <iostream>
#include <fstream>

/// Implements ILog interface on local files. 
class LogLocalFile : public ILog {
 protected:
  /// Store local file name
  std::string m_localFileName;
  /// ofstream object to write to local file
  std::ofstream m_localFile;

 public:
  /// Open/close logging file
  LogLocalFile(std::string pName);
  ~LogLocalFile();

  // --- Implementation of base class virtual function
  /// Initialization after configuration of CSM to open output file
  virtual void Init();

  /// Implements ILog::say() to write to file
  virtual void say(ILogMsgType msgType, std::string msg, std::string callerObj="");

  /// Implement setting level of detail to keep track of it
  virtual StatusCode SetLogDetail(ILogMsgType pLogDetail);  

  // --- Accessores
  /// Set m_localFileName property
  void SetLocalFileName(std::string pFileName);
  /// Get m_localFileName property
  std::string GetLocalFileName();

};

#endif
