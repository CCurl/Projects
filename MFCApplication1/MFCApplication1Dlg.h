
// MFCApplication1Dlg.h : header file
//

#pragma once
#include "Critter.h"


// CMFCApplication1Dlg dialog
class CMFCApplication1Dlg : public CDialogEx
{
// Construction
public:
	CMFCApplication1Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

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
	void PaintCritter(CDC* dc, Critter* p, int oldX, int oldY);
	afx_msg void OnBnClickedGo();
	void InitWorld();
	void OneStep(CDC* dc);
	CStatic mfcWorld;
	Brain brain;
	int maxX, maxY;
};