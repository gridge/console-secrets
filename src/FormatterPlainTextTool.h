/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: FormatterPlainTextTool.h
 Description: Implements IFormatterTool with a plain comma/LF separated list of fields/records.
 Last Modified: $Id: FormatterPlainTextTool.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __FORMATTERPLAINTEXT_TOOL__
#define __FORMATTERPLAINTEXT_TOOL__

#include "IFormatterTool.h"

#include <string>
#include <sstream>

/** Implements IFormatterTool with a simple plain text format.
 * Code/decode a ARecord vector to/from a plain text string. 
 * The format of the file is the following:
 * ---->>CSM_PLAIN_TEXT_FORMATTER Version 1.0
 * ---->><AccountName>
 * @@LABELS
 * Labels_1
 * Labels_2
 * ...
 * @@ESSENTIALS
 * Essentials_1
 * Essentials_2
 * ...
 * @@<FieldName_1>
 * <FieldValue_1
 * @@<FieldName_2>
 * <FieldValue_2>
 * ...
 * --<
 * IMPORTANT: For sake of simplicity the following rules apply:
 *  - The double @@ are prohibited at the beginning of FieldValues
 *  - No FieldName, FieldValue, Label or Essential can contain the record delimited '---->>'
 * If rules are violated and pBruteForce is set, something is still tried.. otherwise return an error
 */

class FormatterPlainTextTool : public IFormatterTool {
 protected:
  /// Keep versioning number
  std::string m_versionNumber;
  /// File header for versioning.
  std::string m_header;
  /// Lines starting with these characthers define the start of a new record
  std::string m_recordSep;
  /// Used inside a record to start a new fields
  std::string m_fieldSep;
  /// Special field for ARecord::m_creationTime
  std::string m_creationTimeField;
  /// Special field for ARecord::m_lastModificationTime
  std::string m_modificationTimeField;
  /// Special field for ARecord::m_labels
  std::string m_labelsField;
  /// Special field for ARecord::m_essentials
  std::string m_essentialsField;

  /// Init separators values
  void InitSeparators();

  /// Check filled values respect rules
  bool CheckFieldValue(std::string pFieldValue);
  bool CheckFieldName(std::string pFieldName);

  /// Edit string to make it as the rules want
  StatusCode MakeSafeField(std::string &pField);

  /// Decoding function evolution schema for m_format = "1.0"
  StatusCode Decode_v1(std::istringstream &inStream, std::vector<ARecord *> &pData, int pBruteForce=0);
  /// Decoding function for m_format = "2.0"
  StatusCode Decode_v2(std::istringstream &inStream, std::vector<ARecord *> &pData, int pBruteForce=0);

 public:
  FormatterPlainTextTool(std::string pName);
  FormatterPlainTextTool(std::string pName, std::string pFormat);
  ~FormatterPlainTextTool();
  
  /// See IFormatterTool::Code and class description
  virtual StatusCode Code(std::vector<ARecord *> pData, std::string &pFormattedString, int pBruteForce=0);  
  
  /// See IFormatterTool::Code and class description
  virtual StatusCode Decode(std::string &pFormattedString, std::vector<ARecord *> &pData, int pBruteForce=0);

};

#endif
