#pragma once
#include "stdafx.h"
enum Operation{Ins,Del,Rep};

typedef class Undo {
public:
	selectPos Begin;
	int Op;//记录执行过的操作

	selectPos End;//插入时记入End位置，删除时删除从x,y 到 End
	wchar_t * str;//删除时记录进入，撤销时在x,y位置插入str

	Undo(selectPos, selectPos);//插入时记入End位置，删除时删除从x,y 到 End
	Undo(selectPos, wchar_t *);//删除时记录进入，撤销时在x,y位置插入str
	Undo();//替换标识符，意味着要连续三次pop
	~Undo();
}*undo;