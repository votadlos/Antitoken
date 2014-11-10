
// CPPhDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Token.h"

#define MTM_LEN 100

// CCPPhDlg dialog
class CCPPhDlg : public CDialogEx
{
private:
	std::vector<std::string> token_containeres;
	Token token;
	BYTE cur_container_id = 0;
	int pw_counter = 12;
	string cur_container_name = "";
	LONG established_token_status = -1;
	std::ofstream  flog;
	char mtm[MTM_LEN];

	char *loctime();

// Construction
public:
	CCPPhDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CPPH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedOk();
	int timer = 600;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	// Timer for window to appear
	CStatic mTimer;
	// Input language
	CStatic mLang;
	afx_msg void OnInputLangChange(UINT nCharSet, UINT nLocaleId);
	// Message
	CStatic mMsg;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg void OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId);
	// password input text field
	CEdit mPw;
};
