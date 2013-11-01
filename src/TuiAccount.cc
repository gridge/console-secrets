/*
 Project: Console-Secrets
 Copyright (C) 2013  Simone Pagan Griso
 Distributed WITHOUT ANY WARRANTY under the GPL-3.0 licence,
 see the LICENCE file or visit <http://www.gnu.org/licenses/>
 File: TuiAccount.cc
 Description: Text User Interface New Account page
 Last Modified: $Id: TuiAccount.cc 54 2013-03-20 21:54:57Z gridge $
*/

#include "TuiAccount.h"
#include "MiscUtils.h"
#include "IErrorHandler.h"
#include "ILog.h"
#include "IConfigurationService.h"
#include "IIOService.h"
#include "MultipleSourceIOSvc.h"
#include "TuiSvc.h"

extern ILog *log;
extern IConfigurationService *cfgMgr;
extern IIOService *ioSvc;
extern TuiSvc *tuiSvc;

#include <algorithm>
#include <string.h> // need to use strcpy for storing userptr of ncurses

using namespace Ncurses;
using namespace std;
using namespace CSMUtils;

TuiAccount::TuiAccount(std::string pName) : ITuiPage(pName)
{
  m_record = 0;
  m_fieldsLocked = true;
  m_rowOffset = 0;
  m_colOffset = 0;
  m_accountFieldWnd = 0;
  m_accountForm = 0;
  m_accountFields = 0;
  m_cfgFieldNameWidth = cfgMgr->GetAccountFieldNameSize();
  m_numberFieldsAccProp = 3; //will be set by Display function, default is 3: Name,Labels,Essentials
  m_maxFieldDisplayHeight = 10; // not more than 10 rows, then becomes scrollable
  fNameIdx = 0; // index of special AccountName field
  fLabelsIdx = 1; // index of special AccountName field
  fEssentialsIdx = 2; // index of special AccountName field

}

TuiAccount::~TuiAccount()
{

}

ARecord *TuiAccount::GetRecord()
{
  return m_record;
}

