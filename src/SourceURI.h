/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: SourceURI.h
 Description: Implement format of data source
 Last Modified: $Id: SourceURI.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __SOURCEURI__
#define __SOURCEURI__

#include <string>

/* Class to set the format of data source identifiers. */
class SourceURI {
 protected:
  /** Contains the source location.
   * m_source format is: [type://]sourceName[.format]
   * where
   *  - type(optional): is used to decide which Storage Tool to use for reading/writing the source.
   *  - sourceName: Identify the source name.
   *  - format(optional): is used to determine the Formatter Tool to be used.
   * Defaults (set in IConfigurationService are:
   *  - type: file [local file]
   *  - name: accounts
   *  - format: czx [crypted zipped xml]
   */
  std::string m_source;

  //define separators
  std::string m_typeSeparator; ///< Define separator delimiting TYPE and NAME fields
  std::string m_formatSeparator; ///< Define separator delimiting NAME and FORMAT fields

 protected:
  /// Set default values for m_typeSeparator and m_formatSeparator;
  void SetDefaultSeparators();

 public:
  /// Define the fileds of m_source
  enum SourceFields {
    TYPE,   // Type of source
    NAME,   // Source location
    FORMAT, // Source format
    NSOURCES // number of fields for this enum
  };
 public:
  /// Default constructor
  SourceURI();
  /// Initialize using string pSource, formatted accordingly.
  SourceURI(std::string pSource);
  /// Initi using separate field (concatenate using appropriate separators)
  SourceURI(std::string pName, std::string pType, std::string pFormat="");
  /// Destructor
  ~SourceURI();

  // --- Accessors
  ///@todo check with regexp if pSource has valid characters
  virtual void SetURI(std::string pSource); ///< see m_source
  virtual std::string GetURI(); ///< see m_source
  virtual std::string GetFullURI(); ///< Get always full URI with all fields
  virtual std::string str(); ///< alias for GetSource()

  virtual bool Empty(); ///< check if source is empty

  // --- Helper functions
  /// Helper function to split m_source format.  
  virtual std::string GetField(SourceFields pFields);

  // --- Accessors
  /// Customize TYPE separator, know what you're doing..
  void SetTypeSeparator(std::string pTypeSeparator);
  std::string GetTypeSeparator();
  void SetFormatSeparator(std::string pFormatSeparator);
  std::string GetFormatSeparator();

  ///return GetSource()
  //std::string operator=(SourceURI& x);

  //Friend operators
  /// Check if two sources are the same. Compare m_source.
  friend bool operator==(const SourceURI& x, const SourceURI& y);
  friend bool operator==(const SourceURI& x, const std::string& y);
  friend bool operator!=(const SourceURI& x, const SourceURI& y);

};

#endif
