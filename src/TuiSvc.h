/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiSvc.h
 Description: Text User Interface Service
 Last Modified: $Id: TuiSvc.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __TUI_SERVICE__
#define __TUI_SERVICE__

#include "IErrorHandler.h"
#include "SourceURI.h"
#include "ITuiPage.h"
#include "TuiStatusBar.h"
#include "TuiMainMenu.h"
#include "TuiAccount.h"

#include <string>
#include <vector>

/** Text User Interface Service.
 * Implements a simple text-based user interface.
 * Uses ncurses, menu and panel libraries.
 */
class TuiSvc : public IErrorHandler {
 protected:
  // --- Store Tui status and properties
  Ncurses::WINDOW *m_mwnd; ///< main ncurses window

  // --- General properties
  int m_screenColumns; ///< Contains number of columns of the screen
  int m_screenRows; ///< Constains number of rows of the screen
  std::string m_welcomeMessage; ///< any welcome message to be displayed in the statusbar

  // --- Methods to operate on the Tui
  /// Init ncurses library
  StatusCode InitTui();
  /// Destroy Tui and get back to normal
  StatusCode CloseTui();

  // --- Tui pages
  TuiStatusBar *m_statusBar;  
  // -- Main Page
  /// Tui Main menu page
  TuiMainMenu *m_mainMenuPage;
  /** Display main menu.
   * This function exits when user decide to quit the TUI.
   */
  void MainMenu();

  // -- New account page
  TuiAccount *m_newAccountPage;
  /** Display new account page.
   * Call TuiNewAccount page for inserting a new account.
   * It also allows to select the source.
   */
  void NewAccount();

  // -- Browse page

  // -- Search page

 public:
  TuiSvc(std::string pName);
  ~TuiSvc();

  /// Run the user interface
  virtual StatusCode Run();

  // --- Accessors
  int GetScreenRows(); ///< return number of screen rows
  int GetScreenCols(); ///< return number of screen columns

  StatusCode SetWelcomeMessage(std::string msg);

};

#endif
