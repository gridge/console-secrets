/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: FormatterPlainTextTool.cc
 Description: Implements IFormatterTool with a plain comma/LF separated list of fields/records.
 Last Modified: $Id: FormatterPlainTextTool.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "FormatterPlainTextTool.h"
#include "ISecurityTool.h"

#include <sstream>

extern ILog *log;

using namespace std;

FormatterPlainTextTool::FormatterPlainTextTool(string pName) : IFormatterTool(pName)
{
  InitSeparators();
}


FormatterPlainTextTool::FormatterPlainTextTool(string pName, string pFormat) : IFormatterTool(pName, pFormat)
{
  InitSeparators();
}

FormatterPlainTextTool::~FormatterPlainTextTool()
{

}

void FormatterPlainTextTool::InitSeparators() {
  m_header = "---->>CSM_PLAIN_TEXT_FORMATTER Version 1.0";
  m_recordSep = "---->>";
  m_fieldSep = "@@";
  m_labelsField = "LABELS";
  m_essentialsField = "ESSENTIALS";
}

bool FormatterPlainTextTool::CheckFieldValue(std::string pFieldValue)
{
  //Check it does not start with the field separator
  if (pFieldValue.find(m_fieldSep) == 0)
    return false;
  //Also performs the same check as for field names
  return CheckFieldName(pFieldValue);
}

bool FormatterPlainTextTool::CheckFieldName(std::string pFieldName)
{
  //Check it does not starti with the record separator
  if (pFieldName.find(m_recordSep) == 0)
    return false;
  return true;
}

IErrorHandler::StatusCode FormatterPlainTextTool::MakeSafeField(std::string &pField)
{
  size_t idx1 = pField.find(m_fieldSep);
  if (idx1 != string::npos) {
    //replace it!
    pField.replace(idx1, m_fieldSep.size(), "");
  }
  size_t idx2 = pField.find(m_recordSep);
  if (idx2 != string::npos) {
    //replace it!
    pField.replace(idx1, m_recordSep.size(), "");
  }
  return SC_OK;
}

IErrorHandler::StatusCode FormatterPlainTextTool::Code(std::vector<ARecord *> pData, std::string &pFormattedString, int pBruteForce)
{
  m_statusCode = SC_OK;
  //will stream everything here, and then copy back to pFormattedString
  ostringstream outStream;
  //print header
  outStream<<m_header<<endl;
  for (std::vector<ARecord *>::iterator r = pData.begin(); r != pData.end(); ++r) {
    string strBuf; //temporary buffer
    //Start a new record
    outStream << m_recordSep << (*r)->GetAccountName() << endl;
    //Write Labels
    outStream << m_fieldSep << m_labelsField << endl;
    for (ARecord::TLabelsIterator lit = (*r)->GetLabelsIterBegin(); lit != (*r)->GetLabelsIterEnd(); ++lit) {
      strBuf = (*lit);
      if (!CheckFieldValue(strBuf)) {
	if (!pBruteForce) {
	  log->say(ILog::ERROR, string("Error while writing file. Label not valid: ") + strBuf, this);
	  return m_statusCode = SC_ERROR;
	} else {
	  //who cares.. write it anyway
	  if (MakeSafeField(strBuf) != SC_OK) {
	    log->say(ILog::ERROR, string("Error while writing file. Label not valid: ") + strBuf, this);
	    return m_statusCode = SC_ERROR;
	  }
	}
      }
      outStream<<strBuf<<endl;
    } // end writing labels
    //Write Essentials
    outStream << m_fieldSep << m_essentialsField << endl;
    for (ARecord::TEssentialsIterator eit = (*r)->GetEssentialsIterBegin(); eit != (*r)->GetEssentialsIterEnd(); ++eit) {
      strBuf = (*eit);
      if (!CheckFieldValue(strBuf)) {
	if (!pBruteForce) {
	  log->say(ILog::ERROR, string("Error while writing file. Essential not valid: ") + strBuf, this);
	  return m_statusCode = SC_ERROR;
	} else {
	  //who cares.. write it anyway
	  if (MakeSafeField(strBuf) != SC_OK) {
	    log->say(ILog::ERROR, string("Error while writing file. Essential not valid: ") + strBuf, this);
	    return m_statusCode = SC_ERROR;
	  }
	  m_statusCode = SC_WARNING;
	}
      }
      outStream<<strBuf<<endl;
    } // end writing essentials
    //Write fields
    for (ARecord::TFieldsIterator fit = (*r)->GetFieldsIterBegin(); fit != (*r)->GetFieldsIterEnd(); ++fit) {
      strBuf = fit->first;
      //check field name
      if (!CheckFieldName(strBuf)) {
	if (!pBruteForce) {
	  log->say(ILog::ERROR, string("Error while writing file. Field not valid: ") + strBuf, this);
	  return m_statusCode = SC_ERROR;	  
	} else {
	  //who cares.. write it anyway
	  if (MakeSafeField(strBuf) != SC_OK) {
	    log->say(ILog::ERROR, string("Error while writing file. Field not valid: ") + strBuf, this);
	    return m_statusCode = SC_ERROR;	  
	  }
	  m_statusCode = SC_WARNING;
	}
      }
      outStream<<m_fieldSep<<strBuf<<endl;
      strBuf = fit->second;
      if (!CheckFieldValue(strBuf)) {
	if (!pBruteForce) {
	  log->say(ILog::ERROR, string("Unfortunately the field value conflict with our standards. Field Name = ") + fit->first);
	  return m_statusCode = SC_ERROR;
	} else {
	  //write it anyway.. your problem.	  
	  m_statusCode = SC_WARNING;
	}
      }
      outStream << strBuf << endl;
    } //end loop over fields
  } //loop over records

  //flush stream to output and clear it
  pFormattedString = outStream.str();
  outStream.clear();
  return m_statusCode;
}

