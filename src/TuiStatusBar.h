/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiStatusBar.h
 Description: Text User Interface status bar, command bar and header management
 Last Modified: $Id: TuiStatusBar.h 54 2013-03-20 21:54:57Z gridge $
*/

#ifndef __TUISTATUSBAR__
#define __TUISTATUSBAR__

#include <cstdlib>
#include <vector>
#include <string>

#include "IErrorHandler.h"
#include "ITuiPage.h"


/** Text User Interface status bar, command bar and header management.
 */
class TuiStatusBar : public IErrorHandler {
 protected:
  Ncurses::WINDOW *m_hwnd; ///< header window
  Ncurses::WINDOW *m_sbwnd; ///< status+command bar window
  int m_statusBarHeight; ///< Height of statusBar+command bar
  int m_headerHeight; ///< Height of header

  /// Status bar maximum lenght for each description
  int m_maxDescriptionSize;
  // Header string always displayed
  std::string m_headerFixedString;

  // Keep track of current command bar
  std::vector<std::pair<std::string, std::string> > m_commands;

 public:
  // ----------------------------------------
  // --- Constructor
  // ----------------------------------------  

  TuiStatusBar(std::string pName);
  ~TuiStatusBar();

  /** Init header, status and command bars.
   */
  void Init();

  // ----------------------------------------
  // --- Header
  // ----------------------------------------  

  /** Display header.
   * Draw header in m_hwnd and redraws.
   * @param pHeader optional additional message to the header
   * @param pRefresh if false omit to refresh window
   */
  void HeaderBar(std::string pHeader="", bool pRefresh=true);

  /// Return header height
  int GetHeaderHeight();

  // ----------------------------------------
  // --- StatusBar
  // ----------------------------------------    
  /** Defines types of inputs for StatusBar questions.
   *
   */
  enum TStatusBarInput {
    SBIN_STRING,
    SBIN_YN, ///< Yes/No question (with abbreviations accepted)
  };

  /** Display status bar.
   * Display header on m_sbwnd and redraws.
   * @param pStatus message to be displayed on the status bar
   * @param pRefresh if false omit to refresh window
   */
  void StatusBar(std::string pStatus="", bool pRefresh=true);

  /** Display a question in the status bar.
   * This function updates status bar and retrieve a string
   * from the user.
   * @param pStatus message to be displayed on the status bar
   * @param pAnswer returns the user input string (sligtly modified if the type requires)
   */
  StatusCode StatusBar(std::string pQuestion, std::string &pAnswer, TStatusBarInput pInputType = SBIN_STRING);

  // ----------------------------------------
  // --- Command Bar
  // ----------------------------------------  

  /** Display command bar.
   * Display command bar on m_sbwnd and redraws.
   * @param pCommands map of shortcut-description pairs of available commands
   * @param pRefresh if false omit to refresh window
   */
  void CommandBar(std::vector<std::pair<std::string, std::string> > pCommands, bool pRefresh=true);

  /// Draws an empty Command bar
  void CommandBar(bool pRefresh=true);

  /// Return status bar + command bar height
  int GetBottomBarHeight();

};

#endif
