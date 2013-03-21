/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: ITuiPage.h
 Description: Text User Interface general page
 Last Modified: $Id: ITuiPage.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __ITUIPAGE__
#define __ITUIPAGE__

#include "IErrorHandler.h"

//forward declarations
class TuiStatusBar;

/// Include all curses-related headers
namespace Ncurses {
#include <ncurses.h>
#include <menu.h>
#include <form.h>

///useful macro for handling CTRL key -- stolen from dialog
#define CTRL(n)    ((n) & 0x1f)
}

/** Text User Interface general page.
 * Implements basic method of a TuiPage.
 * Allows to create/display pages and save their status.
 * It also helps in keeping modular all the TUI code.
 */
class ITuiPage : public IErrorHandler {
 protected:
  /// Reference to status/command bar and header.
  TuiStatusBar *m_statusBar;

  /// Main window used for this page
  Ncurses::WINDOW *m_wnd;

  //Size of displayable window to be set in the Init and be used by other functions if needed
  int m_wnd_lines; ///< number of lines of displayable window
  int m_wnd_cols; ///< number of columns of displayable window
  int m_wnd_y; ///< starting column of displayable window
  int m_wnd_x; ///< starting row of displayable window  
 public:
  /** Possible states of a page.
   * Right now there's no multi-thread support, so the state RUNNING will be visible only
   * from inside the "Display" function of the class.
   */
  enum PageStates {
    NONE, ///< Page has not been initialized yet (or has been cleared).
    CLEARED, ///< Page has been initialized but is not displayed.
    RUNNING, ///< Page is currently displayed.
  };
  /** Get page status.
   * See m_pageStatus for details.
   */
  PageStates Status();
 protected:
  /** Keep track of page status.
   * Instance is initialized to NONE,
   * Set to CLEARED when Init() is called.
   * Set to RUNNING when Display() is called.
   * Set to CLEARED again when Close() is called.
   * Set to NONE when Kill() is called
   */
  PageStates m_pageStatus;

  /// Set page status.
  void SetPageStatus(PageStates pStatus);

 public:
  ITuiPage(std::string pName);
  ~ITuiPage();

  /** Init elements needed for the page.
   * Initialize pointers (WINDOWS, MENU, ..) needed to be displayed on this page.
   * @param pStatusBar reference to a TuiStatusBar object for displaying status/shortcuts and header
   * @return status of initialization
   */
  virtual StatusCode Init(TuiStatusBar *pStatusBar);

  /** Close the page.
   * Basically clear the screen. Do not delete associated elements.
   * A new call to Display will show the same page again.
   */
  virtual StatusCode Close();

  /** Definitively kills the page. 
   * Clear the page and free all memory of internal objects.
   */
  virtual StatusCode Kill();

  /** Display the page
   * Preserves the state of its element from one call to another,
   * if not esplicitly initialized again.
   */
  virtual StatusCode Display();


};

#endif
