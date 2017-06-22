//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
// History:
//  24 Jan 2010 by William Blum: Column sorting plugin based on NppSort code.
//           More info at http://william.famille-blum.org
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include <string.h>
#include "PluginDefinition.h"
#include "Scintilla.h"
#include "menuCmdID.h"
#include "commctrl.h"
#include "resource1.h"
#include <vector>
#include <algorithm>

using namespace std;


extern HMODULE g_hModule;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;



size_t g_iEOLSize = 0;

// Handle of the criteria setting dialog box
HWND hSettings = NULL;

enum OrderDirection
{
    Ascending,
    Descending
};

enum CaseSensitivity
{
    CaseSensitive,
    CaseInsensitive,
    Numeric
};

typedef struct
{
    int from;
    int length;
    OrderDirection order;
    CaseSensitivity casesensitivy;
} ColumnCriteria;

// list of criterias
std::vector<ColumnCriteria> criterias;


//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    PTSTR szMenu[3] =
    { LoadResString(g_hModule, IDS_SORTMENU),
      LoadResString(g_hModule, IDS_SETTINGSMENU),
      LoadResString(g_hModule, IDS_ABOUTMENU) };
    setCommand(0, szMenu[0], SortCommand, NULL, false);
    setCommand(1, szMenu[1], CriteriaCommand, NULL, false);
    setCommand(2, szMenu[2], AboutCommand, NULL, false);
    FreeResString(szMenu[0]);
    FreeResString(szMenu[1]);
    FreeResString(szMenu[2]);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    // Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void CriteriaCommand()
{   
#ifdef MODLESS
    if( hSettings == NULL)
    {
        hSettings = CreateDialog((HMODULE)g_hModule, MAKEINTRESOURCE(IDD_COLUMNS), nppData._nppHandle, ColumnsDialogProc);
    }
    ShowWindow(hSettings, SW_SHOW);
#else
        DialogBox((HMODULE)g_hModule, MAKEINTRESOURCE(IDD_COLUMNS), nppData._nppHandle, ColumnsDialogProc);
#endif
}


void AboutCommand()
{   
    PTSTR msg = LoadResString(g_hModule, IDS_ABOUTSTRING);
    PTSTR title = LoadResString(g_hModule, IDS_PLUGINTITLE);
    ::MessageBox(nppData._nppHandle, msg, title, MB_OK);
    FreeResString(msg);
    FreeResString(title);
}

void HelloCommand()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");
}



// Load a string from the resources
PTSTR LoadResString(HMODULE hInst, UINT wID)
{
    UINT    block, num;
    int     len = 0;
    HRSRC   hRC = NULL;
    HGLOBAL hgl = NULL;
    PWSTR  str = NULL;
    register UINT i;
    PTSTR pResStr;

    if( wID == 0)
        return 0 ;

    block = (wID >> 4) + 1;// compute block number
    num = wID & 0xf;// compute offset into block

    hRC = FindResource(hInst, MAKEINTRESOURCE(block), RT_STRING);
    if (!hRC)
        return 0 ;

    hgl = LoadResource(hInst, hRC);
    if (!hgl)
        return 0 ;

    str = (LPWSTR)LockResource(hgl);
    if (!str)
        return 0 ;

    // Move up block to string we want
    for (i = 0; i < num; i++)
        str += *str + 1;

    // convert the string to current code page
    len = *str;

    pResStr = (PTSTR)malloc(sizeof(TCHAR) * (len+1) );

    if( pResStr )
    {
        // convert the string to current code page
#ifdef _UNICODE     
        wcsncpy(pResStr, str+1, len);
#else

        len = WideCharToMultiByte(CP_ACP,
                                  WC_COMPOSITECHECK,
                                  str + 1, *str,
                                  pResStr, len,
                                  NULL, NULL);
#endif

        pResStr[len] = '\0';
    }

    return pResStr;
}

void FreeResString(PTSTR pszResStr)
{
    free(pszResStr);
}


