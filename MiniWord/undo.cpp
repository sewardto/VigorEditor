#include"stdafx.h"
Undo::Undo(selectPos B, wchar_t * wcs) 
{	
	Begin = B;
	Op = Del;
	str = new wchar_t[wcslen(wcs) + 1];
	memset(str, 0, sizeof(wchar_t)*(wcslen(wcs) + 1));
	wcsmove(str, wcs);
}
Undo::Undo(selectPos B, selectPos E) 
{
	Begin = B;
	Op = Ins;
	End = E;
	str = NULL;
}
Undo::Undo() {
	Op = Rep;
}
Undo::~Undo() 
{
	if (Op == Del && str)
		delete[] str;
}