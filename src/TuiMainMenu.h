/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiMainMenu.h
 Description: Text User Interface Main Menu
 Last Modified: $Id: TuiMainMenu.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __TUIMAINMENU__
#define __TUIMAINMENU__

#include "ITuiPage.h"

#include <vector>

/** Implements main menu page.
 */
class TuiMainMenu : public ITuiPage {
 public:
  ///Enum to comunicate to TuiSvc the menu selection
  enum TMSel {
    NEW_ACCOUNT, ///< Create a new account
    BROWSE, ///< Browse existing accounts
    SEARCH, ///< Advanced Search
    CONFIGURE, ///< Configure CSM settings
    QUIT, ///< Quit CSM
    NOT_VALID ///< init value, will return that in case of error
  };
  TMSel menuSelection;
 protected:
  /// Define each element of m_menuStr
  class TMenuStrContent {
  public:
    /** Init class and assign values.
     * @param pShortcut will assing value to shortcut.
     * @param pShortDescr will assign value to shortDescrCmd and shortDescrMenu (translating to upper case).
     * @param pLongDescr will assign value to longDescrMenu
     */
    TMenuStrContent(std::string pShortcut, std::string pShortDescr, std::string pLongDescr);
    /** Init class and assign values.
     * @copydoc TMenuStrContent(std::string, std::string, std::string)
     * @param pShortDescrMenu will assign value to shortDescrMenu
     */
    TMenuStrContent(std::string pShortcut, std::string pShortDescr, std::string pShortDescrMenu, std::string pLongDescr);
    ~TMenuStrContent();

    std::string shortcut; ///< shortcut for command bar
    std::string shortDescrCmd; ///< Short description for command bar
    std::string shortDescrMenu; ///< Short description for menu
    std::string longDescrMenu; ///< Long description for menu
  };
  /// Menu strings, initialized in constructor
  std::vector<TMenuStrContent> m_menuStr;
  /// ncurses menu sub-window for items
  Ncurses::WINDOW *m_itemWnd;
  /// ncurses menu object
  Ncurses::MENU *m_mainMenu;
  /// ncurses menu items
  Ncurses::ITEM **m_mainItems; 
  ///container for command bar
  std::vector< std::pair<std::string, std::string> > m_commands;  
 public:
  /** Init internal variables.
   * Initialize menu content. Zero pointers.
   */
  TuiMainMenu(std::string  pName);
  ~TuiMainMenu();

  /** @copydoc ITuiPage::Init() */
  virtual StatusCode Init(TuiStatusBar *pStatusBar);  

  /** @copydoc ITuiPage::Close() */
  virtual StatusCode Close();

  /** @copydoc ITuiPage::Kill() */
  virtual StatusCode Kill();

  /** @copydoc ITuiPage::Display() */
  virtual StatusCode Display();
  
};

#endif