PTSTR LoadStringFromDlg(HWND hwndDlg, int id)
{
    PTSTR psz;
    int len = (int)SendDlgItemMessageA(hwndDlg, id, WM_GETTEXTLENGTH, 0, 0) +1;
    psz = (PTSTR)malloc(len*sizeof(TCHAR));
    GetDlgItemText(hwndDlg, id, psz, len);

    int j = -1;
    for(int i=0; i<len;i++)
    {
        if(psz[i] == '&')
        {
            j = i;
        }
        else if(j>=0)
        {
            psz[j++] = psz[i];
        }
    }
    return psz;
}



int ColumnCompareFunction(const string& S1, const string& S2, size_t from, size_t len, CaseSensitivity caseSensitivity)
{
    size_t l1 = S1.length();
    size_t l2 = S2.length();

    // Is S1 not long enough for this criteria?
    if(l1 < from )
    {
        if( l2 < from )
            return 0; // both S1 and S2 are too short
        else
            return -1; // S1 is shorter
    }
    else if( l2 < from )
        return +1; // S2 is shorter

    int fieldLen1 = min(len, l1-from);
    int fieldLen2 = min(len, l2-from);
    int mLength = min(fieldLen1, fieldLen2);

    if( caseSensitivity == CaseSensitive )
    {
        int c = strncmp(S1.c_str()+from, S2.c_str()+from, mLength);
        if( c != 0 )
            return c;
        else
            return fieldLen1-fieldLen2;
    }
    else if( caseSensitivity == CaseInsensitive )
    {
        int c = strnicmp(S1.c_str()+from, S2.c_str()+from, mLength);
        if( c != 0 )
            return c;
        else
            return fieldLen1-fieldLen2;
    }
    else if( caseSensitivity == Numeric )
    {
        int i1 = atoi(S1.substr(from, len).c_str());
        int i2 = atoi(S2.substr(from, len).c_str());
        return i1-i2;
    }
    return 0;
}

// Line comparison function
int LineCompareFunction(const string& S1, const string& S2)
{
    // Compare how may chars? Use the length of the smaller string
    size_t l1 = S1.length();
    size_t l2 = S2.length();
    int mLength = l1 < l2 ? l1 : l2;

    // Don't compare the end of line chars
    mLength -= g_iEOLSize;

    // If no criteria is set then do a normal comparison on the entire line
    if( criterias.size() == 0 )
    {
        // perform a case insensitive comparison
        int c = strnicmp(S1.c_str(), S2.c_str(), mLength);
        if ( c != 0 )
            return c;

        // S1 and S2 have a common prefix but possibly with different cases
        c = l1 - l2;
        if ( c != 0 )
            return c;

        // the string have same length:  perform a case-sensitive comparison
        return strncmp(S1.c_str(), S2.c_str(), mLength);
    }
    else
    {
        // Loop through the criterias
        for(vector<ColumnCriteria>::iterator it = criterias.begin();
            it != criterias.end();
            it++)
        {
            int c = ColumnCompareFunction(S1, S2, it->from-1, it->length, it->casesensitivy);
            if ( c != 0 )
            {
                return it->order == Ascending ? c : -c;
            }
        }
        
        return false;
    }

}

bool CompareFunction(const string& S1, const string& S2)
{
    return LineCompareFunction(S1, S2) < 0 ? true : false;
}

