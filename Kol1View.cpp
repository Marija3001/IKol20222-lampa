// Kol1View.cpp : implementation of the CKol1View class  (I kolokvijum 2022 - Lampa)
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Kol1.h"
#endif

#include "Kol1Doc.h"
#include "Kol1View.h"
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define STEP_ANGLE 10.0   // 11. korak rotacije

// Tacka na base.png gde se kaci ruka (centar kruga na postolju), u
// koordinatama slike. Nije data u zadatku - PROVERITI na slici base.png.
#define BASE_ARM_X 80.0f
#define BASE_ARM_Y 20.0f

// Polozaj lampe na stolu: donja sredina postolja, relativno u odnosu na
// donji desni deo prozora (procena sa slike iz zadatka).
#define LAMP_X_FROM_CENTER 150.0f
#define LAMP_Y_FROM_BOTTOM 160.0f

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 1. Ucitavanje slika u konstruktoru, dealokacija u destruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pBase = new DImage();        m_pBase->Load(CString("base.png"));
	m_pArm1 = new DImage();        m_pArm1->Load(CString("arm1.png"));
	m_pArm2 = new DImage();        m_pArm2->Load(CString("arm2.png"));
	m_pHead = new DImage();        m_pHead->Load(CString("head.png"));
	m_pPozadina = new DImage();    m_pPozadina->Load(CString("pozadina.jpg"));
	m_pBaseShadow = new DImage();  m_pBaseShadow->Load(CString("base_shadow.png"));
	m_pArm1Shadow = new DImage();  m_pArm1Shadow->Load(CString("arm1_shadow.png"));
	m_pArm2Shadow = new DImage();  m_pArm2Shadow->Load(CString("arm2_shadow.png"));
	m_pHeadShadow = new DImage();  m_pHeadShadow->Load(CString("head_shadow.png"));

	// pocetni polozaj kao na slici: arm1 ide gore-desno, arm2 gore-levo
	m_angleArm1 = -45.0;
	m_angleArm2 = -90.0;
	m_angleHead = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pBase;
	delete m_pArm1;
	delete m_pArm2;
	delete m_pHead;
	delete m_pPozadina;
	delete m_pBaseShadow;
	delete m_pArm1Shadow;
	delete m_pArm2Shadow;
	delete m_pHeadShadow;
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 2. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY  (lokalna, u odnosu na tekuci k.s.)
//    rightMultiply = true  -> MWT_RIGHTMULTIPLY (globalna)
// ---------------------------------------------------------------
void CKol1View::Translate(CDC* pDC, float dX, float dY, bool rightMultiply)
{
	XFORM form = { 1.0f, 0.0f, 0.0f, 1.0f, dX, dY };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * M_PI / 180.0);
	XFORM form = { cosf(rad), sinf(rad), -sinf(rad), cosf(rad), 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Scale(CDC* pDC, float sX, float sY, bool rightMultiply)
{
	XFORM form = { sX, 0.0f, 0.0f, sY, 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

// ---------------------------------------------------------------
// 3. Pozadina: centrirana po X, donja ivica = donja ivica prozora
// ---------------------------------------------------------------
void CKol1View::DrawBackground(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int w = m_pPozadina->Width();
	int h = m_pPozadina->Height();

	int x = (rcClient.Width() - w) / 2;
	int y = rcClient.Height() - h;

	m_pPozadina->Draw(pDC, CRect(0, 0, w, h), CRect(x, y, x + w, y + h));
}

// ---------------------------------------------------------------
// 4. Transparentno crtanje - transparentna je boja prvog piksela
// ---------------------------------------------------------------
void CKol1View::DrawImgTransparent(CDC* pDC, DImage* pImage)
{
	unsigned char* bits = pImage->GetDIBBits();
	COLORREF bgColor = RGB(bits[2], bits[1], bits[0]);   // u memoriji je BGR(A)

	CRect rc(0, 0, pImage->Width(), pImage->Height());
	pImage->DrawTransparent(pDC, rc, rc, bgColor);
}

// ---------------------------------------------------------------
// 5. Postolje. Ulaz: k.s. u donjoj sredini postolja (tacka na stolu).
//    Izlaz: k.s. u tacki kacenja ruke - ne vraca se transformacija.
// ---------------------------------------------------------------
void CKol1View::DrawLampBase(CDC* pDC, bool bIsShadow)
{
	DImage* pImg = bIsShadow ? m_pBaseShadow : m_pBase;

	Translate(pDC, -pImg->Width() / 2.0f, -(float)pImg->Height(), false);
	DrawImgTransparent(pDC, pImg);
	Translate(pDC, BASE_ARM_X, BASE_ARM_Y, false);
}

// ---------------------------------------------------------------
// 6. arm1: rotacija oko veceg kruga (58,61);
//    na kraju k.s. ostaje u centru manjeg kruga (309,61)
// ---------------------------------------------------------------
void CKol1View::DrawLampArm1(CDC* pDC, bool bIsShadow)
{
	DImage* pImg = bIsShadow ? m_pArm1Shadow : m_pArm1;

	Rotate(pDC, (float)m_angleArm1, false);
	Translate(pDC, -58.0f, -61.0f, false);
	DrawImgTransparent(pDC, pImg);
	Translate(pDC, 309.0f, 61.0f, false);
}

// ---------------------------------------------------------------
// 7. arm2: rotacija oko veceg kruga (36,40); manji krug (272,40).
//    Natpis 'GDI' je na kraju arm2 koji ulazi u glavu, pa se glava
//    crta PRE arm2 (arm2 ide preko glave i natpis ostaje vidljiv).
// ---------------------------------------------------------------
void CKol1View::DrawLampArm2(CDC* pDC, bool bIsShadow)
{
	DImage* pImg = bIsShadow ? m_pArm2Shadow : m_pArm2;

	Rotate(pDC, (float)m_angleArm2, false);
	Translate(pDC, -36.0f, -40.0f, false);   // k.s. = gornji levi ugao arm2

	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);
	Translate(pDC, 272.0f, 40.0f, false);    // manji krug -> zglob glave
	DrawLampHead(pDC, bIsShadow);            // prvo glava
	pDC->SetWorldTransform(&oldForm);

	DrawImgTransparent(pDC, pImg);           // pa arm2 preko nje
	Translate(pDC, 272.0f, 40.0f, false);
}

// ---------------------------------------------------------------
// 8. Glava: rotacija oko (178,100)
// ---------------------------------------------------------------
void CKol1View::DrawLampHead(CDC* pDC, bool bIsShadow)
{
	DImage* pImg = bIsShadow ? m_pHeadShadow : m_pHead;

	Rotate(pDC, (float)m_angleHead, false);
	Translate(pDC, -178.0f, -100.0f, false);
	DrawImgTransparent(pDC, pImg);
}

// ---------------------------------------------------------------
// 9. Cela lampa (glavu crta DrawLampArm2 zbog redosleda)
// ---------------------------------------------------------------
void CKol1View::DrawLamp(CDC* pDC, bool bIsShadow)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	DrawLampBase(pDC, bIsShadow);   // k.s. -> zglob postolje/arm1
	DrawLampArm1(pDC, bIsShadow);   // k.s. -> zglob arm1/arm2
	DrawLampArm2(pDC, bIsShadow);   // crta glavu pa arm2

	pDC->SetWorldTransform(&oldForm);
}

void CKol1View::MoveToLampOrigin(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	Translate(pDC,
		rcClient.Width() / 2.0f + LAMP_X_FROM_CENTER,
		rcClient.Height() - LAMP_Y_FROM_BOTTOM, false);
}

// ---------------------------------------------------------------
// 10. Senka: lampa se "obori" ulevo (rotacija -90 oko donje sredine
//     postolja) i spljosti po Y (skaliranje 1, 0.25).
//     Redosled poziva (LEFTMULTIPLY): T, S, R -> na tacku se prvo
//     primenjuje R, pa S, pa T.
// ---------------------------------------------------------------
void CKol1View::DrawLampShadow(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	MoveToLampOrigin(pDC);
	Scale(pDC, 1.0f, 0.25f, false);
	Rotate(pDC, -90.0f, false);
	DrawLamp(pDC, true);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 12. Scena bez flicker-a (memorijski DC)
// ---------------------------------------------------------------
BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;   // pozadinu brise memorijski DC, ne Windows
}

void CKol1View::OnDraw(CDC* pDC)
{
	CKol1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rcClient;
	GetClientRect(&rcClient);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));

	int oldMode = memDC.SetGraphicsMode(GM_ADVANCED);
	XFORM oldForm;
	memDC.GetWorldTransform(&oldForm);

	DrawBackground(&memDC);
	DrawLampShadow(&memDC);   // senka ispod lampe

	MoveToLampOrigin(&memDC);
	DrawLamp(&memDC, false);

	memDC.SetWorldTransform(&oldForm);
	memDC.SetGraphicsMode(oldMode);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();
}

// ---------------------------------------------------------------
// 11. Tasteri: 1/2 arm1, 3/4 arm2, 5/6 glava - po 10 stepeni
// ---------------------------------------------------------------
void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case '1': case VK_NUMPAD1: m_angleArm1 -= STEP_ANGLE; break;
	case '2': case VK_NUMPAD2: m_angleArm1 += STEP_ANGLE; break;
	case '3': case VK_NUMPAD3: m_angleArm2 -= STEP_ANGLE; break;
	case '4': case VK_NUMPAD4: m_angleArm2 += STEP_ANGLE; break;
	case '5': case VK_NUMPAD5: m_angleHead -= STEP_ANGLE; break;
	case '6': case VK_NUMPAD6: m_angleHead += STEP_ANGLE; break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
