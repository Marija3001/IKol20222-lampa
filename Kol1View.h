// Kol1View.h : interface of the CKol1View class  (I kolokvijum 2022 - Lampa)
#pragma once
#include "DImage.h"

class CKol1View : public CView
{
protected:
	CKol1View() noexcept;
	DECLARE_DYNCREATE(CKol1View)

public:
	CKol1Doc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CKol1View();

protected:
	// 1. slike
	DImage* m_pBase;
	DImage* m_pArm1;
	DImage* m_pArm2;
	DImage* m_pHead;
	DImage* m_pPozadina;
	DImage* m_pBaseShadow;
	DImage* m_pArm1Shadow;
	DImage* m_pArm2Shadow;
	DImage* m_pHeadShadow;

	// 11. uglovi (u stepenima) - atributi View klase
	double m_angleArm1;   // 1 / 2
	double m_angleArm2;   // 3 / 4
	double m_angleHead;   // 5 / 6

	// 2.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima
	void Scale(CDC* pDC, float sX, float sY, bool rightMultiply);

	// 3., 4.
	void DrawBackground(CDC* pDC);
	void DrawImgTransparent(CDC* pDC, DImage* pImage);

	// 5. - 10.
	void DrawLampBase(CDC* pDC, bool bIsShadow);
	void DrawLampArm1(CDC* pDC, bool bIsShadow);
	void DrawLampArm2(CDC* pDC, bool bIsShadow);
	void DrawLampHead(CDC* pDC, bool bIsShadow);
	void DrawLamp(CDC* pDC, bool bIsShadow);
	void DrawLampShadow(CDC* pDC);

	// postavlja k.s. u tacku na stolu gde stoji lampa (donja sredina postolja)
	void MoveToLampOrigin(CDC* pDC);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG
inline CKol1Doc* CKol1View::GetDocument() const
   { return reinterpret_cast<CKol1Doc*>(m_pDocument); }
#endif
