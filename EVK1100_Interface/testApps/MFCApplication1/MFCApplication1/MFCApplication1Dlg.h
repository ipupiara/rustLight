
// MFCApplication1Dlg.h : header file
//

#pragma once

#include "rustlightUdpClient.h"


// mfc_if_Dlg dialog
class mfc_if_Dlg : public CDialogEx
{
// Construction
public:
	mfc_if_Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	RustlightUdpClient* rustlightUpdClient;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	const wchar_t *mfc_if_Dlg::GetWC(const char *c);
	afx_msg void OnBnClickedSend();
	UINT triacAddress;
	UINT triacValue;
	afx_msg void OnBnClickedCheckenabled();
	BOOL triacEnabled;
	CString logText;
	static void logT(const char *emsg, ...);
	void addLogText(const char *emsg, ...);
	static mfc_if_Dlg* singleton;
};
