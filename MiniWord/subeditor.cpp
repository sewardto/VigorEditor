#include "stdafx.h"
#define MAXL 500


Article::Article()
{
	L = new Line;
	firstL = new Line;
	lastL = new Line;

	firstL->pre = nullptr;
	lastL->next = nullptr;

	firstL->next = L;
	lastL->pre = L;
	L->next = lastL;
	L->pre = firstL;
	lineNum = 1;
}

Article::~Article()
{
	line  p = firstL->next;
	while (p != lastL) {
		Remove(p);
		p = firstL->next;
	}
	Emptyredo();
	Emptyundo();
}

bool Article::IsEmpty() const {
	if (firstL->next->next == lastL) {
		line curL = firstL->next;
		if (curL->Getlen() == 0) return true;
	}
	return false;
}

line Article::GetLine(int lineNum) const
{

	line p = firstL->next;
	int j = 0;
	while (p != lastL && j != lineNum) {
		p = p->next;
		j++;
	}
	if (p == lastL)
		return nullptr;
	return p;
}
/*给line返回行号，注意，如果没找到这一行，则返回-1 */
int Article::GetNum(line& l) const
{
	int i = 0;
	line lstart = this->firstL->next;
	while (l != lstart && !IsEnd(lstart))
	{
		i++;
		lstart = lstart->next;
	}
	if (IsEnd(lstart)) return -1;
	return i;
}

void Article::InsertAfter(line& L)
{
	line newL = new Line;
	newL->pre = L;
	newL->next = L->next;
	L->next->pre = newL;
	L->next = newL;
}

void Article::Remove(line & L)
{
	delete(L);
	lineNum--;
}

int Article::MaxWidth(HDC hdc) const
{
	int Max = -1;
	int a;
	for (line L = this->firstL->next; L != this->lastL; L = L->next) {
		a = L->CharWidth(hdc);
		if (a > Max)
			Max = a;
	}
	return Max;
}

void Article::clearWord() {

	line curL = firstL -> next;
	curL->MakeEmpty();
	curL = curL-> next;
	line nextL;

	while (!IsEnd(curL)) {
		nextL = curL->next;
		Remove(curL);
		curL = nextL;
	}
	lineNum = 1;
	Emptyredo();
	Emptyundo();
}

Line::Line(int sz)
{
	if (sz < 0)
		return;
	size = sz;
	len = 0;
	gstart = 0;//gstart 默认指向将要输入字符的位置,即gap的第一个位置
	gend = size;//gend 默认指向右侧buffer第一个字符的位置，数组下标从0到size-1, gend=size表示右侧没有字符。
	arr = NULL;

	arr = new wchar_t[sz+1];
	memset(arr, 0, sizeof(wchar_t)*(sz + 1));

	if (arr == NULL) exit(0);//申请空间失败
}

/*析构一个Line,并连接上下指针*/
Line::~Line()
{
	pre->next = next;
	next->pre = pre;
	delete[]arr;
}


/*判断Line是否为满*/
int Line::IsFull()
{
	return gstart == gend;
}

/*判断Line是否为空*/
int Line::IsEmpty()
{
	return gstart == 0 && gend == size;
}

int Line::IsEmpty(int i) const
{
	if (i == LF)
		return gstart == 0;
	return gend == size;
}

/*在本Line后面新建一个Line，用在检测回车上。ps:若newLine时执行savespace函数*/
line  Line::NewLine()
{
	line  l = new Line;
	l->next = this->next;
	l->pre = this;
	this->next->pre = l;
	this->next = l;

	return l;
}

/*释放gapbuffer为0;*/
int Line::RleaseProcess()
{
	delete[]arr;
	arr = NULL;
	return 1;
}

