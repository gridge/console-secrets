/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: SourceURI.cc
 Description: Implement format of data source
 Last Modified: $Id: SourceURI.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "SourceURI.h"
#include "IConfigurationService.h"

using namespace std;

//extern declarations
extern IConfigurationService *cfgMgr;
extern ILog *log; 

SourceURI::SourceURI()
{
  SetDefaultSeparators();
}

SourceURI::SourceURI(std::string pSource)
{  
  SetDefaultSeparators();
  SetURI(pSource);
}

SourceURI::SourceURI(std::string pName, std::string pType, std::string pFormat)
{
  SetDefaultSeparators();
  m_source.clear();
  if (!pType.empty()) {
    m_source += pType + m_typeSeparator;
  }
  m_source += pName;
  m_source += m_formatSeparator;
  m_source += pFormat;
}

SourceURI::~SourceURI()
{

}

void SourceURI::SetDefaultSeparators()
{
  m_typeSeparator = "://";
  m_formatSeparator = ".";
}

void SourceURI::SetURI(std::string pSource)
{
  m_source = pSource;
}

string SourceURI::GetURI()
{
  return m_source;
}

string SourceURI::GetFullURI()
{
  string fullURI;
  fullURI = GetField(TYPE);
  fullURI += m_typeSeparator;
  fullURI += GetField(NAME);
  fullURI += m_formatSeparator;
  fullURI += GetField(FORMAT);
  return fullURI;
}

string SourceURI::str()
{
  return GetFullURI();
}

bool SourceURI::Empty()
{
  if (GetField(NAME).empty())
    return true;
  return false;
}

string SourceURI::GetField(SourceFields pFields)
{
  if (pFields >= NSOURCES) {
    log->say(ILog::ERROR, string("Invalid field requested for source: ")+m_source, "SourceURI");
    return string("");
  }
  size_t index_type;
  index_type = m_source.find(m_typeSeparator);
  if (index_type == string::npos) {
    if (pFields == TYPE)
      //Type is optional, use default
      return cfgMgr->GetDefaultDataType();
    //set to begin of the string when adding separator length
    index_type = -m_typeSeparator.size(); 
  }
  if (pFields == TYPE)
    return m_source.substr(0, index_type);
  //search for FORMAT separator, starting from the end of the string
  size_t index_format;
  index_format = m_source.rfind(m_formatSeparator);
  if (index_format == string::npos) {
    if (pFields == FORMAT) 
      //Format is optional, use default
      return cfgMgr->GetDefaultDataFormat();
    index_format = m_source.length(); //end of the string
  }
  if (pFields == FORMAT) {
    string format = m_source.substr(index_format+m_formatSeparator.size());
    if (format.empty()) //no format explicitly provided (string ending with m_formatSeparator)
      return cfgMgr->GetDefaultDataFormat();
    else 
      return format;
  }
  if (pFields != NAME) {
    log->say(ILog::FIXME, string("Internal inconsistency: requesting unknown field for data source ")+m_source, "SourceURI");
    return string("");    
  }
  if (m_source.empty()) {
    //no name provided
    log->say(ILog::WARNING, "Requested field NAME of empty source.");
    return string("");
  }
  return m_source.substr(index_type+m_typeSeparator.size(), 
			 index_format - (index_type+m_typeSeparator.size()));
    
}

void SourceURI::SetTypeSeparator(std::string pTypeSeparator)
{
  m_typeSeparator = pTypeSeparator;
}

std::string SourceURI::GetTypeSeparator()
{
  return m_typeSeparator;
}

void SourceURI::SetFormatSeparator(std::string pFormatSeparator)
{
  m_formatSeparator = pFormatSeparator;
}

std::string SourceURI::GetFormatSeparator()
{
  return m_formatSeparator;
}

// --- Implement friend operators

bool operator==(const SourceURI& x, const SourceURI& y)
{
  return (x.m_source == y.m_source);
}

bool operator==(const SourceURI& x, const string& y)
{
  return (x.m_source == y);
}

bool operator!=(const SourceURI& x, const SourceURI& y)
{
  return (x.m_source != y.m_source);
}

//string SourceURI::operator=(SourceURI& x)
//{
//  return x.GetSource();
//}
