
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
	, m_numSteps(0)
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
	DDX_Text(pDX, IDC_NUMSTEPS, m_numSteps);
	DDX_Check(pDX, IDC_CHECK1, m_isReset);
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
	m_numSteps = 100;
	m_isReset = TRUE;
	UpdateData(0);
	InitWorld();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication1Dlg::InitWorld() {
	World* w = TheWorld();
	Critter::numCritters = m_numCritters;
	brain.Init(m_numHidden, m_numConnections);
	w->Init();
	maxX = 128;
	maxY = 128;
	w->SetSize(maxX, maxY);
	for (int i = 1; i <= Critter::numCritters; i++) {
		int x = RAND(maxX);
		int y = RAND(maxY);
		while (w->EntityAt(x, y)) {
			x = RAND(maxX);
			y = RAND(maxY);
		}
		Critter* pC = Critter::At(i);
		pC->CreateRandom(i, x, y, &brain);
		w->SetEntityAt(x, y, i);
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

void PaintBlock(CDC* dc, int x, int y, COLORREF c) {
	int sz = 4;
	x *= sz;
	y *= sz;
	for (int xi = 0; xi < sz; xi++) {
		for (int yi = 0; yi < sz; yi++) {
			dc->SetPixel(x + xi, y + yi, c);
		}
	}
}

void CMFCApplication1Dlg::PaintCritter(CDC* dc, Critter* p) {
	if ((p->x != p->lX) && (p->y != p->lY)  ) {
		PaintBlock(dc, p->lX, p->lY, 0xFFFFFF);
		PaintBlock(dc, p->x, p->y, 0x000000);
		p->RememberLoc();
	}
}

void CMFCApplication1Dlg::OneStep() {
	TRACE("OneStep\n");
	for (int i = 1; i <= Critter::numCritters; i++) {
		brain.OneStep(Critter::At(i));
	}
}

void CMFCApplication1Dlg::PaintCritters(bool ClearFirst) {
	CDC* dc = mfcWorld.GetDC();
	HGDIOBJ prevSel = dc->SelectObject(mfcWorld.GetBitmap());

	if (ClearFirst) {
		COLORREF c = RGB(255, 255, 255);
		dc->FillSolidRect(0, 0, 516, 516, c);
	}

	for (int i = 1; i <= Critter::numCritters; i++) {
		PaintCritter(dc, Critter::At(i));
	}

	dc->SelectObject(prevSel);
}

void CMFCApplication1Dlg::OnBnClickedGo() {
	UpdateData();
	if (m_isReset) {
		OnBnClickedInitworld();
	}

	for (int i = 1; i <= m_numSteps; i++) {
		OneStep();
		if ((i % 4) == 0) {
			PaintCritters(false);
			Sleep(66);
		}
	}

	PaintCritters(false);
}

void CMFCApplication1Dlg::OnBnClickedInitworld()
{
	UpdateData();
	InitWorld();
	PaintCritters(true);
}