void SortCommand()
{
    int m_iLineLength = 0, m_iStartPos = 0, m_iEndPos = 0, m_iEOL, m_iFirstLine = 0, m_iLastLine = 0;
    int m_iLineCount = 0;
    int currentEdit = 0;
    vector<string> strLines;
    string strLine;
    static const basic_string <char>::size_type npos = (size_t)-1;
    char *szLine, szEOL[3];

    HWND hCurrentEditView = nppData._scintillaMainHandle;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
    if (currentEdit)
        hCurrentEditView = nppData._scintillaSecondHandle;

    m_iFirstLine = ::SendMessage(hCurrentEditView, SCI_GETANCHOR, 0, (LPARAM)0);
    m_iLastLine = ::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, (LPARAM)0);
    if ( m_iLastLine < m_iFirstLine )
    {
        int m_iTemp = m_iLastLine;
        m_iLastLine = m_iFirstLine;
        m_iFirstLine = m_iTemp;
    }

    m_iFirstLine = ::SendMessage(hCurrentEditView, SCI_LINEFROMPOSITION, m_iFirstLine, 0);
    m_iLastLine = ::SendMessage(hCurrentEditView, SCI_LINEFROMPOSITION, m_iLastLine, 0);
    m_iLineCount = ::SendMessage(hCurrentEditView, SCI_GETLINECOUNT, 0, (LPARAM)0);

    if ( m_iFirstLine == m_iLastLine )
    {
        m_iFirstLine = 0;
        m_iLastLine = m_iLineCount - 1;
    }
    m_iEOL = ::SendMessage(hCurrentEditView, SCI_GETEOLMODE, 0, (LPARAM)0);

    if ( m_iEOL == SC_EOL_CRLF ) //windows
    {
        g_iEOLSize = 2;
        strcpy(szEOL, "\r\n");
    }
    else if ( m_iEOL == SC_EOL_CR ) //unix
    {
        g_iEOLSize = 1;
        strcpy(szEOL, "\n");
    }
    else //mac
    {
        g_iEOLSize = 1;
        strcpy(szEOL, "\r");
    }

    for (int i = m_iFirstLine; i <= m_iLastLine; i++)
    {
        m_iLineLength = ::SendMessage(hCurrentEditView,
            SCI_LINELENGTH, (WPARAM)i, (LPARAM)0);

        szLine = new char[m_iLineLength + 1];
        strLine.resize(m_iLineLength + 1);

        ::SendMessage(hCurrentEditView,
            SCI_GETLINE, (WPARAM)i, (LPARAM)&szLine[0]);

        memset(&szLine[m_iLineLength], 0, 1);
        if (m_iLineLength > 0)
            strLine.assign(szLine);
        else
            strLine.assign(szEOL);

        if ( strLine.at(strLine.length() - 1) != '\r' && strLine.at(strLine.length() - 1) != '\n' )
            strLine.append(szEOL);

        strLines.push_back(strLine);
        delete [] szLine;
    }

    sort(strLines.begin(), strLines.end(), CompareFunction);

    m_iStartPos = ::SendMessage(hCurrentEditView, SCI_POSITIONFROMLINE, m_iFirstLine, 0);
    m_iEndPos = ::SendMessage(hCurrentEditView, SCI_POSITIONFROMLINE, m_iLastLine+1, 0);

    ::SendMessage(hCurrentEditView, SCI_BEGINUNDOACTION, 0, 0);
    ::SendMessage(hCurrentEditView, SCI_SETTARGETSTART, m_iStartPos, 0);
    ::SendMessage(hCurrentEditView, SCI_SETTARGETEND, m_iEndPos, 0);

    ::SendMessage(hCurrentEditView, SCI_REPLACETARGET, 0, (LPARAM)"");
    //::SendMessage(hCurrentEditView, SCI_CLEARALL, (WPARAM)0, (LPARAM)0);

    size_t totalSize = 0;
    for (size_t i = 0; i < strLines.size(); i++)
    {
        //If last item, erase new line characters
        if ( i + 1 == strLines.size()
            && strLines[i].length() >= g_iEOLSize
            && m_iLineCount == m_iLastLine - m_iFirstLine + 1 )
                strLines[i].erase(strLines[i].length() - g_iEOLSize, g_iEOLSize);


        ::SendMessage(hCurrentEditView, SCI_ADDTEXT,
            (WPARAM)strLines[i].length(), (LPARAM)strLines[i].c_str());
        totalSize += strLines[i].length();

    }
    ::SendMessage(hCurrentEditView, SCI_SETANCHOR, m_iStartPos, (LPARAM)0);
    ::SendMessage(hCurrentEditView, SCI_SETCURRENTPOS, m_iEndPos-1, (LPARAM)0);

    ::SendMessage(hCurrentEditView, SCI_ENDUNDOACTION, 0, 0);
}


#define ListView_GetSelectedItem(hwnd)      ListView_GetNextItem((hwnd), -1, LVNI_SELECTED)