IErrorHandler::StatusCode TuiAccount::ClearRecord()
{
  if (m_record)
    delete m_record;
  m_record = 0;
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode TuiAccount::NewRecord()
{
  m_record = new ARecord();
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode TuiAccount::SetRecord(ARecord* pRecord)
{
  m_record = pRecord;
  return m_statusCode = SC_OK;
}

IErrorHandler::StatusCode TuiAccount::AddRecordFields(ARecord pRecordType, bool force)
{
  if (!m_record)
    m_record = new ARecord();
  // add fields
  for (ARecord::TFieldsIterator itf = pRecordType.GetFieldsIterBegin(); itf != pRecordType.GetFieldsIterEnd(); ++itf) {
    //check if field exists yet, if so, do not add it (unless force=true)
    if (force or not m_record->HasField(itf->first))
      m_record->AddField(itf->first, itf->second); //add field (itf->second could contain some pre-defined values for the field)
  }
  // add essentials
  for (ARecord::TEssentialsIterator ite = pRecordType.GetEssentialsIterBegin(); ite != pRecordType.GetEssentialsIterEnd(); ++ite)
    m_record->AddEssential(*ite);
  return m_statusCode = SC_OK;
}

SourceURI TuiAccount::GetSource()
{
  return m_source;
}

void TuiAccount::SetWndOffset(int pColumns, int pRows)
{
  m_rowOffset = pRows;
  m_colOffset = pColumns;
}

IErrorHandler::StatusCode TuiAccount::Init(TuiStatusBar *pStatusBar)
{
  m_statusCode = SC_OK;
  ITuiPage::Init(pStatusBar);
  // --- define window for new account page
  if (!m_statusBar) {
    *log << ILog::FIXME << "A valid status bar is needed to initialize TuiAccount page!"
	 << this << ILog::endmsg;
    return m_statusCode;
  }

  // main window
  m_wnd_lines = LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight() + 2) - m_rowOffset;
  m_wnd_cols = COLS - m_colOffset;
  m_wnd_y = m_statusBar->GetHeaderHeight()+1+m_rowOffset;
  m_wnd_x = m_colOffset; 
  m_wnd = newwin(m_wnd_lines, m_wnd_cols, m_wnd_y, m_wnd_x);
  keypad(m_wnd, TRUE);
  // field window (whole main window)
  m_accountFieldWnd = subwin(m_wnd, m_wnd_lines, m_wnd_cols, m_wnd_y, m_wnd_x+1); //leave one column for space/border
  *log << ILog::DEBUG << "Created field window: " 
       << "H:" << m_wnd_lines << " W:" << m_wnd_cols << " Y:" << m_wnd_y << " X:" << m_wnd_x << this << ILog::endmsg;

  // command bar
  m_commands.clear();
  m_commands.push_back(make_pair<string, string>("^X","Save account"));
  m_commands.push_back(make_pair<string, string>("^C","Cancel editing"));
  m_commands.push_back(make_pair<string, string>("^N", "New field"));
  m_commands.push_back(make_pair<string, string>("^R", "Remove field"));
  m_commands.push_back(make_pair<string, string>("^F", "Quick-add fields"));
  //  m_commands.push_back(make_pair<string, string>("^K", "Cut line"));
  //  m_commands.push_back(make_pair<string, string>("^U", "Un-cut line"));

  return m_statusCode;
}

IErrorHandler::StatusCode TuiAccount::Close()
{
  return ITuiPage::Close();
}

IErrorHandler::StatusCode TuiAccount::Kill()
{
  *log << ILog::DEBUG << "Destroying window" << this << ILog::endmsg;
  if (m_accountFieldWnd) {
    wclear(m_accountFieldWnd);
    delwin(m_accountFieldWnd);
  }
  m_accountFieldWnd = 0;

  return ITuiPage::Kill();  
}

IErrorHandler::StatusCode TuiAccount::Display()
{  
  m_statusCode = SC_OK;
  
  // --- clear screen, set status to running
  ITuiPage::Display(); 
  if (!m_fieldsLocked) {
    m_statusBar->HeaderBar("ACCOUNT DETAILS");    
    m_statusBar->StatusBar(); // clear status bar
    m_statusBar->CommandBar(m_commands);
  }

  if (!m_record) {
    // it should never happen
    *log << ILog::FIXME << "Invalid record opened for editing/viewing." << this << ILog::endmsg;
    return m_statusCode = SC_ERROR;
  }

  // --- hanldes input
  bool quitAccount=false;
  int fieldSelected = -1; //keep track of selected field to trigger actions when it changes
  CreateFields(); //draw form
  if (m_statusCode >= SC_ERROR) 
    return m_statusCode;  
  string cutPasteBuffer; //keep buffer for cut&paste

  while (!quitAccount) {

    //refresh titles
    PrintFieldTitles();
    //refresh window
    wrefresh(m_wnd);
    int ch = getch();

    //clear status bar from previous messages
    m_statusBar->StatusBar();

    // check input charachter
    switch (ch) {

      // Moving actions
    case KEY_DOWN:
      form_driver(m_accountForm, REQ_NEXT_FIELD);
      break;
    case KEY_UP:
      form_driver(m_accountForm, REQ_PREV_FIELD);
      break;
    case KEY_RIGHT:
      //todo: check end of string
      form_driver(m_accountForm, REQ_NEXT_CHAR);
      break;
    case KEY_LEFT:
      form_driver(m_accountForm, REQ_PREV_CHAR);
      break;
    case KEY_EOL:      
    case CTRL('e'):
      form_driver(m_accountForm, REQ_END_FIELD);
      break;
    case CTRL('a'):
      form_driver(m_accountForm, REQ_BEG_FIELD);
      break;

      // Editing actions
    case KEY_BACKSPACE:
    case CTRL('?'):
    case CTRL('h'):
    case 127: //XFree 4 style
      form_driver(m_accountForm, REQ_DEL_PREV);      
      break;
    case CTRL('d'):
    case KEY_DC:
      form_driver(m_accountForm, REQ_DEL_CHAR);      
      break;
    case CTRL('k'):
      //TODO: implement cut into a buffer and the paste back (does not work in the middle of words!)
      cutPasteBuffer=TrimStr( static_cast<char*>( field_buffer (m_accountFields[fieldSelected], 0) ) );
      *log << ILog::DEBUG << "Cutting: " << cutPasteBuffer << ILog::endmsg;
      form_driver(m_accountForm, REQ_CLR_EOF);      
      break;
    case CTRL('y'):
      //paste content
      *log << ILog::DEBUG << "Pasting: " << cutPasteBuffer << ILog::endmsg;
      for (string::iterator itc = cutPasteBuffer.begin(); itc != cutPasteBuffer.end(); ++itc)
	form_driver(m_accountForm, *itc);
    case KEY_ENTER:
    case 10:
    case 13:
      //todo: make multi-line fields? in that case need to check also after one charachter is entered if we need to grow the field
      //      up to a maximum (then becomes scrollable)
      form_driver(m_accountForm, REQ_NEXT_FIELD);
      break;      

      // Special actions
    case CTRL('r'):
      {
	//remove field
	// if not field selected, exit
	int selF = field_index(current_field(m_accountForm));
	if (selF == ERR) 
	  break;
	//ask confirmation
	string really;      
	m_statusBar->StatusBar("Delete this field (y/n)", really, TuiStatusBar::SBIN_YN);
	if (! (really == "Y")) //converted upper-case from option 
	  break; //abort
	//syncronize forms<->record
	UpdateAndFreeForm();
	if (m_statusCode >= SC_ERROR)
	  return m_statusCode;      
	//find corresponding field in m_record to delete
	ARecord::TFieldsIterator recordF = m_record->GetFieldsIterBegin();
	//need to loop only to < curF since we're incerementing every time from the beginning
	for (int curF = 0; curF < selF; curF++)
	  if (curF != fNameIdx && curF != fLabelsIdx && curF != fEssentialsIdx)
	    ++recordF;
	//now remove fields
	m_record->EraseField(recordF);
	//re-make form
	CreateFields();
	if (m_statusCode >= SC_ERROR)
	  return m_statusCode;      
	break;
      }
    case CTRL('n'):
      {
	//create new field
	//syncronize forms<->record
	UpdateAndFreeForm();
	if (m_statusCode >= SC_ERROR)
	  return m_statusCode;      
	//ask the name of the new field
	string newFieldName;
	m_statusCode = m_statusBar->StatusBar("Name of the new field", newFieldName); 
	if ( ( ! ( m_statusCode >= SC_ERROR ) ) && ( ! ( newFieldName.empty() ) ) ) {
	  //now create the new field in m_record
	  std::string emptyStr;
	  m_record->AddField(newFieldName, emptyStr); //add empty field to the bottom
	}
	//re-make form
	CreateFields();
	if (m_statusCode >= SC_ERROR) 
	  return m_statusCode;  
	break;
      }
    case CTRL('f'):
      {
	// Add set of pre-defined fields
        ARecord accountType = SelectPredefAccType(false);// not empty record
	if (m_statusCode >= SC_ERROR)
	  break;
	UpdateAndFreeForm();
	if (m_statusCode >= SC_ERROR) 
	  return m_statusCode;  
	AddRecordFields(accountType);	
	if (m_statusCode >= SC_ERROR) 
	  return m_statusCode;  
	CreateFields();
	if (m_statusCode >= SC_ERROR) 
	  return m_statusCode;         
	break;
      }
    case CTRL('l'):
      // call label or essential selection, if applicable
      if (fieldSelected == fLabelsIdx) {
	vector<string> labelList = ioSvc->GetLabels(); //get unique and sorted label list
	if (labelList.size() == 0) {
	  *log << ILog::DEBUG << "No labels available for selection" << this << ILog::endmsg;
	  m_statusBar->StatusBar("No labels defined yet. Just type your preferred one.");
	  break;
	}	
	int labToAdd = SelectOne("Select label", labelList);
	if (labToAdd >= 0) {
	  //move to the end of the field
	  form_driver(m_accountForm, REQ_END_FIELD);	  
	  //add all charachters
	  if (string(field_buffer(m_accountFields[fLabelsIdx],0)).substr(0,5) != "     ") // ugly: if field is not blank
	    form_driver(m_accountForm, ','); //use comma to separate fields
	  string labToAddStr = labelList.at(labToAdd);
	  for (string::iterator itc = labToAddStr.begin(); itc != labToAddStr.end(); ++itc)
	    form_driver(m_accountForm, *itc);
	}	
      } else if (fieldSelected == fEssentialsIdx) {
	if (m_record->GetNumberOfFields() == 0) {
	  *log << ILog::DEBUG << "No fields available for selection" << this << ILog::endmsg;
	  m_statusBar->StatusBar("No fields available to make essentials. Create first a new field.");
	  break;
	}
	// Ask for user using SelectOne() function
	int fieldToAdd;
	fieldToAdd = SelectOne("Select field", m_record->GetFieldNameList()); 	
	//- got it, add the answer to the field emulating user input (useful for dynamic size of fields)
	if (fieldToAdd >= 0) {
	  //move to the end of the field
	  form_driver(m_accountForm, REQ_END_FIELD);	  
	  //add all charachters
	  if (string(field_buffer(m_accountFields[fEssentialsIdx],0)).substr(0,5) != "     ") // ugly: if field is not blank
	    form_driver(m_accountForm, ','); //use comma to separate fields
	  string fieldToAddStr = m_record->GetFieldNameList().at(fieldToAdd);
	  for (string::iterator itc = fieldToAddStr.begin(); itc != fieldToAddStr.end(); ++itc)
	    form_driver(m_accountForm, *itc);
	}
      }
      //force refresh of command bar
      fieldSelected = -1;
      break;

      // Quit account editing
    case CTRL('c'):
      // TODO: ask for confirmation with statusbar
      *log << ILog::INFO << "Account editing cancelled." << this << ILog::endmsg;
      m_statusCode = SC_ABORT;
      quitAccount = true;
      break;
    case CTRL('x'):
      // TODO: ask for confirmation with statusbar
      m_statusCode = SC_OK;
      quitAccount = true;
      break;

      // Default: pass to driver
    default:
      form_driver(m_accountForm, ch);
      break;

    } // end switch of input char

    // handle special actions at field change
    int newFieldSelected = field_index(current_field(m_accountForm));
    if (newFieldSelected != fieldSelected) {
      //actions when exiting a field
      if (fieldSelected == fEssentialsIdx) {
	//validate input
	if (field_status(m_accountFields[fieldSelected]) != 0) {
	  string insEss( TrimStr( static_cast<char*>( field_buffer (m_accountFields[fieldSelected], 0) ) ) );
	  m_record->ClearEssentials();
	  //todo: make list unique!
	  m_record->AddEssentials( SplitListStr( insEss ) );		
	  set_field_buffer( m_accountFields[fieldSelected], 0, CreateListStr(m_record->GetEssentials()).c_str() );
	}
      } else if (fieldSelected == fLabelsIdx) {
	//re-format input
	string insLab( TrimStr( static_cast<char*>( field_buffer (m_accountFields[fieldSelected], 0) ) ) );
	m_record->ClearLabels();
	m_record->AddLabels( SplitListStr( insLab ) );		
	set_field_buffer( m_accountFields[fieldSelected], 0, CreateListStr(m_record->GetLabels()).c_str() );
      }
      //actions when entering a field
      if (newFieldSelected == fLabelsIdx) {
	//Field "Labels" selected
	//add special command
	vector<pair<string, string> >newCommands = m_commands;
	newCommands.push_back(make_pair<string, string>("^L", "List labels"));
	m_statusBar->CommandBar(newCommands);
      } else if (newFieldSelected == fEssentialsIdx) {
	//Field "Essentials" selected
	//add special command
	vector<pair<string, string> >newCommands = m_commands;
	newCommands.push_back(make_pair<string, string>("^L", "List fields"));
	m_statusBar->CommandBar(newCommands);
      } else {
	//restore (even if not always necessary) usual command bar
	m_statusBar->CommandBar(m_commands);
      }
      fieldSelected = newFieldSelected; //store new selected field
    }
    
  } // end loop for input key

  // --- check if fields have changed, and update m_record accordingly
  UpdateAndFreeForm(); //updated m_record and frees memory (if a form is available)    

  // --- all done

  curs_set(0); //restore cursor

  return m_statusCode;
}

IErrorHandler::StatusCode TuiAccount::LockFields()
{
  m_fieldsLocked = true;
  return SC_OK;
}

IErrorHandler::StatusCode TuiAccount::UnlockFields()
{
  m_fieldsLocked = false;
  return SC_OK;
}

IErrorHandler::StatusCode TuiAccount::NewAccount()
{
  ITuiPage::Display();  
  m_statusBar->HeaderBar("NEW ACCOUNT");
  m_statusBar->StatusBar(); // clear status bar
  m_statusBar->CommandBar(); // clear command bar

  // --- select the source where to store your account
  SelectSource();
  if (m_statusCode >= SC_ERROR) {
    //this is also the case for user abort (SC_ABORT)
    m_statusBar->StatusBar("New account creation cancelled.");
    *log << ILog::ERROR << m_errorMsg << this << ILog::endmsg;
    Kill();
    return m_statusCode = SC_ABORT;
  }
  *log << ILog::VERBOSE << "Selected source:" << m_source.str() << this << ILog::endmsg;

  // --- select the pre-defined account-type  
  ARecord accountType = SelectPredefAccType();
  if (m_statusCode >= SC_ERROR) {
    // user abort
    *log << ILog::INFO << "User aborted new account creation." << this << ILog::endmsg;
    m_statusBar->StatusBar("New account creation cancelled.");
    Kill();
    return m_statusCode = SC_ABORT;
  }
  if (accountType.GetAccountName().empty())
    *log << ILog::VERBOSE << "Selected predefined account type: EMPTY" << this << ILog::endmsg;    
  else
    *log << ILog::VERBOSE << "Selected predefined account type: " << accountType.GetAccountName() << this << ILog::endmsg;

  // --- create new account m_record and add predefined fields (if any)
  NewRecord();
  AddRecordFields(accountType,true); //force over-writing of fields (should not be needed)
  if (! (accountType.GetAccountName().empty() || accountType.GetAccountName() == "EMPTY"))
    m_record->SetAccountName(accountType.GetAccountName()); //default account name

  // --- now invoke account editor
  // if NewAccount-mode we want the full screen for us, if it's not force it!
  if ((m_rowOffset != 0) || (m_colOffset != 0)) {
    Kill();
    if (m_statusCode >= SC_ERROR) 
      return m_statusCode;
    m_rowOffset = 0;
    m_colOffset = 0;
    Init(0); //preserves existing m_statusBar
    if (m_statusCode >= SC_ERROR)
      return m_statusCode;
  }
  //unlock fields before invoking editor
  UnlockFields();
  //finally call the editor page
  return Display();

}

SourceURI TuiAccount::SelectSource()
{
  m_statusCode = SC_OK;
  //Get size of the window
  int maxCOLS=COLS;
  int maxLINES=LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight());
  //Are we working in a multi-source mode?
  MultipleSourceIOSvc *ioSvcM = dynamic_cast<MultipleSourceIOSvc*>(ioSvc);
  if (!ioSvcM) {
    *log << ILog::WARNING << "Cannot find object handling multi-source."
	 << "Using single-source mode: " << ioSvc->GetSource().str() 
	 << this << ILog::endmsg;
    return ioSvc->GetSource().str();
  }
  //get existing sources
  vector<SourceURI> allSources = ioSvcM->GetManagedSources();
  // disable the following when new source creation possible
  //----
  //if (allSources.size() == 0) {
  //  m_errorMsg = "No source available for storing account.";
  //  m_statusCode = SC_ERROR;
  //  return SourceURI("");
  //}
  //if (allSources.size() == 1)
  //  return allSources[0]; //no choice..
  //----
  SourceURI itemNewSource("Create new", "=>", "<="); //add special item for creating a new source
  allSources.push_back(itemNewSource);

  //Put command bar
  vector<pair<string, string> > sourceMenuCmd;
  sourceMenuCmd.push_back(make_pair<string,string>("^C", "Cancel"));
  m_statusBar->CommandBar(sourceMenuCmd);

  // now create the menu
  MENU *sourceMenu;
  const int nSourceItems = allSources.size();
  ITEM *sourceItems[nSourceItems+2]; //+2 = title + end-of-items item
  string itemStr[nSourceItems+1]; // item string values
  //we need a formatted table menu
  // create each item in the form:
  // |  type  |  name  | format |
  // where type is max 6 char, format is max 6 char, name takes is at most the rest of the available space.
  int maxTypeWidth = 6;
  int maxFormatWidth = 6;
  char tableSeparator = '|';
  int maxNameWidth = 0;
  // first determine the maximum length of 'name' field
  for (vector<SourceURI>::iterator its = allSources.begin(); its != allSources.end(); ++its) {
    string sourceName = (*its).GetField(SourceURI::NAME);
    if ((int)sourceName.length() > maxNameWidth)
      maxNameWidth = (*its).GetField(SourceURI::NAME).length(); 
  }
  // check if max_name_width is displayable (at least 6 spaces for separators: 4 for '|' plus one extra space at each end of the screen)
  if (maxNameWidth > maxCOLS - (maxTypeWidth + maxFormatWidth + 6))
    maxNameWidth = maxCOLS - (maxTypeWidth + maxFormatWidth + 6);
  // make first item (will be grayed) just as label
  itemStr[0] = PadStrCenter("Type",maxTypeWidth);
  itemStr[0] += tableSeparator;  
  itemStr[0] += PadStrCenter("Name",maxNameWidth);
  itemStr[0] += tableSeparator;  
  itemStr[0] += PadStrCenter("Format",maxFormatWidth);
  sourceItems[0] = new_item(itemStr[0].c_str(), 0);
  // then make each new item
  int curItem=1;
  for (vector<SourceURI>::iterator its = allSources.begin(); its != allSources.end(); ++its) {
    //format each item
    //    itemStr[curItem] = tableSeparator; // use menu border
    itemStr[curItem] += PadStrCenter((*its).GetField(SourceURI::TYPE), maxTypeWidth);
    itemStr[curItem] += tableSeparator;
    itemStr[curItem] += PadStrCenter((*its).GetField(SourceURI::NAME), maxNameWidth);
    itemStr[curItem] += tableSeparator;
    itemStr[curItem] += PadStrCenter((*its).GetField(SourceURI::FORMAT), maxFormatWidth);
    //    itemStr[curItem] += tableSeparator; // use menu border
    sourceItems[curItem] = new_item(itemStr[curItem].c_str(), 0);
    *log << ILog::DEBUG << "Adding source to the list: " << (*its).str() << " (pos: " << curItem << "/" << nSourceItems << ")" 
	 << this << ILog::endmsg;
    curItem++;    
  }
  sourceItems[nSourceItems+1] = new_item(NULL, NULL);
  // make the menu and set options
  item_opts_off(sourceItems[0], O_ACTIVE); //make title a static label
  item_opts_off(sourceItems[0], O_SELECTABLE); //make title a static label
  ///@todo: decide if we want the "new source" option here
  //  item_opts_off(sourceItems[nSourceItems], O_SELECTABLE); //make "new source" not selectable (still to be implemented: do we want it?
  sourceMenu = new_menu((ITEM**)sourceItems);
  string menuHeader(" Select destination ");
  int menuWidth = maxTypeWidth + maxNameWidth + maxFormatWidth + 6; // space for window decoration + 2 blank spaces right/left (one for menu mark)
  menuWidth = max((int)menuHeader.size() + 2, menuWidth); //maxNameWidth already limited above
  int menuHeight = allSources.size() + 5; //borders + space (top-bottom) + table title
  menuHeight = min(maxLINES - 2, menuHeight); //limit to the screen (excluding header and status bar and 2 spaces), in case it becomes a scroll menu
  int menuY = m_statusBar->GetHeaderHeight() + (maxLINES - menuHeight) / 2; // centered
  int menuX = (maxCOLS - menuWidth) / 2;
  WINDOW *sourceWnd = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *sourceItemWnd = subwin(sourceWnd, menuHeight - 4, menuWidth - 2, menuY+2, menuX+1);
  *log << ILog::DEBUG << "Created SourceSelect window: " 
       << "H:" << menuHeight << ",W:" << menuWidth 
       << ",Y:" << menuY << ",X:" << menuX << this << ILog::endmsg;
  *log << ILog::DEBUG << "Created SourceSelect item subwindow: " 
       << "H:" << menuHeight - 4 << ",W:" << menuWidth - 2 
       << ",Y:" << menuY+2 << ",X:" << menuX+1 << this << ILog::endmsg;
  set_menu_win(sourceMenu, sourceWnd);
  set_menu_sub(sourceMenu, sourceItemWnd);
  keypad(sourceWnd, true);
  //draw box around the menu
  box(sourceWnd, 0, 0);
  //Draw "title"
  mvwprintw(sourceWnd, 0, (menuWidth - menuHeader.size()) / 2, "%s", menuHeader.c_str());  

  //print everything!
  post_menu(sourceMenu);
  set_current_item(sourceMenu, sourceItems[1]); //put on first selection available after Title
  wrefresh(sourceWnd);

  int sourceSelected = 0;
  while (sourceSelected == 0) {
    int c = getch();
    m_statusBar->StatusBar(); //clear status bar from previous messages
    switch (c) {
    case KEY_DOWN:
      menu_driver(sourceMenu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(sourceMenu, REQ_UP_ITEM);
      break;
    case KEY_ENTER: //Enter
    case 10: //Enter
    case 13: //Enter
      {
	//ok, find out what's the requested action and act accordingly
	ITEM *selItem = current_item(sourceMenu);
	sourceSelected = item_index(selItem); // 0 = title, cannot happen (-> continue loop)
	break;
      } // end source selected
    case CTRL('c'):
      //user abort
      sourceSelected = -1;
      break;
    } // end charachter pressed
    wrefresh(sourceWnd);
  } // quit source selection

  //free everything!
  unpost_menu(sourceMenu);
  free_menu(sourceMenu);
  for (int it=0; it<nSourceItems+2; it++)
    free_item(sourceItems[it]);

  // almost-last thing: translate in a source string
  sourceSelected--; //not count title
  SourceURI sourceSelectedStr;
  if (sourceSelected == nSourceItems-1) {
    //create new source and put into sourceSelectedStr
    // ok, now not implemented. now sure we want it (disabled in the menu)
    *log << ILog::DEBUG << "Selected to create a new source. This is not implemented." 
	 << this << ILog::endmsg;
    m_errorMsg = "Source creation not implemented here yet.";
    m_statusCode = SC_ERROR;    
  } else if (sourceSelected < 0) {
    //user decide to quit
    *log << ILog::DEBUG << "User aborted source selection." << this << ILog::endmsg;
    m_errorMsg = "Source selection cancelled.";
    sourceSelectedStr.SetURI("");
    m_statusCode = SC_ABORT;
  } else {
    sourceSelectedStr = allSources[sourceSelected];
    m_source = sourceSelectedStr; // set also m_source
    m_statusCode = SC_OK;
  }

  //clear screen
  wclear(sourceWnd);
  wrefresh(sourceWnd);

  delwin(sourceItemWnd);
  delwin(sourceWnd);  

  return sourceSelectedStr; 

}

ARecord TuiAccount::SelectPredefAccType(bool emptyShow)
{
  m_statusCode = SC_OK;
  *log << ILog::DEBUG << "Selecting predefined account type." << this << ILog::endmsg;
  //Get size of the window
  int maxCOLS=COLS;
  int maxLINES=LINES - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight());
  //get existing account types
  vector<ARecord> allAccTypes = cfgMgr->GetPredefinedAccountTypes();
  if (allAccTypes.size() == 0) return ARecord("EMPTY", "");
  if (emptyShow)
    allAccTypes.push_back(ARecord("EMPTY", "")); //add special type for empty account selection
  
  //Put command bar
  vector<pair<string, string> > menuCmd;
  menuCmd.push_back(make_pair<string,string>("^C", "Cancel"));
  menuCmd.push_back(make_pair<string,string>("E", "Empty Account"));
  m_statusBar->CommandBar(menuCmd);

  // now create the menu
  MENU *accMenu;
  const int nAccItems = allAccTypes.size();
  ITEM *accItems[nAccItems+2]; //+2 = title + end-of-items item
  string itemStr[nAccItems+1]; // item string values
  //we need a formatted table menu
  // create each item in the form:
  // |  name  | fields |
  // where name is max 20 chars, fields is at most the rest of the available space.
  int maxNameWidth = 20;
  int maxFieldsWidth = 8; // min. length of title
  char tableSeparator = '|';
  // first determine the maximum length of 'name' and 'fields'
  int tmpMaxNameWidth=6; // min. length of title
  for (vector<ARecord>::iterator its = allAccTypes.begin(); its != allAccTypes.end(); ++its) {
    if ((int)its->GetAccountName().length() > tmpMaxNameWidth)
      tmpMaxNameWidth = its->GetAccountName().length();
  }
  maxNameWidth = min(tmpMaxNameWidth, maxNameWidth); //we need less space than the maximum allowed?
  for (vector<ARecord>::iterator its = allAccTypes.begin(); its != allAccTypes.end(); ++its) {
    string fieldListStr = CreateListStr(its->GetFieldNameList());
    if ((int)fieldListStr.length() > maxFieldsWidth)
      maxFieldsWidth = fieldListStr.length();
  }    
  // check if maxFieldsWidth is displayable (at least 5 spaces for separators: 3 for '|' plus one extra space at each end of the screen)
  maxFieldsWidth = min(maxFieldsWidth, maxCOLS - (maxNameWidth + 5));
  // make first item (will be grayed) just as label
  itemStr[0] = PadStrCenter("Name",maxNameWidth);
  itemStr[0] += tableSeparator;  
  itemStr[0] += PadStrCenter("Fields",maxFieldsWidth);
  accItems[0] = new_item(itemStr[0].c_str(), 0);
  // then make each new item
  int curItem=1;
  for (vector<ARecord>::iterator its = allAccTypes.begin(); its != allAccTypes.end(); ++its) {
    //format each item
    //    itemStr[curItem] = tableSeparator; // use menu border
    itemStr[curItem] += PadStrCenter(its->GetAccountName(), maxNameWidth);
    itemStr[curItem] += tableSeparator;
    itemStr[curItem] += PadStrCenter(CreateListStr(its->GetFieldNameList()), maxFieldsWidth);
    //    itemStr[curItem] += tableSeparator; // use menu border
    accItems[curItem] = new_item(itemStr[curItem].c_str(), 0);
    curItem++;    
  }
  accItems[nAccItems+1] = new_item(NULL, NULL);
  // make the menu and set options
  item_opts_off(accItems[0], O_ACTIVE); //make title as label
  item_opts_off(accItems[0], O_SELECTABLE); //make title as label
  accMenu = new_menu((ITEM**)accItems);
  string menuHeader(" Select account type ");
  int menuWidth = maxNameWidth + maxFieldsWidth + 5; // space for window decoration + 2 blank spaces right/left (one for menu mark)
  menuWidth = max((int)menuHeader.size() + 2, menuWidth); //maxNameWidth and maxFieldsWidth already limited above
  int menuHeight = allAccTypes.size() + 5; //borders + space (top-bottom) + table title
  menuHeight = min(maxLINES - 2, menuHeight); //limit to the screen (excluding header and status bar and 2 spaces), in case it becomes a scroll menu
  int menuY = m_statusBar->GetHeaderHeight() + (maxLINES - menuHeight) / 2; // centered
  int menuX = (maxCOLS - menuWidth) / 2;
  WINDOW *accWnd = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *accItemWnd = subwin(accWnd, menuHeight - 4, menuWidth - 2, menuY+2, menuX+1);
  set_menu_win(accMenu, accWnd);
  set_menu_sub(accMenu, accItemWnd);
  keypad(accWnd, true);
  //draw box around the menu
  box(accWnd, 0, 0);
  //Draw "title"
  mvwprintw(accWnd, 0, (menuWidth - menuHeader.size()) / 2, "%s", menuHeader.c_str());  

  //print everything!
  post_menu(accMenu);
  set_current_item(accMenu, accItems[1]); //put on first selection available after Title
  wrefresh(accWnd);

  int accSelected = 0;
  while (accSelected == 0) {
    int c = getch();
    m_statusBar->StatusBar(); //clear status bar from previous messages
    switch (c) {
    case KEY_DOWN:
      menu_driver(accMenu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(accMenu, REQ_UP_ITEM);
      break;
    case KEY_ENTER: //Enter
    case 10: //Enter
    case 13: //Enter
      {
	//ok, find out what's the requested action and act accordingly
	ITEM *selItem = current_item(accMenu);
	accSelected = item_index(selItem); // 0 = title, cannot happen (-> continue loop)
	break;
      } // end source selected
    case 'e':
    case 'E':
      //select empty account
      accSelected = nAccItems; //remember: title=0
      break;
    case CTRL('c'):
      //user abort
      accSelected = -1;
      break;
    } // end charachter pressed
    wrefresh(accWnd);
  } // quit source selection

  //free everything!
  unpost_menu(accMenu);
  free_menu(accMenu);
  for (int it=0; it<nAccItems+2; it++)
    free_item(accItems[it]);

  // almost-last thing: translate in a source string
  accSelected--; //not count title
  ARecord selectedAccType; //if EMPTY selected, will return a fresh selectedAccType
  if (accSelected < 0) {
    //user decide to quit
    *log << ILog::DEBUG << "User aborted predefined account selection." << this << ILog::endmsg;
    m_statusCode = SC_ABORT;
  } else if ( accSelected < nAccItems ) { 
    if (! ( emptyShow == true && accSelected == nAccItems - 1 ) )   // do not include special account empty
	selectedAccType = allAccTypes[accSelected];
    m_statusCode = SC_OK;
  }

  //clear screen
  wclear(accWnd);
  wrefresh(accWnd);

  delwin(accItemWnd);
  delwin(accWnd);  

  return selectedAccType; 

}

int TuiAccount::SelectOne(string pTitle, vector<string> pChoices)
{
  //- Init
  m_statusCode = SC_OK;
  *log << ILog::DEBUG << "Entering selection: " << pTitle << this << ILog::endmsg;
  int maxCOLS=tuiSvc->GetScreenCols();
  int maxLINES=tuiSvc->GetScreenRows() - (m_statusBar->GetHeaderHeight() + m_statusBar->GetBottomBarHeight());  
  // put command bar standard
  vector<pair<string, string> > menuCmd;
  menuCmd.push_back(make_pair<string,string>("^C", "Cancel"));
  m_statusBar->CommandBar(menuCmd);

  //- Now create the menu
  MENU *selMenu;
  const int nSelItems = pChoices.size();
  ITEM *selItems[nSelItems+1]; //+1 = end-of-items item
  size_t maxNameWidth = pTitle.size(); // at least this size
  // make each new item
  int curItem=0;
  for (vector<string>::iterator its = pChoices.begin(); its != pChoices.end(); ++its) {
    selItems[curItem] = new_item(its->c_str(), 0);
    maxNameWidth = max(maxNameWidth, its->length());
    curItem++;    
  }
  selItems[nSelItems] = new_item(NULL, NULL);

  // make the menu and set options
  selMenu = new_menu((ITEM**)selItems);
  string menuHeader;
  menuHeader = string(" ") + pTitle + string (" "); //pad with one space each side
  int menuWidth = maxNameWidth + 5; // space for window decoration + 2 blank spaces right/left + marker
  menuWidth = min(menuWidth, maxCOLS - 5); // max displayable width (cut borders, 2 spaces, marker)
  int menuHeight = pChoices.size() + 4; //borders + space (top-bottom)
  menuHeight = min(maxLINES - 2, menuHeight); //limit to the screen (excluding header and status bar and 2 spaces), in case it becomes a scroll menu
  int menuY = m_statusBar->GetHeaderHeight() + (maxLINES - menuHeight) / 2; // centered
  int menuX = (maxCOLS - menuWidth) / 2;
  WINDOW *selWnd = newwin(menuHeight, menuWidth, menuY, menuX);
  WINDOW *selItemWnd = subwin(selWnd, menuHeight - 4, menuWidth - 2, menuY+2, menuX+2); 
  *log << ILog::DEBUG << "Created window: " 
       << "H:" << menuHeight << ",W:" << menuWidth 
       << ",Y:" << menuY << ",X:" << menuX << this << ILog::endmsg;
  set_menu_win(selMenu, selWnd);
  set_menu_sub(selMenu, selItemWnd);
  keypad(selWnd, true);

  // draw box around the menu
  box(selWnd, 0, 0);
  // Draw "title"
  mvwprintw(selWnd, 0, (menuWidth - menuHeader.size()) / 2, "%s", menuHeader.c_str());  

  //- Print everything!
  post_menu(selMenu);
  set_current_item(selMenu, selItems[0]); //put on first selection available after Title
  wrefresh(selWnd);

  // - Get user input
  int Selected = -1;
  while (Selected == -1) {
    int c = getch();
    m_statusBar->StatusBar(); //clear status bar from previous messages
    switch (c) {
    case KEY_DOWN:
      menu_driver(selMenu, REQ_DOWN_ITEM);
      break;
    case KEY_UP:
      menu_driver(selMenu, REQ_UP_ITEM);
      break;
    case KEY_ENTER: //Enter
    case 10: //Enter
    case 13: //Enter
      {
	//ok, find out what's the choice
	ITEM *selItem = current_item(selMenu);
	Selected = item_index(selItem); 
	break;
      } 
    case CTRL('c'):
      Selected = -2; //user abort
      break;
    } // end charachter pressed
    wrefresh(selWnd);
  } // quit selection

  //- Free everything!
  unpost_menu(selMenu);
  free_menu(selMenu);
  for (int it=0; it<nSelItems+1; it++)
    free_item(selItems[it]);

  // - Check user choice
  if (Selected < 0) {
    //user decide to quit
    *log << ILog::DEBUG << "User aborted selection." << this << ILog::endmsg;
    m_statusCode = SC_ABORT;
  } else if (Selected < nSelItems) { 
    m_statusCode = SC_OK;
  }

  //- Clear screen and quit
  wclear(selWnd);
  wrefresh(selWnd);
  delwin(selItemWnd);
  delwin(selWnd);  

  return Selected; //return index inside vector

}

int TuiAccount::GetFieldTitleSize(string pTitle)
{
    int effTitleSize = pTitle.size();
    if (effTitleSize <= m_cfgFieldNameWidth && effTitleSize < COLS / 2) {
      effTitleSize = m_cfgFieldNameWidth; //pad to m_cfgFieldNameWidth
    } else if (effTitleSize > COLS / 2) {
      effTitleSize = COLS / 2;
    } 
    return effTitleSize;
}

void TuiAccount::PrintFieldTitles()
{
  //save cursor position
  int cursx, cursy;
  getyx(m_wnd, cursy, cursx);
  int totfields = m_record->GetNumberOfFields() + 3; //3=account name, labels, essentials
  int fieldSelected = field_index(current_field(m_accountForm));
  //first print title for accountproperties
  mvwprintw(m_wnd, 0, 1, "----- Account properties -----");
  for (int idf=0; idf < totfields; idf++) {  
    // print title of the field
    int starty;
    field_info(m_accountFields[idf], 0, 0, &starty, 0, 0, 0);
    //check if we're starting account fields, if so print title
    if (idf == m_numberFieldsAccProp) 
      mvwprintw(m_wnd, starty - 1, 1, "----- Account fields ---------");      
    string title(static_cast<char*>(field_userptr(m_accountFields[idf])));
    int effTitleSize = GetFieldTitleSize(title);
    if (fieldSelected == field_index(m_accountFields[idf]))
      wattron(m_wnd, A_REVERSE);
    mvwprintw(m_wnd, starty, 1, title.substr(0, effTitleSize).c_str()); //leave one space at the window border
    if (fieldSelected == field_index(m_accountFields[idf]))
      wattroff(m_wnd, A_REVERSE);
    mvwprintw(m_wnd, starty, effTitleSize+1, ":"); // +1: space for left forder
  }
  //restore cursor position
  wmove(m_wnd, cursy, cursx);
}

void TuiAccount::CreateFields()
{
  m_statusCode = SC_OK;

  // --- create new form for editing
  // We need to do this job here, since it's value is determined only at this point and 
  // can dinamically change (adding/deleting fields, changing record while browsing)
  // - create fields
  m_numberFieldsAccProp = 3; //AccountName, Labels, Essentials
  int totfields=m_numberFieldsAccProp; 
  int starty=1; //m_wnd_y; // start from 1, since first row is for title: --- Account properties ---
  totfields += m_record->GetNumberOfFields(); // add number of custom fields
  m_accountFields = new FIELD* [totfields+1]; //space for empty field at the end
  ARecord::TFieldsIterator itf=m_record->GetFieldsIterBegin();
  for (int idf = 0; idf < totfields; ++idf) {
    // boundary check.. just to be sure
    if (itf == m_record->GetFieldsIterEnd() && itf != m_record->GetFieldsIterBegin()) {
      *log << ILog::FIXME << "Size of fields in record does not match actual elements stored." << this << ILog::endmsg;
    }
    // get field contents
    string f_title;
    string f_content;
    if (idf == fNameIdx) {
      f_title = "Account Name";
      f_content = m_record->GetAccountName();
    } else if (idf == fLabelsIdx) {
      f_title = "Labels";
      f_content = CreateListStr(m_record->GetLabels());
    } else if (idf == fEssentialsIdx) {
      f_title = "Essentials";
      f_content = CreateListStr(m_record->GetEssentials());
    } else {
      f_title = itf->first;
      f_content = itf->second;
      //if first user fields, skip an extra line for title: --- Account fields ---
      if (itf == m_record->GetFieldsIterBegin()) 
	++starty;
      ++itf; // get next element
    }
    int effTitleSize = GetFieldTitleSize(f_title);
    int f_cols = m_wnd_cols - effTitleSize - 3; //space for delimiter (colon + space) and intial pad
    int f_x = effTitleSize + 3;
    // TODO: make fields automatically grows in rows
    int f_rows = count(f_content.begin(), f_content.end(), '\n')+1;
    int f_rows_display = f_rows;
    if (f_rows > m_maxFieldDisplayHeight)
    f_rows_display = m_maxFieldDisplayHeight;
    // create field from m_rowOffset to the end of the screen line
    //    m_accountFields[idf] = new_field(f_rows_display, f_cols, starty, f_x, f_rows - f_rows_display, 0); 
    m_accountFields[idf] = new_field(1, f_cols, starty, f_x, 0, 0); 
    *log << ILog::DEBUG << "Created new field: " 
	 << f_rows_display << "," << f_cols << "," << starty << "," << f_x << "," << f_rows - f_rows_display
	 << this << ILog::endmsg;
    //store field title and content (use userptr for title, need its own memory)
    char *userptr = new char[f_title.length()+1]; //+1 for trailing \x0
    strcpy(userptr, f_title.c_str());
    set_field_userptr(m_accountFields[idf], userptr);
    set_field_buffer(m_accountFields[idf], 0, f_content.c_str());
    set_field_status(m_accountFields[idf], 0);
    // set field options
    field_opts_on(m_accountFields[idf], O_VISIBLE);
    field_opts_on(m_accountFields[idf], O_ACTIVE);
    field_opts_on(m_accountFields[idf], O_PUBLIC);
    field_opts_on(m_accountFields[idf], O_WRAP);
    field_opts_off(m_accountFields[idf], O_BLANK);
    field_opts_off(m_accountFields[idf], O_AUTOSKIP);
    //set_field_back(m_accountFields[idf], A_UNDERLINE);
    if (idf == 0)
      field_opts_off(m_accountFields[idf], O_NULLOK); //requires a valid account name
    else 
      field_opts_on(m_accountFields[idf], O_NULLOK);
    if (m_fieldsLocked) {
      //lock field content
      field_opts_off(m_accountFields[idf], O_EDIT);
    }
    field_opts_off(m_accountFields[idf], O_STATIC); //dynamic fields
    // display field content
    starty+= f_rows_display; //advance of needed rows
  }
  m_accountFields[totfields] = NULL;
  
  // - set form options

  // - create form and associate to window
  m_accountForm = new_form(m_accountFields);
  set_form_win(m_accountForm, m_wnd);
  set_form_sub(m_accountForm, m_accountFieldWnd);
  
  // - print
  post_form(m_accountForm);
  set_current_field(m_accountForm, m_accountFields[0]);
  //enable cursor
  curs_set(1); 
  int firstFieldCol;
  int firstFieldRow;
  field_info(m_accountFields[0], NULL, NULL, &firstFieldRow, &firstFieldCol, NULL, NULL);
  wmove(m_wnd, firstFieldRow, firstFieldCol);

}

void TuiAccount::UpdateAndFreeForm()
{
  m_statusCode = SC_OK;

  //check if a form exists, otherwise, nothing to do
  if (!m_accountForm || !m_accountFields)
    return;

  //update m_record looping over all fields
  // we access members directly, this partially by-pass the initial temptative of LOCK feature
  // Call this function with the same structure of fields in the record and the form (should always be) or 
  // results can be not quite right in some peculiar cases of multiple same-name fields added/deleted.
  *log << ILog::DEBUG << "Updating m_record." << this << ILog::endmsg;
  ARecord::TFieldsIterator recordField = m_record->GetFieldsIterBegin();
  for (int idx = 0; m_accountFields[idx] != NULL; idx++) {
    //get field content and title
    set_current_field(m_accountForm, m_accountFields[idx]);
    form_driver(m_accountForm, REQ_BEG_FIELD);
    string title( TrimStr( static_cast<char*>( field_userptr( m_accountFields[idx] )) ) );    
    string content( TrimStr( static_cast<char*>( field_buffer (m_accountFields[idx], 0) ) ) );
    int index = field_index(m_accountFields[idx]);
    //update corresponding field, if present (safe against re-draw after field deleting)
    bool fieldChanged(false);
    if (field_status(m_accountFields[idx]) != 0) 
      fieldChanged=true;
    if (fieldChanged) {
      *log << ILog::DEBUG << "Field " << idx << ": '" << title << "' changed. Saving: '" 
	   << content << "'" << this << ILog::endmsg;
    }
    if (index == fNameIdx) { //Account name
      m_record->SetAccountName( content );
    } else if (index == fLabelsIdx) {
      m_record->ClearLabels();
      m_record->AddLabels( SplitListStr( content ) );
    } else if (index == fEssentialsIdx) {
      m_record->ClearEssentials();
      m_record->AddEssentials( SplitListStr( content ) );	
    } else {
      //update field if changed or if it is essential (to ensre it's stored even if empty)
      if (fieldChanged or m_record->HasEssential(title)) { 
	//check if field exists, if not continue (guess: that field has been deleted)
	if (not (recordField->first == title)) {
	  *log << ILog::ERROR << "Mismatch of fields. Needs debugging?" 
	       << ". m_record->HasField(" << title << ") = " << m_record->HasField(title) 
	       << ILog::endmsg;	 
	  m_statusBar->StatusBar("Internal error occurred. Please DEBUG (see log file).");
	  break;
	}
	//update it
	recordField->second = TrimStr(content);
      }
      //always advance recorditerator as well if not a special field
      ++recordField;
    }
  } // loop over fields
  *log << ILog::DEBUG << "m_record updated." << this << ILog::endmsg;

  //unpost form
  unpost_form(m_accountForm);
  //free memory
  free_form(m_accountForm);
  m_accountForm = 0;
  for (int idx = 0; m_accountFields[idx] != NULL; idx++) {
    //delete user pointer
    char *userptr = static_cast<char*>(field_userptr(m_accountFields[idx]));
    if (userptr)
      delete[] userptr;
    //delete field
    free_field(m_accountFields[idx]);
    m_accountFields[idx] = 0;
  }
  delete[] m_accountFields;
  m_accountFields = 0;
  
}
