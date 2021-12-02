
// MFCApplication1Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCApplication1Dlg dialog

CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WORLD, mfcWorld);
	DDX_Control(pDX, IDC_LIST1, m_world);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GO, &CMFCApplication1Dlg::OnBnClickedGo)
END_MESSAGE_MAP()


// CMFCApplication1Dlg message handlers

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	InitWorld();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication1Dlg::InitWorld() {
	Critter::numCritters = 300;
	brain.Init(5, 2, 5, 10);
	maxX = 128;
	maxY = 128;
	for (int i = 0; i < Critter::numCritters; i++) {
		int x = rand() % maxX;
		int y = rand() % maxY;
		Critter* pC = Critter::At(i);
		pC->CreateRandom(x, y, &brain);
	}
	CString x = CString('x', 128);
	for (int i = 0; i < 128; i++) {
		m_world.AddString(x);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication1Dlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
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
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::PaintCritter(CDC* dc, Critter *p) {
	int x = p->x * 2;
	int y = p->y * 2;
	dc->SetPixel(x+0, y+0, RGB(0,0,0));
	dc->SetPixel(x+0, y+1, RGB(0,0,0));
	dc->SetPixel(x+1, y+0, RGB(0,0,0));
	dc->SetPixel(x+1, y+1, RGB(0,0,0));
}

void CMFCApplication1Dlg::OneStep() {
	for (int i = 0; i < Critter::numCritters; i++) {
		brain.OneStep(Critter::At(i));
	}
}

void CMFCApplication1Dlg::OnBnClickedGo()
{
	for (int i = 0; i < 10; i++) {
		OneStep();
	}

	CDC *dc = mfcWorld.GetDC();
	HGDIOBJ prevSel = dc->SelectObject(mfcWorld.GetBitmap());
	COLORREF c = RGB(255, 255, 255);
	dc->FillSolidRect(0, 0, 300, 300, c);
	for (int i = 0; i < Critter::numCritters; i++) {
		PaintCritter(dc, Critter::At(i));
	}

	dc->SelectObject(prevSel);
}
