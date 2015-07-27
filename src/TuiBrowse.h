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
  TuiAccount *m_editAccountPage;  
  
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
  /// Edit or view record in new screen
  bool m_editInNewPage;

  ///List of menu items as strings
  std::vector<std::string> m_loaListStr;

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
  void SetNLinesForBrowsing(int pNLines);

  //@TODO Allow filtering of items

};

#endif