/*用于满后申请数组*/
void Line::OverflowProcess()
{
	size += GapIncrement;

	wchar_t * newarr = NULL;

	newarr = new wchar_t[size + 1];
	if (newarr == NULL) exit(0);

	memset(newarr, 0, sizeof(wchar_t)*(size + 1));
	
	wcsnmove(newarr, arr, gstart);
	wcsnmove(newarr + gend + GapIncrement, arr + gend, len-gstart);

	delete[] arr;
	arr = newarr;
	gend += GapIncrement;

}

/*为正，光标往行尾移动p位。为负，光标往行首移动p位。
PointMove(1) 跟 LeftMovePoint 等价
*/
void Line::PointMove(int p)
{
	if (p > 0) {
		if (p == 1 && gstart == len)
		{
			//move point to next line begin
			return;
		}
		if (p > len - gstart)
			return;
		else {
			wcsnmove(arr + gstart, arr + gend, p);
			gstart += p;
			gend += p;
		}
	}
	else {
		p = -p;
		if (p == 1 && gstart == 0)
		{
			//move point to previous line end
			return;
		}
		if (p > gstart)
			return;
		else {
			memmove(arr + gend - p, arr + gstart - p, sizeof(wchar_t)*p);
			gstart -= p;
			gend -= p;
		}
	}
	return;
}

/*将光标移动到第d个字符*/
void Line::PointMoveto(int d)
{
	d = d - gstart;
	PointMove(d);
}

/*改变point后移动gap*/
int Line::Gapmove()
{
	/*将后半的数据拷贝与前半合并*/
	wcsnmove(arr + gstart, arr + gend, size - gend);
	gend = size;
	gstart = len;
	//Line::Savespace();

	return 1;//成功移动
}

/*取len 有效字符长度（用户眼中字符长度）*/
int Line::Getlen()
{
	return len;
}

int Line::Getlen(int i) const
{
	if (i == LF)
		return gstart;
	if (i == RG)
		return len - gstart;
}



/*取size(总大小)值*/
int Line::Getsize()
{
	return size;
}

/*传入面向user字符(有效字符)的位置(index)，返回在arr中的真实位置*/
int Line::UsertoGap(int x)
{
	if (x <= 0 || x >len) return -1;//error,x不合法
	if (x <= gstart) return x-1;
	else return x-1 + Line::Gapgsize();
}

/*取gs（目前光标位置）*/
int Line::GetPoint()
{
	return gstart;
}

/*取gap的宽度*/
int Line::Gapgsize()
{
	return gend - gstart;
}
int Line::GetGend()//取gend;
{
	return gend;
}

/*返回该行字符串*/
wchar_t * Line::GetPos()
{
	return arr;
}

/*LF, 光标前一字符位置 ，RG ， 光标后一个字符位置*/
wchar_t * Line::GetPos(int i)
{
	if (i == LF) {
		return arr;
	}
	else return arr + gend;
}

/*返回字符串;*/
wchar_t * Line::GetStr()
{
	wchar_t * l = new wchar_t[len + 1];
	memset(l,0,sizeof(wchar_t)*(len+1));
	wcsnmove(l, GetPos(LF), Getlen(LF));
	wcsnmove(l + Getlen(LF), GetPos(RG), Getlen(RG));
	return l;
}

/* 插入一个字符*/
void Line::Push(const wchar_t c, int i)
{
	if (IsFull() == 1) {
		OverflowProcess();
	}

	if (i == LF) {
		arr[gstart++] = c;
	}
	else {
		arr[--gend] = c;
	}
	len++;
}


/* 得到光标左侧元素*/
wchar_t Line::Top(int i)
{
	if (i == LF && !IsEmpty(LF))
		return arr[gstart - 1];
	else if (i == RG && !IsEmpty(RG))
		return arr[gend];
	return L'\0';
}


/*删除一个字符*/
wchar_t Line::Pop(int p)
{
	if (p == -1) {
		if (gstart == 0)
		{
			/*合并本段与上一段*/
			return NULL;
		}
		else {
			len--;
			gstart--;
			return arr[gstart];
		}
	}
	if (p == 1) {
		if (gstart == len)
		{
			return NULL;
			/*合并本段与下一段*/
		}
		else {
			len--;
			gend++;
			return arr[gend - 1];
		}
	}
	return NULL;
}


