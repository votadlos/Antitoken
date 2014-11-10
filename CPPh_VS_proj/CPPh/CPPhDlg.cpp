
// CPPhDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CPPh.h"
#include "CPPhDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCPPhDlg dialog
const TCHAR *STATIC_TEXT_1 = _T("Введите pin-код для контейнера");
const TCHAR *STATIC_TEXT_2 = _T("pin-код должен быть не менее 4 символов.");
const TCHAR *STATIC_TEXT_3 = _T("Неправильный pin-код. Осталось попыток: ");

CCPPhDlg::CCPPhDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCPPhDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CCPPhDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIMER, mTimer);
	DDX_Control(pDX, IDC_LANG, mLang);
	DDX_Control(pDX, IDC_MSG, mMsg);
	DDX_Control(pDX, IDC_EDIT1, mPw);
}

BEGIN_MESSAGE_MAP(CCPPhDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK1, &CCPPhDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDOK, &CCPPhDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &CCPPhDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
//	ON_WM_INPUTLANGCHANGEREQUEST(IDC_EDIT1, &CCPPhDlg::OnInputLangChangeRequest)
	ON_WM_INPUTLANGCHANGEREQUEST()
	ON_WM_INPUTLANGCHANGE()
//	ON_EN_CHANGE(IDC_EDIT1, &CCPPhDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CCPPhDlg message handlers

BOOL CCPPhDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	this->flog.open(_T("cpph.log")); //open log

	SetTimer(1, 1000, 0); //timer to show minutes to close

	//established token
	this->established_token_status = this->token.establish_token();
	TCHAR msg_char[200];
	if (this->established_token_status == 0){
		this->token.list_containers(this->token_containeres);
		if (this->cur_container_name != this->token_containeres[this->cur_container_id]){ //if not displayed, show it
			wsprintf(msg_char, _T("%s \"%S\""), STATIC_TEXT_1, this->token_containeres[this->cur_container_id].c_str());
			mMsg.SetWindowTextW(msg_char);
			this->cur_container_name = this->token_containeres[this->cur_container_id];
			this->flog << this->loctime() << ": Established container: " << this->cur_container_name << endl; //log
		}
	}
	
	TCHAR l[100];
	GetKeyboardLayoutName(l);
	this->flog << this->loctime() << ": Current leyboard layout: " << CT2A(l) << endl;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCPPhDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCPPhDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCPPhDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
}


void CCPPhDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	TCHAR *pw;
	int pw_len = mPw.GetWindowTextLengthW();

	pw = new TCHAR[pw_len + 1];
	memset(pw, 0, pw_len + 1);

	mPw.GetWindowTextW(pw, pw_len+1);

	this->token.setUpass(CT2A(pw));

	int retcode;
	if ((retcode = this->token.et72k_Login()) == 0){
		this->flog << loctime() << ": container: " << this->cur_container_name << ", password: " << CT2A(pw) << " - password is OK!"<<endl;

		//go to next container
		if (++this->cur_container_id < this->token_containeres.size()){
			TCHAR msg_char[200];
			if (this->cur_container_name != this->token_containeres[this->cur_container_id]){ //if not displayed, show it
				wsprintf(msg_char, _T("%s \"%S\""), STATIC_TEXT_1, this->token_containeres[this->cur_container_id].c_str());
				mMsg.SetWindowTextW(msg_char);
				this->cur_container_name = this->token_containeres[this->cur_container_id];
			}
		}
		else { //no more containers - exit
			this->flog << loctime() << ": No more containers - Exit" << endl;
			delete[] pw;
			CDialogEx::OnOK();
		}

	}
	else if (retcode == 21){
		this->flog << loctime() << ": container: " << this->cur_container_name << ", password: " << CT2A(pw) << " - password is too short" << endl;
		TCHAR msg_char[200];
		wsprintf(msg_char, _T("%s"), STATIC_TEXT_2);
		mMsg.SetWindowTextW(msg_char);
	}
	else {
		this->flog << loctime() << ": container: " << this->cur_container_name << ", password: " << CT2A(pw) << " - password is WRONG!" << endl;
		TCHAR msg_char[200];
		wsprintf(msg_char, _T("%s %d."), STATIC_TEXT_3, --this->pw_counter);
		if (this->pw_counter < 0){ //token locked - exit
			this->flog << loctime() << "Token is locked" << endl;
			delete[] pw;
			CDialogEx::OnOK();
		}
		mMsg.SetWindowTextW(msg_char);
	}

	//CDialogEx::OnOK();
}


void CCPPhDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	//update timer
	TCHAR tmp[100];
	if (--this->timer < 0){
		this->flog << loctime() << ": Timeout for dialog has passed - will EndDialog()" << endl;
		this->EndDialog(10);

		return;
	}
	else {
		wsprintf(tmp, _T("%0d:%02d:%02d"), this->timer / 3600, this->timer / 60, this->timer % 60);
		mTimer.SetWindowTextW(tmp);
	}

	//established token
	if (this->established_token_status != 0){ //didn't list containers
		this->established_token_status = this->token.establish_token();
		TCHAR msg_char[200];
		if (this->established_token_status == 0){
			this->token.list_containers(this->token_containeres);
			if (this->cur_container_name != this->token_containeres[this->cur_container_id]){ //if not displayed, show it
				wsprintf(msg_char, _T("%s \"%S\""), STATIC_TEXT_1, this->token_containeres[this->cur_container_id].c_str());
				mMsg.SetWindowTextW(msg_char);
				this->cur_container_name = this->token_containeres[this->cur_container_id];
				this->flog << this->loctime() << " " << ": Established container: " << this->cur_container_name << endl; //log
			}
		}
		else {
			//wsprintf(msg_char, _T("Ошибка получения имени контейнера: %d"), this->established_token_status);
			//wsprintf(msg_char, _T("%s"), this->token.reader);
			//mMsg.SetWindowTextW(msg_char);
			this->flog << loctime() << ": Establish token error: " << this->established_token_status << endl;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CCPPhDlg::OnInputLangChange(UINT nCharSet, UINT nLocaleId)
{
	// TODO: Add your message handler code here and/or call default
	this->flog << loctime() << ": OnInputLangChange: " << nCharSet << ", " << nLocaleId << endl;

	TCHAR l[100];
	GetKeyboardLayoutName(l);
	this->flog << this->loctime() << ": Current leyboard layout (OnInputLangChange): " << CT2A(l) << endl;

	CDialogEx::OnInputLangChange(nCharSet, nLocaleId);
}

void CCPPhDlg::OnInputLangChangeRequest(UINT nFlags, UINT nLocaleId)
{
	// TODO: Add your message handler code here and/or call default
	this->flog << loctime() << ": OnInputLangChangeRequest: " << nFlags << ", " << nLocaleId << endl;

	TCHAR l[100];
	GetKeyboardLayoutName(l);
	this->flog << this->loctime() << ": Current leyboard layout (OnInputLangChangeRequest): " << CT2A(l) << endl;

	CDialogEx::OnInputLangChangeRequest(nFlags, nLocaleId);
}

void CCPPhDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	this->flog << loctime() << ": Cancel button was pressed" << endl;
	CDialogEx::OnCancel();
}

void CCPPhDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->flog << loctime() << ": Close dialog window" << endl;
	flog.close(); //close log
	CDialogEx::OnClose();
}

char *CCPPhDlg::loctime(){
	time_t rawtime;
	struct tm *t;

	memset(this->mtm, 0, MTM_LEN);

	time(&rawtime);
	t = localtime(&rawtime);
	strftime(this->mtm, MTM_LEN, "%Y%m%d-%H:%M:%S-%z", t);

	return this->mtm;
}



