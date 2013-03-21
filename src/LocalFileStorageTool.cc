/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: LocalFileStorageTool.cc
 Description: Implementation of IStorageTool for local file load/storage of data
 Last Modified: $Id: LocalFileStorageTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "LocalFileStorageTool.h"
#include "ILog.h"

#include <string>
#include <sstream>
#include <sys/stat.h>


extern ILog *log;

using namespace std;

LocalFileStorageTool::LocalFileStorageTool(string pName, SourceURI pSource) : IStorageTool(pName, pSource)
{
  // open/close of file will be done when requested
}

LocalFileStorageTool::~LocalFileStorageTool()
{
}

IErrorHandler::StatusCode LocalFileStorageTool::Store(std::string &pData)
{
  *log << ILog::VERBOSE << "Writing to source: " << m_source.GetFullURI() << "(" << (unsigned long)pData.size() << " bytes)" << this << ILog::endmsg;
  //open file
  if (m_file.is_open()) {
    log->say(ILog::WARNING, "File already opened. Closing and opening for storage.", this);
    m_file.close();    
  }
  string fileName = GetLocalFileName(m_source);;
  string backupFileName;
  if (FileExists(fileName)) {
    log->say(ILog::INFO, "File already exists. This is OK. We'll make a backup and overwrite its content.", this);
    ifstream origFile(fileName.c_str(), ios::binary);
    if (!origFile.is_open()) {
      log->say(ILog::WARNING, string("Error opening ")+fileName, this);
      //      return m_statusCode = IErrorHandler::SC_ERROR;
      // let's try to write it anyway.. who knows..
    } else {
      //open backup file
      backupFileName = fileName;
      backupFileName = backupFileName + string(".bak");
      ofstream backupFile(backupFileName.c_str(), ios::binary | ios::trunc);
      if (!backupFile.is_open()) {
	log->say(ILog::WARNING, string("Cannot make backup file ") + backupFileName, this);
      } else {
	//finally.. copy the original file, and close them
	try {
	  backupFile << origFile.rdbuf();
	  backupFile.close();
	  origFile.close();
	} catch (char * str) {
	  //Error in I/O
	  log->say(ILog::WARNING, string("I/O Error while making backup file ") + backupFileName, this);
	}	
      }
    }    
  } //FileExists(...)

  //ok, now open, write and return
  log->say(ILog::INFO, string("Opening file ") + fileName, "LocalFileStorageTool");  
  m_file.open(fileName.c_str(), fstream::out | fstream::trunc);
  if (!m_file.is_open()) {
    //Error opening output file
    log->say(ILog::ERROR, string("Error opening output file ") + fileName, this);
    return m_statusCode = SC_ERROR;
  }
  try {
    m_file << pData;
  } catch (char *str) {
    //Error I/O
    log->say(ILog::ERROR, string("I/O Error while writing output file ") + fileName + 
	     string(". You can recovery backup file (if exists) in ") + backupFileName, this);
    return m_statusCode = SC_ERROR;
  }
  m_file.close();

  return SC_OK;
}

IErrorHandler::StatusCode LocalFileStorageTool::Load(std::string &pData)
{
  //open file
  if (m_file.is_open()) {
    log->say(ILog::WARNING, "File already opened. Closing and opening for loading.", this);
    m_file.close();    
  }
  string fileName = GetLocalFileName(m_source);
  m_file.open(fileName.c_str(), fstream::in);
  if (!m_file.is_open()) {
    //Error opening input file
    log->say(ILog::ERROR, string("Error loading data from file ") + fileName, this);
    if (!FileExists(fileName))
      return m_statusCode = SC_NOT_FOUND; // say the the file does not exists explicitly!
    else
      return m_statusCode = SC_ERROR; //other errors may have occurred (ex. wrong permissions)
  }

  //read data
  ostringstream pDataStream;
  string bufStr;
  try {    
    while (getline(m_file, bufStr))
      pDataStream << bufStr << endl;
  } catch (char *str) {
    //Error I/O
    log->say(ILog::ERROR, string("I/O Error while reading input file ") + fileName, this);
    return m_statusCode = SC_ERROR;
  }
  pData = pDataStream.str();
  //close file
  m_file.close();

  //log->say(ILog::DEBUG, string("Loaded file. File content:\n") + pData);
  
  //great, bye
  return SC_OK;
}

string LocalFileStorageTool::GetLocalFileName(SourceURI pSource) 
{
  string fileName = m_source.GetField(SourceURI::NAME);
  fileName += ".";
  fileName += m_source.GetField(SourceURI::FORMAT);
  return fileName;
}

// This routine was shamelessly copied from http://www.techbytes.ca/techbyte103.html
bool LocalFileStorageTool::FileExists(string strFilename)
{
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. 
    // In the second case, any attempt to read/write a new file
    // will fail in this Tool and will be reported soon.
    blnReturn = false;
  }
  
  return(blnReturn);   
}

IErrorHandler::StatusCode LocalFileStorageTool::StorageExists()
{
  if (FileExists(GetLocalFileName(m_source)))
    return SC_OK;
  
  return SC_NOT_FOUND;
}