/*替换输入下一字符*/
void Line::Rwrite(const wchar_t &c)
{
	arr[gstart++] = c;
	gend++;
}

int Line::CharWidth(HDC hdc)
{
	/*	int width = 0;
	for (int i = gstart - 1; i >= 0; i--) {
	if (0x4E00 <= arr[i] && arr[i] <= 0x9FBB) {
	width += 2;
	}
	else width += 1;
	}
	for (int i = gend; i < size; i++) {
	if (0x4E00 <= arr[i] && arr[i] <= 0x9FBB) {
	width += 2;
	}
	else width += 1;
	}
	return width;
	*/
	return CharWidth(LF, hdc) + CharWidth(RG, hdc);
}


int Line::CharWidth(int d, HDC hdc) const
{
	int width = 0, nCharWidth = 0;
	if (d == LF) {
		for (int i = gstart - 1; i >= 0; i--) {
			GetCharWidth32W(hdc, (UINT)arr[i], (UINT)arr[i],
				&nCharWidth);
			width += nCharWidth;
			/*			if (0x4E00 <= arr[i] && arr[i] <= 0x9FBB) {
			width += 2;
			}
			else width += 1;
			*/
		}
	}
	else {
		for (int i = gend; i < size; i++) {
			GetCharWidth32W(hdc, (UINT)arr[i], (UINT)arr[i],
				&nCharWidth);
			width += nCharWidth;
			/*			if (0x4E00 <= arr[i] && arr[i] <= 0x9FBB) {
			width += 2;
			}
			else width += 1;
			*/
		}
	}
	return width;
}


void Line::MakeEmpty()
{
	len = 0;
	gstart = 0;
	gend = size;
	memset(arr, 0, sizeof(wchar_t)*(size + 1));
}
void Line::MakeEmpty(int i)
{
	if (i == -1) {
		memset(arr, 0, sizeof(wchar_t)*gstart);
		len -=gstart;
		gstart = 0;
	}
	else {

		memset(arr+gend, 0, sizeof(wchar_t)*(len - gstart));
		len = gstart;
		gend = size;
	}
}

/*删除 从 py行第px个字符右侧光标 到 my行第mx个字符右侧光标 之间的所有字符*/
selectPos Article::Delete(int py, int px, int my, int mx,int flag)
{
	wchar_t * str = GetStr(py, px, my, mx);

	if (py == my) {
		if (px == mx) return{px,py};
		/*同行操作*/
		else {
			if (px > mx)
			{
				int t = px;
				px = mx;
				mx = t;
			}
			/* px < mx */
			line tmpl = GetLine(py);
			tmpl->PointMoveto(px);
			
				
			tmpl->gend += mx - px;
			tmpl->len -= mx - px;
		}

		/*入栈操作*/
		undo a = new Undo{ selectPos{ px,py }, str };
		if (flag == U) UndoStack.push(a);
		else if (flag == R) RedoStack.push(a);

		return{px,py};
	}
	if (py > my) {
		int t = py;
		py = my;
		my = t;
		t = px;
		px = mx;
		mx = t;
	}
	/*py<my*/
	line lp = GetLine(py);
	line lm = GetLine(my);
	lp->PointMoveto(px);
	lm->PointMoveto(mx);
	
	
	lp->MakeEmpty(RG);
	lp->len = lp->gstart;

	int lenm = lm->Getlen(RG);
	while (lenm > lp->Gapgsize()) lp->OverflowProcess();
	lp->len += lenm;
	lp->gend -= lenm;
	wcsnmove(lp->GetPos(RG), lm->GetPos(RG),lenm);

	while (lp->next != lm)
	{
		delete lp->next;
		DecLineN();
	}
	delete lm;
	DecLineN();

	/*入栈操作*/
	undo a = new Undo{ selectPos{ px,py }, str };
	if (flag == U) UndoStack.push(a);
	else if (flag == R) RedoStack.push(a);

	return { px,py };
}

