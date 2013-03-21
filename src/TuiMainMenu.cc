/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiMainMenu.h
 Description: Text User Interface Main Menu
 Last Modified: $Id: TuiMainMenu.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "TuiMainMenu.h"
#include "TuiStatusBar.h"
#include "ILog.h"
#include "IConfigurationService.h"

#include <algorithm>

extern ILog *log;
extern IConfigurationService *cfgMgr;

using namespace Ncurses;
using namespace std;

TuiMainMenu::TMenuStrContent::TMenuStrContent(string pShortcut, string pShortDescr, string pLongDescr) :
  shortcut(pShortcut),
  shortDescrCmd(pShortDescr),
  longDescrMenu(pLongDescr)
{
  //translate pShortDescr to upper-case and store to shortDescrMenu
  shortDescrMenu.resize(pShortDescr.size()); //allocate space
  transform(pShortDescr.begin(), pShortDescr.end(), shortDescrMenu.begin(), ::toupper);
}

TuiMainMenu::TMenuStrContent::TMenuStrContent(string pShortcut, string pShortDescr, string pShortDescrMenu, string pLongDescr) :
  shortcut(pShortcut),
  shortDescrCmd(pShortDescr),
  shortDescrMenu(pShortDescrMenu),
  longDescrMenu(pLongDescr)
{

}

TuiMainMenu::TMenuStrContent::~TMenuStrContent() 
{
}

TuiMainMenu::TuiMainMenu(string  pName) : ITuiPage(pName)
{
  m_mainMenu = 0;
  m_mainItems = 0;
  m_itemWnd = 0;
  menuSelection = NOT_VALID;
  // init menu values
  m_menuStr.push_back(TMenuStrContent("N", "New Account", "Store information for a new account"));
  m_menuStr.push_back(TMenuStrContent("B", "Browse", "Browse existing accounts"));
  m_menuStr.push_back(TMenuStrContent("S", "Search", "Advanced search"));
  m_menuStr.push_back(TMenuStrContent("C", "Configure", "Change Console-Secrets (CSM) settings"));
  m_menuStr.push_back(TMenuStrContent("Q", "Quit", "Quit Console-Secrets (CSM)"));
}

TuiMainMenu::~TuiMainMenu()
{

}

IErrorHandler::StatusCode TuiMainMenu::Init(TuiStatusBar *pStatusBar)
{
  //call base class init method
  ITuiPage::Init(pStatusBar);
  //ncurses objects
  m_mainItems = new ITEM*[m_menuStr.size()+1]; //we now have 5 choices: New, Browse, Search, Configure, Quit
  //create main menu
  int idx=0;
  for (vector<TMenuStrContent>::iterator itm = m_menuStr.begin(); itm != m_menuStr.end(); ++itm) {
    m_mainItems[idx] = new_item(itm->shortDescrMenu.c_str(), itm->longDescrMenu.c_str());
    m_commands.push_back(make_pair(itm->shortcut.c_str(), itm->shortDescrCmd.c_str()));
    idx++;
  }
  m_mainItems[idx] = new_item(NULL, NULL);
  m_mainMenu = new_menu((ITEM**)m_mainItems);
  //create menu window
  int menu_posx =  COLS/2 - 25;
  if (menu_posx < 0)
    menu_posx = 0;
  //we need a main window, but don't need borders or other text. Make it as big as the item sub-window  
  m_wnd = newwin(LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight() + 2), 60,
		 m_statusBar->GetHeaderHeight()+2, menu_posx);
  m_itemWnd = subwin(m_wnd, LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight() + 2), 60,
		     m_statusBar->GetHeaderHeight()+2, menu_posx);
  keypad(m_wnd, TRUE);
  set_menu_spacing(m_mainMenu, 3, 2, 0);
  //  set_menu_mark(m_mainMenu, NULL);
  int err = set_menu_win(m_mainMenu, m_wnd);
  if (err != 0) {
    *log << ILog::ERROR << "NCurses error in associating menu to window:" << err << this << ILog::endmsg;
    return m_statusCode;
  }  
  err = set_menu_sub(m_mainMenu, m_itemWnd);
  if (err != 0) {
    *log << ILog::ERROR << "NCurses error in associating menu to window:" << err << this << ILog::endmsg;
    return m_statusCode;
  }  

  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode TuiMainMenu::Close()
{
  unpost_menu(m_mainMenu);
  return ITuiPage::Close();
}

