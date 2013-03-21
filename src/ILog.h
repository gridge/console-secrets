/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ILog.h
 Description: Interface for logging and error reporting utility
 Last Modified: $Id: ILog.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ILOG_SERVICE__
#define __ILOG_SERVICE__

#include "IErrorHandler.h"

#include <string>
#include <sstream>

/** Interface for logging service.
 * Declare virtual methods for logging of messages.
 * The proper usage to log messages is to call function say() or
 * to use operators '<<' started by an ILogMsgType and finished by ILogMsgType::endmsg.
 * Ex. log << ILog::FATAL << "I like it!" << my_integer << ILog::endmsg;
 * P.s. If you forget ILog::endmsg, your message will be steamed only after the next ILogMsgType
 */
class ILog : public IErrorHandler {
 public:
  /** Enum for message type.
   * Defines the types of messages available
   */
  enum ILogMsgType {
    FATAL=0, ///< Fatal errors
    FIXME, ///< Bugs catched inside the program
    ERROR, ///< Error messages
    WARNING, ///< Warning messages
    INFO, ///< Standard information messages
    VERBOSE, ///< Verbose printing
    DEBUG, ///< Debug (even more verbose) printing
    endmsg ///< Number of different message types and also used to "flush" the request when used with operator <<
  };

  /// Control which ILogMsgType will be logged
  virtual StatusCode SetLogDetail(ILogMsgType pLogDetail);
  /// Shortcuts for SetLogDetail(-1)
  virtual StatusCode Mute();

 protected:
  /** Control which ILogMsgType will be logged. 
   * Display messages with ILogMsgType <= m_logDetail. m_logDetail = -1 is no log.
   */
  ILogMsgType m_logDetail;

  /// String version of the ILogMsgType enum
  std::string m_msgTypeStr[endmsg];

  std::ostringstream m_bufferMsg; ///< keep message into buffer until an "endmsg" is given in case of streaming with '<<'
  ILogMsgType m_bufferMsgType; ///< keep message type until an "endmsg" is given in case of streaming with '<<'
  std::string m_bufferMsgCaller; ///< caller name, can be set streaming an (IErrorHandler *) object.. nice :)
  IErrorHandler* m_bufferMsgCallerPtr; ///< caller pointer. If present, set m_statusCode and error message.

  // Define statuCode to be set to the caller from log message type
  StatusCode GetStatusCodeFromMsg(ILogMsgType msg);
 public:  
  //Define operators '<<' for log streaming
  ILog &operator<<(const std::string pMsg); ///< streaming to log for string
  ILog &operator<<(const char* pMsg); ///< streaming to log for const char*
  ILog &operator<<(const int pMsg); ///< streaming to log for int
  ILog &operator<<(const unsigned long pMsg); ///< streaming to log for unsigned int
  ILog &operator<<(const double pMsg); ///< streaming to log for double
  ILog &operator<<(const ILogMsgType pMsgType); ///< streaming to log type and end-of-message
  ILog &operator<<(IErrorHandler *pCaller); ///< streaming to log the caller name
  ILog &operator<<(IErrorHandler &pCaller); ///< streaming to log the caller name

  /// Initializer
  void InitValues();

  ///Utility, convert string to ILogMsgType
  StatusCode GetMsgTypeFromStr(ILogMsgType &out, std::string msgTypeStr);

 public:
  ILog(std::string pName);
  ~ILog();

  /// Initialization after configuration of CSM is done (if needed)
  virtual void Init();

  /** Log message from callerObj
   * @param msgType Type of message as defined in ILogMsgType
   * @param msg Message to be logged
   * @param callerObj (optional) object which call the method (useful for error reporting)
  */
  virtual void say(ILogMsgType msgType, std::string msg, IErrorHandler* callerObj);
  virtual void say(ILogMsgType msgType, std::string msg, std::string callerObj="") = 0;

  //friend operators for easy streaming
  
};

#endif