/*拷贝 从 py行第px个字符右侧光标 到 my行第mx个字符右侧光标 之间的所有字符*/
wchar_t* Article::GetStr(int py, int px, int my, int mx) 
{
	if (py == my) {
		if (px == mx) { 
			wchar_t *tmp = new wchar_t;
			tmp[0] = L'\0';
			return tmp;
		}
		/*同行操作*/
		else {
			/*前提p(光标)所在位置gstart在px处*/
			
			line l = GetLine(py);
			int tmper = l->GetPoint();
			l->PointMoveto(px);
			
			if (px > mx) {
				wchar_t * tmp = new wchar_t[px - mx + 1];
				memset(tmp, 0, sizeof(wchar_t)*(px - mx + 1));
				wcsnmove(tmp, l->arr + mx, px - mx);
				return tmp;
			}
			else {
				wchar_t * tmp = new wchar_t[mx - px + 1];
				memset(tmp, 0, sizeof(wchar_t)*(mx - px + 1));
				wcsnmove(tmp, l->arr + l->gend, mx - px);
				return tmp;

			}
			l->PointMoveto(tmper);
		}
	}

	/*情况为m,p光标位置均为mx，px处*/
	
	
	
	int ex = 0;
	
	if (my > py)
	{
		int t = my;
		my = py;
		py = t;
		t = mx;
		mx = px;
		px = t;
		ex = 1;
	}


	/*先将m移送*/
	line m = GetLine(my);
	line p = GetLine(py);
	
	int tmper;
	
	if (ex)  tmper= p->GetPoint();
	else  tmper = m->GetPoint();

	m->PointMoveto(mx);
	p->PointMoveto(px);
	
	/*my<py*/
		
	/*得到长度*/
	int sum = 0;
	sum += m->Getlen(RG) + 2;
	line t = m->next;
	while (t != p)
	{
		sum += t->Getlen() + 2;
		t = t->next;
	}
	sum += p->Getlen(LF);

	/*复制进入字符串*/

	wchar_t * tmp = new wchar_t[sum + 1];
	memset(tmp, 0, sizeof(wchar_t)*(sum+1));
	sum = 0;

	wcsnmove(tmp + sum, m->GetPos(RG), m->Getlen(RG));
	sum += m->Getlen(RG);
	wcsnmove(tmp + sum, L"\r\n", 2);
	sum += 2;

	t = m->next;
	while (t != p)
	{
		wcsnmove(tmp + sum, t->GetPos(), t->Getlen());
		sum += t->Getlen();
		wcsnmove(tmp + sum, L"\r\n", 2);
		sum += 2;
		t = t->next;
	}
	wcsnmove(tmp + sum, p->GetPos(LF), p->Getlen(LF));

	if (ex) p->PointMoveto(tmper);
	m->PointMoveto(tmper);//m的point恢复原位
	
	return tmp;

	
}


int * Article::getNextVal(const wchar_t *s)
{
	size_t len = wcslen(s);
	int * next = new int[len];
	int i = 0;
	int j = -1;
	next[0] = -1;
	while (i<len - 1)//注意这里跟KMP函数里面的不同
	{
		if (j == -1 || s[i] == s[j])
		{
			++i;
			++j;
			next[i] = j;
		}
		else
		{
			j = next[j];
		}
	}
	return next;
}