// Swap two items in a ListView
void SwapListViewItems(HWND hList, int iItem1, int iItem2)
{
    LVITEM lvi1;
    lvi1.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
    lvi1.stateMask = (UINT)-1;
    lvi1.iItem = iItem1;
    lvi1.iSubItem = 0;
    BOOL bRes1 = ListView_GetItem(hList, &lvi1);
    
    LVITEM lvi2;
    lvi2.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE;
    lvi2.stateMask = (UINT)-1;
    lvi2.iItem = iItem2;
    lvi2.iSubItem = 0;
    BOOL bRes2 = ListView_GetItem(hList, &lvi2);

    if (bRes1 && bRes2)
    {
        lvi1.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM  | LVIF_STATE;
        lvi1.stateMask = (UINT)-1;

        lvi2.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM  | LVIF_STATE;
        lvi2.stateMask = (UINT)-1;
        
        // swap the items
        lvi1.iItem = iItem2;
        ListView_SetItem(hList, &lvi1);

        lvi2.iItem = iItem1;
        ListView_SetItem(hList, &lvi2);

    }
}

// Move up the selected item
void MoveSelectionUp(HWND hList)
{
    int iSel = ListView_GetSelectedItem(hList);
    SwapListViewItems(hList, iSel, iSel - 1);
}

// Move down the selected item
void MoveSelectionDown(HWND hList)
{
    int iSel = ListView_GetSelectedItem(hList);
    SwapListViewItems(hList, iSel, iSel + 1);
}


void DeleteItem(HWND hList, int iItem)
{
    LV_ITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.lParam = NULL;
    
    if( ListView_GetItem(hList, &lvi) && lvi.lParam )
    {
        delete(((ColumnCriteria *)lvi.lParam));
    }
    ListView_DeleteItem(hList, iItem);
}
void DeleteListItems(HWND hList)
{
    int iCnt = ListView_GetItemCount(hList);
    for(int iItem=0; iItem<iCnt; iItem++) {
        DeleteItem(hList, iItem);
    }
    ListView_DeleteAllItems(hList);
}

// Save the criterias set by the user in the the dialog box
void SaveListViewCriterias(HWND hList)
{
    int iCnt = ListView_GetItemCount(hList);
    criterias.clear();
    for(int iItem=0; iItem<iCnt; iItem++) {

        LV_ITEM lvi;
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iItem;
        lvi.iSubItem = 0;
        lvi.lParam = NULL;
        
        if( ListView_GetItem(hList, &lvi) && lvi.lParam )
        {
            criterias.push_back(*((ColumnCriteria *)lvi.lParam));
        }
    }
}

// Save the criterias set by the user in the the dialog box
void LoadCriteriasIntoListView(HWND hList)
{
    DeleteListItems(hList);
    int iItem = 0;
    for(std::vector<ColumnCriteria>::iterator it = criterias.begin();
        it != criterias.end();
        it++)
    {
        LV_ITEM lvi;
        lvi.pszText = (PTSTR)LPSTR_TEXTCALLBACK;
        lvi.mask = LVIF_PARAM | LVIF_TEXT;
        lvi.iItem = iItem++;
        lvi.iSubItem = 0;
        lvi.lParam = (LPARAM)new ColumnCriteria(*it);
        ListView_InsertItem(hList, &lvi);
    }
}

// Load info from a listview item into the upper fields of 
// the dialog box.
void FillEditFieldsFromLVItem( HWND hDlg, HWND hList, int iItem )
{
    LVITEM lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_PARAM;
    ListView_GetItem(hList, &lvi);
    if( lvi.lParam )
    {
        ColumnCriteria *crit = (ColumnCriteria *)lvi.lParam;
        SetDlgItemInt(hDlg, IDC_FROM, crit->from, FALSE);
        SetDlgItemInt(hDlg, IDC_LENGTH, crit->length, FALSE);
        switch( crit->casesensitivy )
        {
        case CaseSensitive:
            CheckRadioButton(hDlg, IDC_SENSITIVE, IDC_NUMERIC, IDC_SENSITIVE);
            break;
        case CaseInsensitive:
            CheckRadioButton(hDlg, IDC_SENSITIVE, IDC_NUMERIC, IDC_INSENSITIVE);
            break;
        case Numeric:
            CheckRadioButton(hDlg, IDC_SENSITIVE, IDC_NUMERIC, IDC_NUMERIC);
            break;
        }
        if( crit->order == Ascending )
        {
            CheckRadioButton(hDlg, IDC_ASCENDING, IDC_DESCENDING, IDC_ASCENDING);
        }
        else
        {
            CheckRadioButton(hDlg, IDC_ASCENDING, IDC_DESCENDING, IDC_DESCENDING);
        }
    }
}