IErrorHandler::StatusCode FormatterPlainTextTool::Decode(std::string &pFormattedString, std::vector<ARecord *> &pData, int pBruteForce)
{
  m_statusCode = SC_OK;
  istringstream inStream;
  inStream.str(pFormattedString);
  string bufStr;
  //Read and check header
  getline(inStream, bufStr);
  if (bufStr != m_header) {
    log->say(ILog::ERROR, string("Error reading source. Header mismatch: ")+bufStr, this);
    return m_statusCode = SC_ERROR;
  }
  inStream.exceptions ( istringstream::eofbit );
  bool recordRequiredFields = false;
  ARecord *newRec = 0;
  string fieldName;
  string fieldValue;
  bool skip_record=false;
  try {
    ISecurityTool::ClearString(bufStr);
    getline(inStream, bufStr);
    while (true) {      
      if (bufStr.find(m_recordSep) == 0) {
	//new record!	
	if (newRec != 0) {
	  //not the first one, push previous record into pData
	  //add last field first
	  if ((!fieldName.empty()) && (!fieldValue.empty()))
	    newRec->AddField(fieldName, fieldValue);    
	  pData.push_back(newRec);
	}
	newRec = new ARecord;
	recordRequiredFields = false;
	skip_record = false;
	ISecurityTool::ClearString(fieldName);
	ISecurityTool::ClearString(fieldValue);
	//get account name
	newRec->SetAccountName(bufStr.substr(m_recordSep.length()));
	bool end_labels = false;
	bool end_essentials = false;
	//now read labels
	ISecurityTool::ClearString(bufStr);
	getline(inStream, bufStr);
	if (bufStr != (m_fieldSep + string("LABELS"))) {
	  //we were expecting labels...
	  if (!pBruteForce) {
	    log->say(ILog::ERROR, string("Expecting LABELS field: ") + bufStr, this);
	    ISecurityTool::ClearString(bufStr);
	    ISecurityTool::ClearStrBuffer(inStream);
	    return m_statusCode = SC_ERROR;
	  } else {
	    //whatever.. let's skip it
	    log->say(ILog::WARNING, string("Not found LABELS field in record: ")+newRec->GetAccountName(), this);
	    end_labels = true;
	  }	  
	}
	while (!end_labels) {
	  //read all labels
	  ISecurityTool::ClearString(bufStr);
	  getline(inStream, bufStr);
	  if (bufStr.find(m_recordSep) == 0) {
	    //we were not expecting this here!
	    if (!pBruteForce) {
	      log->say(ILog::ERROR, string("Not expecting new record here: ") + bufStr, this);
	      ISecurityTool::ClearString(bufStr);
	      ISecurityTool::ClearStrBuffer(inStream);
	      return m_statusCode = SC_ERROR;
	    } else {
	      //whatever.. let's skip to the new one
	      log->say(ILog::WARNING, string("Skipping to a new record without all info from the previous one: ")+newRec->GetAccountName(), this);
	      end_labels = true;
	      end_essentials = true;
	      skip_record = true;
	    }
	  } else {
	    if (bufStr.find(m_fieldSep) == 0) {
	      end_labels=true;
	    } else {
	      newRec->AddLabel(bufStr);	      
	    } //add new label
	  }
	} // end labels
	//now read essentials
	if (bufStr != (m_fieldSep + string("ESSENTIALS"))) {
	  //we were expecting essentials...
	  if (!pBruteForce) {
	    log->say(ILog::ERROR, string("Expecting ESSENTIALS field: ") + bufStr, this);
	    ISecurityTool::ClearString(bufStr);
	    ISecurityTool::ClearStrBuffer(inStream);
	    return m_statusCode = SC_ERROR;
	  } else {
	    //whatever.. let's skip it
	    log->say(ILog::WARNING, string("Not found ESSENTIALS field in record: ")+newRec->GetAccountName(), this);
	    end_essentials = true;
	  }	  
	}
	while (!end_essentials) {
	  //read all essentials
	  ISecurityTool::ClearString(bufStr);
	  getline(inStream, bufStr);
	  if (bufStr.find(m_recordSep) == 0) {
	    //we were not expecting this here!
	    if (!pBruteForce) {
	      log->say(ILog::ERROR, string("Not expecting new record here: ") + bufStr, this);
	      ISecurityTool::ClearString(bufStr);
	      ISecurityTool::ClearStrBuffer(inStream);
	      return m_statusCode = SC_ERROR;
	    } else {
	      //whatever.. let's skip to the new one
	      log->say(ILog::WARNING, string("Skipping to a new record without all info from the previous one: ")+newRec->GetAccountName(), this);
	      end_labels = true;
	      end_essentials = true;
	      skip_record = true;
	    }
	  } else {
	    if (bufStr.find(m_fieldSep) == 0) {
	      end_essentials=true;
	    } else {
	      newRec->AddEssential(bufStr, true); //need to force, since fields are not loaded yet
	    } //add new essential
	  }	  
	} // end essentials
	recordRequiredFields = true;
      } // new record     
      // now read fields. Can be multi-line and optional
      if (bufStr.find(m_fieldSep) == 0) {
	//new field
	if (fieldName.empty() && (!fieldValue.empty())) {
	  //we were expecting a field name here
	  if (!pBruteForce) {
	    log->say(ILog::ERROR, string("Expecting FIELD declaration: ")+bufStr, this);
	    ISecurityTool::ClearString(bufStr);
	    ISecurityTool::ClearString(fieldValue);
	    ISecurityTool::ClearStrBuffer(inStream);
	    return m_statusCode = SC_ERROR;
	  } else {
	    //whatever.. put a dummy value
	    fieldName = "csmRecoveredField";
	  }
	}
	if ((!fieldName.empty()) && (!fieldValue.empty())) {
	  //should not happend only for the first field
	  newRec->AddField(fieldName, fieldValue);
	}
	//new fields	
	ISecurityTool::ClearString(fieldValue);
	fieldName = bufStr.substr(m_fieldSep.length()); 
	ISecurityTool::ClearString(bufStr);
	getline(inStream, bufStr); //read value
      } // new field
      if (!skip_record) {
	//add field value
	if (!fieldValue.empty())
	  fieldValue += '\n'; //multi-line input, no \n at the end of the string
	fieldValue += bufStr;
	getline(inStream, bufStr); //read next line
      }
    } //loop over text lines
  } catch (istringstream::failure e) {    
    //add last field
    if ((!fieldName.empty()) && (!fieldValue.empty()))
      newRec->AddField(fieldName, fieldValue);    
  }
  //end of file, just check last record is complete
  if (!recordRequiredFields)  {
    if (!pBruteForce) {
      log->say(ILog::ERROR, "Last record read is not complete!", this);
      ISecurityTool::ClearString(fieldValue);
      ISecurityTool::ClearStrBuffer(inStream);
      return m_statusCode = SC_ERROR;
    } else {
      //just print a warning
      log->say(ILog::WARNING, "Last record read is not complete!", this);
      m_statusCode = SC_WARNING;
    }
  }
  pData.push_back(newRec);

  ISecurityTool::ClearStrBuffer(inStream);
  ISecurityTool::ClearString(bufStr);
  ISecurityTool::ClearString(fieldName);
  ISecurityTool::ClearString(fieldValue);
  return m_statusCode;
}
