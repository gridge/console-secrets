/*
 Project: Console-Secrets
 Copyright (C) 2015  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiBrowse.h
 Description: Text User Interface browsing existing accounts
 Last Modified: $Id$
*/

#ifndef __TUIBROWSE__
#define __TUIBROWSE__

#include "ITuiPage.h"
#include "TuiAccount.h"


/** Text user interface for account browsing / editing. */
class TuiBrowse : public ITuiPage {
 protected:

  // ----------------------------------------
  // --- Internal members
  // ----------------------------------------  
  /// Flag to edit accounts on the same page or a new one
  TuiAccount *m_editAccountPage;

  /// Update current list of records 
  StatusCode UpdateListRecords(std::vector<ARecord*> &newList);

  /// Set command bar default navigation commands
  void SetCommandBarNavigation();

  /// Display record for viewing or editing
  void ViewRecord(ARecord *record);

  /// Remove record
  void RemoveRecord(ARecord *record);
  
  // ----------------------------------------
  // --- ncurses objects
  // ----------------------------------------  
  /// List Of Accounts (window)
  Ncurses::WINDOW *m_loaWnd;
  /// List Of Accounts (menu) 
  Ncurses::MENU *m_loaMenu;  
  /// List Of menu items
  Ncurses::ITEM **m_loaList;

  // ----------------------------------------
  // --- Display options
  // ----------------------------------------  
  /// Number of lines for the browsing window
  int m_nLinesForBrowsing; 
  /// Number of colimns for displaying each item
  int m_nColsPerItem;
  /// Edit or view record in new screen
  bool m_editInNewPage;

  ///List of currently displayed records
  std::vector<ARecord*> m_listOfRecords;

  ///Labels for menu display
  std::vector<std::pair<std::string,std::string> > m_loaLabels;

 public:

  // ----------------------------------------
  // --- Constructors
  // ----------------------------------------  
  TuiBrowse(std::string pName);

  ~TuiBrowse();


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
   * Display list of all the records (apply filter if requested).
   */
  virtual StatusCode Display();

  // ----------------------------------------  
  // --- Accessors
  // ----------------------------------------  

  /// Number of lines for the browsing window
  void SetNLinesForBrowsing(int pNLines);

  /// Number of colimns for displaying each item
  void SetNColumnsPerItem(int pNCols);

  //@TODO Allow filtering of items

};

#endif
