/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiAccount.h
 Description: Text User Interface editing account page
 Last Modified: $Id: TuiAccount.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __TUIACCOUNT__
#define __TUIACCOUNT__

#include "ITuiPage.h"
#include "SourceURI.h"
#include "ARecord.h"
#include "IConfigurationService.h"

/**  Text User Interface editing account page.
 * It also allows selection of destination source and account type.
 */
class TuiAccount : public ITuiPage {
 protected:

  // ----------------------------------------
  // --- Internal members
  // ----------------------------------------  

  /// Store source which the account refers to (optional, used for adding new account)
  SourceURI m_source;

  /// Store the managed ARecord class containing the actual account info
  ARecord *m_record;

  /// Command-bar 
  std::vector< std::pair< std::string, std::string > > m_commands;
  
  // ----------------------------------------
  // --- ncurses objects
  // ----------------------------------------  
  /// subwindow for fields (mother window: ITuiPage::m_wnd
  Ncurses::WINDOW *m_accountFieldWnd;
  /// Fields associated to the account
  Ncurses::FIELD **m_accountFields;
  /// Account form containing fields
  Ncurses::FORM *m_accountForm;

  // ----------------------------------------
  // --- View setting
  // ----------------------------------------  

  /// Lock displayed fields, for viewing and not editing if true
  bool m_fieldsLocked;
  /// Defines a row offset for the display of the page
  int m_rowOffset;
  /// Defines a column offset for the display of the page
  int m_colOffset;
  ///Defines maximum field lines displayable before scrolling in multi-line fields --- not used now
  int m_maxFieldDisplayHeight;
  /// Stores the number of fields used for account properties
  int m_numberFieldsAccProp;
  // Stores order of special fields.
  int fNameIdx; /// index among fields of special AccountName field
  int fLabelsIdx; /// index among fields of special AccountName field
  int fEssentialsIdx; /// index among fields of special AccountName field
  // -- the following settings are inherited from IConfigurationManager
  /// Defines field name width for displaying
  int m_cfgFieldNameWidth;


 public:

  // ----------------------------------------
  // --- Constructors
  // ----------------------------------------  

  TuiAccount(std::string pName);

  ~TuiAccount();

  // ----------------------------------------  
  // --- Set Options
  // ----------------------------------------  

  // - Manipulate record associated.
  // Note that memory is left to be managed externally, since we may want to just edit an existing record

  /// Get a pointer to managed record.
  ARecord *GetRecord();
  /// Free memory associated with managed record.
  StatusCode ClearRecord();  
  /// Allocate memory for a new record.
  StatusCode NewRecord();
  /// Set pointer of managed record to an external record.
  StatusCode SetRecord(ARecord* pRecord);
  /* Add fields to managed record.
   * Add to existing m_record (if null, create a new one) fields from pRecordType.
   * If field exists, do not overwirte it, unlsess force=true
   */
  StatusCode AddRecordFields(ARecord pRecordType, bool force=false);

  // - Manipulate internal source
  SourceURI GetSource();

  // - View options

  /** Lock fields from editing.
   * To be called before invoking TuiAccount::Display() (default).
   * It avoid fields to be modified,
   * It does not display anything on the (header, command) status bar. 
   * It's a non-blocking call (process only charachter already in the buffer). 
   * User then needs to call Close() to close the page.
   */
  virtual StatusCode LockFields();
  /** Unlock fields for editing.
   * To be called before invoking TuiAccount::Display(), it allows fields to be modified
   */
  virtual StatusCode UnlockFields();

  /** Set an offset for window displaying.
   * Set a row/column offset for the upper-left corner of the window displaying the account information.
   * Used when browsing to restrict to a subset of the screen.
   */
  void SetWndOffset(int pColumns, int pRows);

  // ----------------------------------------  
  // --- ITuiPage methods
  // ----------------------------------------  

  /** @copydoc ITuiPage::Init() */
  virtual StatusCode Init(TuiStatusBar *pStatusBar);  

  /** @copydoc ITuiPage::Close() */
  virtual StatusCode Close();

  /** @copydoc ITuiPage::Kill() */
  virtual StatusCode Kill();

  /** @copydoc ITuiPage::Display() 
   * Works on data of the internal managed m_record and display account edit page.
   */
  virtual StatusCode Display();

  /** Create a new account.
   * Ask for source destination, predefined accout type. Then create a new m_record and invoke the account editor.
   */
  StatusCode NewAccount();


  // ----------------------------------------  
  // --- Helper functions
  // ----------------------------------------  

  /** Select source.
   * Display and select an active source or create a new one.
   * @param mainWnd parent window to which this selection menu belongs
   * @return selected (or newly created) source
   */
  SourceURI SelectSource();

  /** Select among predefined account types.
   * Allow user to start from one of the predefined account types, or from an empty one.
   * @param emptyShow show field for empty set of fields
   */
  ARecord SelectPredefAccType(bool emptyShow=true);

  /** Calculate size allowed for field title. 
   * Determine field title displayable size: space for all the title, up to COLS/2, and at least m_cfgFieldNameWidth
   */
  int GetFieldTitleSize(std::string pTitle);
  
  /** Print to m_wnd the fields titles.
   * Loop over fields and retrieve titles from user pointer. Determine max width and print to screen.
   * This function does not refresh the screen.
   */
  void PrintFieldTitles();

  /** Select among predefined list.
   * General simple selection of one item out of the input list.
   * @param pTitle title to be displayed
   * @param pColumns list of choices
   * @return index of choice inside pChoices, or < 0 if an error occurred.
   */
  int SelectOne(std::string pTitle, std::vector<std::string> pChoices);

  /** Creates form window.
   * Used everytime a field is added or removed.
   */
  void CreateFields();

  /** Updates m_record from field content and frees associated memory. */
  void UpdateAndFreeForm();

};

#endif
