#include "stdafx.h"

void Caret::MvLeft(line& L, HDC& hdc)
{
	if (!L->IsEmpty(LF)) {
		int nCharWidth;
		GetCharWidth32W(hdc, (UINT)L->Top(LF), (UINT)L->Top(LF),
			&nCharWidth);
		CaretPosX -= nCharWidth;
		L->PointMove(-1);
	}
	else if (!L->IsFirstL()) {
		L->Gapmove();
		CaretPosX = L->pre->CharWidth(hdc);
		CaretPosY -= 1;
		L = L->pre;
	}
}

void Caret::MvRight(line& L, HDC& hdc)
{
	if (!L->IsEmpty(RG)) {
		int nCharWidth;
		GetCharWidth32W(hdc, (UINT)L->Top(RG), (UINT)L->Top(RG),
			&nCharWidth);
		CaretPosX += nCharWidth;
		L->PointMove(1);
	}
	else if (!L->IsLastL()) {
		CaretPosX = 0;
		CaretPosY += 1;
		L = L->next;
		L->PointMoveto(0);
	}

}

void Caret::MvUp(line& L, HDC& hdc)
{
	if (!L->IsFirstL()) {
		L->Gapmove();
		CaretPosY--;
		L = L->pre;
		if (CaretPosX > L->CharWidth(hdc)) {
			CaretPosX = L->CharWidth(hdc);
		}
		else {
			while (L->CharWidth(LF, hdc) > CaretPosX)
				L->PointMove(-1);
			CaretPosX = L->CharWidth(LF, hdc);
		}
	}
	else {
		L->PointMoveto(0);
		CaretPosX = 0;
	}
}

void Caret::MvDown(line& L, HDC& hdc)
{
	if (!L->IsLastL()) {
		L->Gapmove();
		CaretPosY++;
		L = L->next;
		while (L->CharWidth(LF, hdc) > CaretPosX)
			L->PointMove(-1);
		CaretPosX = L->CharWidth(LF, hdc);
	}
	else {
		L->Gapmove();
		CaretPosX = L->CharWidth(hdc);
	}
}

void Caret::MvCHome(line& L, HDC& hdc)
{
	L->Gapmove();
	while (!L->IsFirstL()) {
		L = L->pre;
	}
	CaretPosX = 0;
	CaretPosY = 0;
	L->PointMoveto(0);
}

void Caret::MvCEnd(line& L, HDC& hdc)
{
	L->Gapmove();
	while (!L->IsLastL()) {
		L = L->next;
		CaretPosY++;
	}
	L->Gapmove();
	CaretPosX = L->CharWidth(hdc);

}
void Caret::MvHome(line& L, HDC& hdc)
{
	CaretPosX = 0;
	L->PointMoveto(0);
}

void Caret::MvEnd(line& L, HDC& hdc)
{
	CaretPosX = L->CharWidth(hdc);
	L->Gapmove();
}


void Caret::CtrDelete(Article &Ar, line& tmpL, HDC& hdc)
{
	selectPos p;

	if (!tmpL->IsEmpty(RG))
		p = Ar.Delete(CaretPosY, tmpL->Getlen(LF), CaretPosY, tmpL->Getlen(LF) + 1);
	else if (!tmpL->IsLastL())p = Ar.Delete(CaretPosY, tmpL->Getlen(LF), CaretPosY + 1, 0);
	else return;
	Ar.Emptyredo();
}

void Caret::CtrEnter(Article &Ar, line& tmpL, HDC& hdc)
{
	wchar_t c[] = L"\r\n";
	Ar.Insert(CaretPosY, c);
	tmpL = tmpL->next;
	CaretPosX = 0;
	Ar.Emptyredo();
}

line Caret::CtrCaretMv(Article& Ar, int x, int y, HDC& hdc)
{
	line L = Ar.GetLine(CaretPosY);
	L->Gapmove();
	CaretPosY = y;
	L = Ar.GetLine(CaretPosY);
	
	int width = 0;
	int i = 0, nCharWidth = 0;

	for (i = 0; i != L->gstart && width <= x; ++i) {
		GetCharWidth32W(hdc, (UINT)L->arr[i], (UINT)L->arr[i], &nCharWidth);
		width += nCharWidth;
	}
	if (width > x) { 
		width -= nCharWidth;
		i--;
	}
	CaretPosX = width;
	L->PointMoveto(i);
	return L;
}