#pragma once

#include<iostream>
#include <stack>
#include"undo.h"

const int GapIncrement = 30;// gapArticle每次增加大小
const int DefaultSize = 60;//默认
typedef class Line * line;
enum stack { LF=-1 , RG=1 };

enum UNRE { U, R };//区分对哪个栈进行操作

typedef class Line
{
public:
	int len;//有效字符数量
	int size;//Line的总大小(字符数量)
	int gstart;//gapstart,gap开始位置，光标位置（光标若在）
	int gend;//gapend,gap结束位置
	wchar_t* arr;//数组

	line pre;//上一个Line
	line next;//下一个Line

	Line(int sz = DefaultSize);//创建一个空Line，包括创建指针，申请空间size
	~Line();//析构一个Line,并连接上下指针
	line NewLine();//在本Line后面新建一个Line

	int IsFull();//判断Line是否为满
	int RleaseProcess();//释放gapbuffer为0;
	void OverflowProcess(); //用于满了后申请数组

	void PointMove(int p); //为正，光标往行尾移动p位。为负，光标往行首移动p位。
	void PointMoveto(int d);//将光标移动到第d个字符
	//PointMove( -1 )		int LeftMovePoint();//不改变原句，光标左移
	//PointMove( 1 )		int RightMovePoint();//不改变原句，光标右移
	int Gapmove();//改变point后移动gap
	int UsertoGap(int);//传入面向user字符的位置，返回在arr中的真实位置
	int Gapgsize();//取gap的宽度;
	int GetPoint();//取gstart（目前光标位置）
	int Getlen();//取len 有效字符长度（用户眼中字符长度）
	int Getlen(int i) const;//取len 有效字符长度（用户眼中字符长度）
	wchar_t * GetPos();//返回该行字符串指针
	wchar_t * GetPos(int i);//返回左右字符串 LF, return arr ，RG ， return arr+gend
	wchar_t * GetStr();//返回字符串;
	int CharWidth(HDC hdc);//字符长度
	int CharWidth(int i, HDC hdc) const;//i=1 右侧，i=-1 左侧
	int IsEmpty(int i) const;	//判断Line是否为空 ，i可为LF，RG
	int IsEmpty();			//判断Line是否为空
	void MakeEmpty();//清空内容
	void MakeEmpty(int i);//清空内容 清空左右内容
	wchar_t Top(int i);//LG得到左侧元素 RG得到右侧元素
	void Push(const wchar_t c, int i);//插入一个字符 LF,插左，RG插右
	wchar_t Pop(int p);//删除一个字符，p=1删除光标后面的相当于del , p=-1删除光标前面的相当于backspace。
	void Rwrite(const wchar_t &c);//替换输入下一字符
	bool IsFirstL() { return this->pre->pre == nullptr; }
	bool IsLastL() { return this->next->next == nullptr; }

	int GetGend();//取gend;
	int Getsize();//取size (总大小)值

} *line;

class Article {
private:
	line firstL;
	line lastL;
	int lineNum;

public:
	line L;//首行，无法删掉
	int totalnum;
	int totalnumwithoutspace;
	int chinesenum;

	Article();//构造 linehead的next为空
	~Article();
	bool IsEmpty() const;
	bool IsFirstL(line& L) const { return L->pre == firstL; }
	bool IsLastL(line& L) const { return L->next == lastL; }
	bool IsEnd(line& L) const { return L == lastL; }
	line GetLine(int lineNum) const;
	int GetNum(line& L) const;
	int MaxWidth(HDC) const;
	void InsertAfter(line& L);
	void Remove(line &L);
	int LineNum(void) const { return lineNum; }
	void IncLineN(void) { lineNum++; }
	void DecLineN(void) { lineNum--; }
	void clearWord(); //清空当前Article
	selectPos Delete(int py, int px, int my, int mx, int flag = U); //删除 从 py行第px个字符右侧光标 到 my行第mx个字符右侧光标 之间的所有字 符
	wchar_t* GetStr(int py, int px, int my, int mx); //复制 从 py行第px个字符右侧光标 到 my行第mx个字符右侧光标 之间的所有字符
	void Insert(int &py, int &px, const wchar_t * cc, int flag);//指定光标位置 插入字符串,并且改变py px 为当前所在位置,主要面向粘贴等
	void Insert(int &py, const wchar_t * cc);//同上插入字符串,不过此时为光标位置已经选好，主要面向撤销
	 /* 查找功能 */
	line onSearch(line tmpL, const wchar_t * t);
	int KMP(const wchar_t *s, const wchar_t *t);
	int * getNextVal(const wchar_t *s);
	/*替换功能*/
	line OnReplace(line tmpL, wchar_t * preStr, wchar_t * rpStr); //替换操作，preStr是查找的串，rpStr是待替换上的串
	/*给定坐标x（左条长+现鼠标与左端距离），行号y
	return值为：若点下后的gstart。超过行距返回glen*/
	int GetCharNum(int x, int y, HDC& hdc);

	/*撤销栈与恢复栈*/
	std::stack<undo> UndoStack;//撤销栈
	std::stack<undo> RedoStack;//恢复栈
	void Emptyredo();//当有新操作时，清空恢复栈
	void Emptyundo();//新建文档时，清空撤销栈

	void GetTotal();//返回总字数
	int GetTotal(selectPos Begin, selectPos End);//返回选中总字数 并返回总行数
};