// Fill a ColumnCriteria structure using information from the upper edit fields in the dialog box
void ColumnCriteriaFromEditFields(ColumnCriteria *cc, HWND hwndDlg)
{
    BOOL bOk;
    cc->from = GetDlgItemInt(hwndDlg, IDC_FROM, &bOk, FALSE);
    cc->length = GetDlgItemInt(hwndDlg, IDC_LENGTH, &bOk, FALSE);
    cc->order = IsDlgButtonChecked(hwndDlg, IDC_ASCENDING) ? Ascending : Descending;
    cc->casesensitivy = IsDlgButtonChecked(hwndDlg, IDC_SENSITIVE) ? CaseSensitive : 
                IsDlgButtonChecked(hwndDlg, IDC_INSENSITIVE) ? CaseInsensitive : Numeric;
}

// Converse of FillEditFieldsFromLVItem
void UpdateItemFromEditFields(HWND hDlg, HWND hList, int iItem)
{
    LV_ITEM lvi;
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    if( ListView_GetItem(hList, &lvi) && lvi.lParam )
    {
        ColumnCriteriaFromEditFields((ColumnCriteria *)lvi.lParam, hDlg);
        ListView_RedrawItems(hList, iItem, iItem);
    }
}


INT_PTR CALLBACK ColumnsDialogProc(
  HWND hwndDlg, 
  UINT uMsg, 
  WPARAM wParam, 
  LPARAM lParam
)
{
    static PTSTR pszAscending = NULL;
    static PTSTR pszDescending = NULL;
    static PTSTR pszCaseSensitive = NULL;
    static PTSTR pszCaseInsensitive = NULL;
    static PTSTR pszNumeric = NULL;

    static HWND hList = NULL;

    switch(uMsg)
    {
    case WM_DESTROY:
        _ASSERT(hwndDlg);
        _ASSERT(hList);

        if( pszAscending ) free(pszAscending);
        if( pszDescending ) free(pszDescending);
        if( pszCaseSensitive ) free(pszCaseSensitive);
        if( pszCaseInsensitive ) free(pszCaseInsensitive);
        if( pszNumeric ) free(pszNumeric);
        if( hList ) DeleteListItems(hList);
        hList = NULL;
        
        return true;

    case WM_INITDIALOG:
        {
            //
            // Load string from resources
            //
            pszAscending = LoadStringFromDlg(hwndDlg, IDC_ASCENDING);
            pszDescending = LoadStringFromDlg(hwndDlg, IDC_DESCENDING);
            pszCaseSensitive = LoadStringFromDlg(hwndDlg, IDC_SENSITIVE);
            pszCaseInsensitive = LoadStringFromDlg(hwndDlg, IDC_INSENSITIVE);
            pszNumeric = LoadStringFromDlg(hwndDlg, IDC_NUMERIC);
            
            
            //
            // Configure the listview control
            //
            hList = GetDlgItem(hwndDlg, IDC_LISTCOLUMNS);
            LONG oldStyle = GetWindowLong(hList, GWL_STYLE);
            SetWindowLong(hList, GWL_STYLE, oldStyle | LVS_SHOWSELALWAYS);
            ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

            //
            // Create columns in listview control
            //
            
            LVCOLUMN c;
            c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM | LVCF_ORDER;
            c.fmt = LVCFMT_LEFT;

            c.iSubItem = c.iOrder = 0;
            c.cx = 70;
            c.pszText = LoadResString(g_hModule, IDS_COLFROM);
            ListView_InsertColumn(hList, c.iSubItem, &c);
            FreeResString(c.pszText);

            c.iSubItem = ++c.iOrder;
            c.cx = 60;
            c.pszText = LoadResString(g_hModule, IDS_COLLENGTH);
            ListView_InsertColumn(hList, c.iSubItem, &c);
            FreeResString(c.pszText);

            c.iSubItem = ++c.iOrder;
            c.cx = 100;
            c.pszText = LoadResString(g_hModule, IDS_COLORDER);
            ListView_InsertColumn(hList, c.iSubItem, &c);
            FreeResString(c.pszText);

            c.iSubItem = ++c.iOrder;
            c.cx = 100;
            c.pszText = LoadResString(g_hModule, IDS_COLCASESENS);
            ListView_InsertColumn(hList, c.iSubItem, &c);
            FreeResString(c.pszText);
            
            //
            // Fill the upper fields with default values
            //
            SetDlgItemInt(hwndDlg, IDC_FROM, 1, FALSE);
            SetDlgItemInt(hwndDlg, IDC_LENGTH, 5, FALSE);
            CheckDlgButton(hwndDlg, IDC_ASCENDING, TRUE);
            CheckDlgButton(hwndDlg, IDC_SENSITIVE, TRUE);
            
            //
            // Load the criterias into the list view
            //
            LoadCriteriasIntoListView(hList);
        }
        return TRUE;

    case WM_COMMAND:
        _ASSERT(hwndDlg);
        switch(wParam)
        {
            case IDCANCEL:
                EndDialog(hwndDlg, 1);
                return TRUE;
            case IDOK:
                SaveListViewCriterias(hList);
                EndDialog(hwndDlg, 0);
                return TRUE;
            case IDC_SORT:
                SaveListViewCriterias(hList);
                SortCommand();
                return TRUE;
            case IDC_ADD:
                {
                    HWND hList = GetDlgItem(hwndDlg, IDC_LISTCOLUMNS);

                    ColumnCriteria *cc = new ColumnCriteria();
                    ColumnCriteriaFromEditFields(cc, hwndDlg);

                    LVITEM item;
                    item.iItem = ListView_GetItemCount(hList);;
                    item.iSubItem = 0;
                    item.lParam = (LPARAM)cc;
                    item.pszText = (PTSTR)LPSTR_TEXTCALLBACK;
                    item.mask = LVIF_PARAM | LVIF_TEXT;
                    int iItem = ListView_InsertItem(hList, &item);
                    SetFocus(GetDlgItem(hwndDlg, IDC_FROM));
                }
                break;
            case IDC_DELETE:
                {
                    int sel = ListView_GetSelectedItem(hList);
                    if(sel!=-1)
                    {
                        DeleteItem(hList, sel);
                    }
                    SetFocus(hList);
                    return TRUE;
                }
            case IDC_UP:
                MoveSelectionUp(hList);
                SetFocus(hList);
                return TRUE;
            case IDC_DOWN:
                MoveSelectionDown(hList);
                SetFocus(hList);
                return TRUE;
            case IDC_UPDATE:
                UpdateItemFromEditFields(hwndDlg, hList, ListView_GetSelectedItem(hList));
                return TRUE;
        }
        break;

    case WM_NOTIFY:
        _ASSERT(lParam);

        switch (((LPNMHDR)lParam)->code)
        {
            case LVN_ITEMCHANGED:
                if ((((LPNMLISTVIEW)lParam)->uChanged & LVIF_STATE)
                 && (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED))
                        FillEditFieldsFromLVItem(hwndDlg, hList, ((LPNMLISTVIEW)lParam)->iItem);
                return TRUE;
            case LVN_GETDISPINFO:
            {
                // Provide the item or subitem's text, if requested.
                NMLVDISPINFO *pnmv =  (NMLVDISPINFO *)lParam;
                if (pnmv->item.mask & LVIF_TEXT)
                {
                    ColumnCriteria *cc = (ColumnCriteria *)pnmv->item.lParam;
                    switch( pnmv->item.iSubItem ) {
                    case 0:
                        _itow_s(cc->from, pnmv->item.pszText, pnmv->item.cchTextMax, 10);
                        break;
                    case 1:
                        _itow_s(cc->length, pnmv->item.pszText, pnmv->item.cchTextMax, 10);
                        break;
                    case 2:                         
                        wcscpy(pnmv->item.pszText,
                            cc->order == Ascending ? pszAscending : pszDescending);
                        break;
                    case 3:
                        wcscpy(pnmv->item.pszText,
                            cc->casesensitivy == CaseSensitive ? pszCaseSensitive : 
                            cc->casesensitivy  == CaseInsensitive ? pszCaseInsensitive : pszNumeric);
                        break;
                    }
                    break;
                }
                return TRUE;
            }           

            default:
                break;
        }
        break;

        return FALSE;

    default:
        return FALSE;
    }
    return FALSE;
}

