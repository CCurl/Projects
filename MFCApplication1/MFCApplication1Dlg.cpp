
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
	, m_numCritters(0)
	, m_numHidden(0)
	, m_numConnections(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	maxX = 0;
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WORLD, mfcWorld);
	DDX_Text(pDX, IDC_NUMCRITTERS, m_numCritters);
	DDX_Text(pDX, IDC_NUMHIDDEN, m_numHidden);
	DDX_Text(pDX, IDC_NUMCONNECTIONS, m_numConnections);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GO, &CMFCApplication1Dlg::OnBnClickedGo)
	ON_BN_CLICKED(IDC_InitWorld, &CMFCApplication1Dlg::OnBnClickedInitworld)
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
	m_numHidden = 2;
	m_numConnections = 10;
	m_numCritters = 100;
	UpdateData(0);
	InitWorld();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication1Dlg::InitWorld() {
	Critter::numCritters = m_numCritters;
	brain.Init(m_numHidden, m_numConnections);
	maxX = 128;
	maxY = 128;
	for (int i = 0; i < Critter::numCritters; i++) {
		int x = rand() % maxX;
		int y = rand() % maxY;
		Critter* pC = Critter::At(i);
		pC->CreateRandom(x, y, &brain);
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
	int sz = 4;
	int x = p->x * sz;
	int y = p->y * sz;
	for (int xi = 0; xi < sz; xi++) {
		for (int yi = 0; yi < sz; yi++) {
			dc->SetPixel(x + xi, y + yi, RGB(0, 0, 0));
		}
	}
}

void CMFCApplication1Dlg::OneStep() {
	TRACE("OneStep\n");
	for (int i = 0; i < Critter::numCritters; i++) {
		brain.OneStep(Critter::At(i));
	}
}

void CMFCApplication1Dlg::PaintCritters(CDC* dc) {
	COLORREF c = RGB(255, 255, 255);
	dc->FillSolidRect(0, 0, 516, 516, c);
	for (int i = 0; i < Critter::numCritters; i++) {
		// Critter::At(i)->x = rand() % 128; // TEMP
		// Critter::At(i)->y = rand() % 128; // TEMP
		PaintCritter(dc, Critter::At(i));
	}
}

void CMFCApplication1Dlg::OnBnClickedGo() {
	UpdateData();

	CDC* dc = mfcWorld.GetDC();
	HGDIOBJ prevSel = dc->SelectObject(mfcWorld.GetBitmap());

	for (int i = 0; i < 100; i++) {
		OneStep();
		PaintCritters(dc);
	}

	PaintCritters(dc);
	dc->SelectObject(prevSel);
}


void CMFCApplication1Dlg::OnBnClickedInitworld()
{
	UpdateData();
	InitWorld();
}