int Article::KMP(const wchar_t *s, const wchar_t *t)
{
	int slen, tlen;
	int i=0 , j=0;
	int * next = getNextVal(t);
	slen = wcslen(s);
	tlen = wcslen(t);

	while (i<slen)
		while ( j<tlen && i<slen)
		{
			if (j == -1 || s[i] == t[j]){
				++i; ++j;
			}
			else  j = next[j];
			if (j == tlen) {
				return i;
			}
		}

	delete[] next;
	return -1;
}
line Article::onSearch(line tmpL, const wchar_t * t) //tmpL是当前光标所在行，t是待匹配的字串
{
	/*此处的tmpL是形参，改变它不会对全局变量产生改变，因此用返回值传回去*/
	/*先对当前行光标后面的部分进行查找*/
	wchar_t * s = new wchar_t[tmpL->size + 1];
	memset(s, 0, sizeof(wchar_t)*(tmpL->size +1));
	wcsnmove(s , tmpL->GetPos(RG) ,  tmpL->Getlen(RG));

	int lenL = tmpL->Getlen(LF);//光标左侧的字符数量
	int res = KMP(s, t);
	if (res != -1) {
		tmpL->PointMoveto(res+lenL);
		return tmpL;
	}
	else {
		/*如果当前行之后的部分未找到，则对后面的行进行搜索*/
		if (!IsEnd(tmpL)) tmpL = tmpL->next;

		while (!IsEnd(tmpL))
		{  
			wchar_t * s = tmpL->GetStr();
			int res = KMP(s, t);
			if (res != -1) {
				tmpL->PointMoveto(res);
				return tmpL;
			}
			delete[]  s; 
			tmpL->Gapmove();
			tmpL = tmpL->next;
		}
		return NULL;
	}
	delete[] t;
	return NULL;
}

int Article::GetCharNum(int x, int y, HDC& hdc)
{
	line L = GetLine(y);

	if (!L) return -1;

	int width = 0;
	int i = 0, nCharWidth = 0;

	for (i = 0; i <= L->gstart - 1; ++i) {
		GetCharWidth32W(hdc, (UINT)L->arr[i], (UINT)L->arr[i], &nCharWidth);
		width += nCharWidth;
		if (width > x)
			return i;
	}

	for (i = L->gend; i <= L->size; ++i) {
		GetCharWidth32W(hdc, (UINT)L->arr[i], (UINT)L->arr[i], &nCharWidth);
		width += nCharWidth;
		if (width > x)
			return i - L->Gapgsize();
	}
	return L->len;
}


line Article::OnReplace(line tmpL, wchar_t * preStr,wchar_t * rpStr) {

	int Y = GetNum(tmpL);
	int mx = tmpL->Getlen(LF);//光标左侧的字符数量
	int px = mx - wcslen(preStr);
	selectPos p = Delete (Y, px, Y, mx);
	Insert(p.second, p.first, rpStr, U);
	undo a = new Undo();
	UndoStack.push(a);
	return GetLine(p.second);

}

void Article::Insert(int &y, const wchar_t *cc) {
	int x = GetLine(y)->Getlen(LF);
	Insert(y, x, cc,U);
}