IErrorHandler::StatusCode TuiMainMenu::Kill()
{
  free_menu(m_mainMenu);
  for (size_t it=0; it<m_menuStr.size()+1; it++)
    free_item(m_mainItems[it]);
  delete[] m_mainItems;
  if (m_itemWnd)
    delwin(m_itemWnd);
  m_mainMenu = 0;
  m_mainItems = 0;
  m_itemWnd = 0;
  return ITuiPage::Kill(); //this will free menu main window m_wnd too
}

IErrorHandler::StatusCode TuiMainMenu::Display()
{
  if (Status() != RUNNING) {
    //need to refresh few things
    ITuiPage::Display();
    //display header, command bar. Do not clean status-bar now (keep last message)
    m_statusBar->HeaderBar(cfgMgr->GetTopMessage());
    m_statusBar->CommandBar(m_commands);
    //  m_statusBar->StatusBar();
    //draw the main menu
    post_menu(m_mainMenu);
    //refresh screen
    wrefresh(m_wnd);  
  }

  //enter the main loop
  menuSelection = NOT_VALID;
  while (menuSelection == NOT_VALID) {
    int c = getch();
    m_statusBar->StatusBar(); //clear status bar from previous messages. half_delay set to appropriate value.
    switch (c) {
    case KEY_DOWN:
      menu_driver(m_mainMenu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(m_mainMenu, REQ_UP_ITEM);
      break;
    case KEY_ENTER: //Enter
    case 10: //Enter
    case 13: //Enter
      {
	//ok, find out what's the requested action and act accordingly
	ITEM *selItem = current_item(m_mainMenu);
	switch (item_index(selItem)) {
	case 0: 
	  //create new account
	  menuSelection = NEW_ACCOUNT;
	  break;
	case 1:
	  //browse accounts
	  menuSelection = BROWSE;
	  break;
	case 2:
	  //search
	  menuSelection = SEARCH;
	  break;
	case 3:
	  //Configure
	  menuSelection = CONFIGURE;
	  break;
	case 4:
	  //quit
	  menuSelection = QUIT;
	  break;
	default:
	  //what? none of above... don;t quit!
	  *log << ILog::FIXME << "Invalid selection from Main Menu: " << item_index(selItem) << this << ILog::endmsg;
	}
	break;
      }
      //now define shortcuts -- very inelegant sorry 
    case 'n':
    case 'N':
      //create new account
      set_current_item(m_mainMenu, m_mainItems[0]);
      menuSelection = NEW_ACCOUNT;
      break;
    case 'b':
    case 'B':
      //Browse
      set_current_item(m_mainMenu, m_mainItems[1]);
      menuSelection = BROWSE;
      break;
    case 's':
    case 'S':
      //Search
      set_current_item(m_mainMenu, m_mainItems[2]);
      menuSelection = SEARCH;
      break;
    case 'c':
    case 'C':
      //Configure
      set_current_item(m_mainMenu, m_mainItems[3]);
      menuSelection = CONFIGURE;
      break;
    case 'q':
    case 'Q':
      //quit
      set_current_item(m_mainMenu, m_mainItems[4]);
      menuSelection = QUIT;
      break;
    }
    wrefresh(m_wnd);
    // catch errors
    if (m_statusCode >= SC_FATAL) {
      *log << ILog::FATAL << "ERROR condition detected. Closing Tui."
	   << this << ILog::endmsg;
      return m_statusCode;
    }
  }

  return m_statusCode = SC_OK;
 
}
