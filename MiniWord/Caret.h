#pragma once
#include "subeditor.h"

class Caret
{
public:
	int CaretPosX, CaretPosY;

	Caret(int x = 0, int y = 0) : CaretPosX(x), CaretPosY(y) {}
	~Caret() = default;

	void MvRight(line&, HDC&);
	void MvLeft(line&, HDC&);
	void MvUp(line&, HDC&);
	void MvDown(line&, HDC&);
	void MvHome(line&, HDC&);
	void MvEnd(line&, HDC&);
	
	void MvCHome(line&, HDC&);
	void MvCEnd(line&, HDC&);

	void CtrDelete(Article &,line&, HDC&);
	void CtrEnter(Article &,line&, HDC&);

	line CtrCaretMv(Article&, int x, int y, HDC&);//将光标移动到第y行，坐标x位置——即修改crt。其中若x大于y行行长，则光标横坐标crt.CaretPosX变为y行行长 ,返回当前行的指针
};