void Article::Insert(int &y, int &x, const wchar_t * cc,int f)
{
	int tablen = 4;

	int sely = y;
	int selx = x;
	line l = GetLine(y);
	l->PointMoveto(x);

	size_t cclen = wcslen(cc);
	int counter = 0;
	int flag = 0;

	for (int i = 0; i < cclen; i++) {
		if (cc[i] == L'\r') {
			flag = 1;
			break;
		}
	}

	int storelen =l-> Getlen(RG);
	wchar_t * store = new wchar_t[storelen + 1];
	memset(store, 0, sizeof(wchar_t)*(storelen + 1));
	sizeof(store);/////////////
	wcsnmove(store, l->GetPos(RG), storelen);

	l->MakeEmpty(RG);

	for (int i = 0; i <= cclen; i++) {
		
		if (cc[i] != L'\r' && i != cclen && cc[i]!=L'\t') {
			continue;
		}
		int len = i - counter;
		
		while (len > l->Gapgsize()) l->OverflowProcess();

		wcsnmove(l->arr + l->gstart, cc + counter, len);

		l->gstart += len;
		l->len += len;


		if (cc[i] == L'\r')
		{
			i++;//跳到'\n'处,然后for循环的++跳到下一字符
			counter = i + 1;
			l = l->NewLine();
			IncLineN();//行数加一
			y++;
		}
		if (cc[i] == L'\t') {
			if (tablen > l->Gapgsize()) l->OverflowProcess();
			for (int ii = 0; ii < tablen; ii++)
			{
				wcsnmove(l->arr + l->gstart, L" ", 1);
				l->gstart ++;
				l->len ++;

			}
			counter = i + 1;
		}
	}

	while (storelen > l->Gapgsize()) l->OverflowProcess();

	l->gend = l->size - storelen;
	wcsnmove(l->arr + l->gend, store, storelen);
	l->len += storelen;
	
	
	x = l->Getlen(LF);
	undo a = new Undo(selectPos{selx,sely}, selectPos{ x,y });
	if (f == U) UndoStack.push(a);
	else if (f == R) RedoStack.push(a);

}
void Article::Emptyredo() {
	undo tmp;
	while (!RedoStack.empty()) {
		tmp = RedoStack.top();
		RedoStack.pop();
		delete tmp;
	}
}
void Article::Emptyundo() {
	undo tmp;
	while (!UndoStack.empty()) {
		tmp = UndoStack.top();
		UndoStack.pop();
		delete tmp;
	}
}

void Article::GetTotal()
{
	totalnum = 0;
	chinesenum = 0;
	int spacenum = 0;
	line p = L;
	while (!IsEnd(p)) {
		for (int i = p->gstart - 1; i >= 0; i--)
		{
			if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
				chinesenum++;
			else if (p->arr[i] == L' ')
				spacenum++;
		}
		for (int i = p->gend; i < p->size; i++)
		{
			if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
				chinesenum++;
			else if (p->arr[i] == L' ')
				spacenum++;
		}
		totalnum += p->len;
		p = p->next;
	}
	totalnumwithoutspace = totalnum - spacenum;
}

int Article::GetTotal(selectPos selectBegin, selectPos selectEnd)//返回选中总字数
{
	selectPos fronter = (selectBegin.second < selectEnd.second) || (selectBegin.second == selectEnd.second&&selectBegin.first <= selectEnd.first) ? selectBegin : selectEnd;
	selectPos backer = (selectBegin.second < selectEnd.second) || (selectBegin.second == selectEnd.second&&selectBegin.first <= selectEnd.first) ? selectEnd : selectBegin;
	line p = GetLine(fronter.second);
	line q = GetLine(backer.second);

	totalnum = 0;
	chinesenum = 0;
	int spacenum = 0;
	
	if (p == q) {
		totalnum = backer.first - fronter.first + 1;
		for(int i = p->UsertoGap(fronter.first+1);i<totalnum+ p->UsertoGap(fronter.first);i++)
		{
			if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
				chinesenum++;
			else if (p->arr[i] == L' ')
				spacenum++;
		}
	} else {
		totalnum += p->len-fronter.first;
		for (int i = p->UsertoGap(fronter.first+1); i<=p->UsertoGap(p->len); i++)
			if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
				chinesenum++;
			else if (p->arr[i] == L' ')
				spacenum++;
		p = p->next;
		while (p != q) {
			for (int i = p->UsertoGap(1); i <= p->UsertoGap(p->len); i++)
				if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
					chinesenum++;
				else if (p->arr[i] == L' ')
					spacenum++;

			totalnum += p->len;
			p = p->next;
		}
		for (int i = p->UsertoGap(1); i <= p->UsertoGap(backer.first); i++)
			if (0x4E00 <= p->arr[i] && p->arr[i] <= 0x9FBB)
				chinesenum++;
			else if (p->arr[i] == L' ')
				spacenum++;
		totalnum += backer.first;

	}
	totalnumwithoutspace = totalnum - spacenum;
	return backer.second - fronter.second + 1;
}
