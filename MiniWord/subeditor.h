#pragma once

#include<iostream>
#include <stack>
#include"undo.h"

const int GapIncrement = 30;// gapArticleÿ�����Ӵ�С
const int DefaultSize = 60;//Ĭ��
typedef class Line * line;
enum stack { LF=-1 , RG=1 };

enum UNRE { U, R };//���ֶ��ĸ�ջ���в���

typedef class Line
{
public:
	int len;//��Ч�ַ�����
	int size;//Line���ܴ�С(�ַ�����)
	int gstart;//gapstart,gap��ʼλ�ã����λ�ã�������ڣ�
	int gend;//gapend,gap����λ��
	wchar_t* arr;//����

	line pre;//��һ��Line
	line next;//��һ��Line

	Line(int sz = DefaultSize);//����һ����Line����������ָ�룬����ռ�size
	~Line();//����һ��Line,����������ָ��
	line NewLine();//�ڱ�Line�����½�һ��Line

	int IsFull();//�ж�Line�Ƿ�Ϊ��
	int RleaseProcess();//�ͷ�gapbufferΪ0;
	void OverflowProcess(); //�������˺���������

	void PointMove(int p); //Ϊ�����������β�ƶ�pλ��Ϊ��������������ƶ�pλ��
	void PointMoveto(int d);//������ƶ�����d���ַ�
	//PointMove( -1 )		int LeftMovePoint();//���ı�ԭ�䣬�������
	//PointMove( 1 )		int RightMovePoint();//���ı�ԭ�䣬�������
	int Gapmove();//�ı�point���ƶ�gap
	int UsertoGap(int);//��������user�ַ���λ�ã�������arr�е���ʵλ��
	int Gapgsize();//ȡgap�Ŀ��;
	int GetPoint();//ȡgstart��Ŀǰ���λ�ã�
	int Getlen();//ȡlen ��Ч�ַ����ȣ��û������ַ����ȣ�
	int Getlen(int i) const;//ȡlen ��Ч�ַ����ȣ��û������ַ����ȣ�
	wchar_t * GetPos();//���ظ����ַ���ָ��
	wchar_t * GetPos(int i);//���������ַ��� LF, return arr ��RG �� return arr+gend
	wchar_t * GetStr();//�����ַ���;
	int CharWidth(HDC hdc);//�ַ�����
	int CharWidth(int i, HDC hdc) const;//i=1 �Ҳ࣬i=-1 ���
	int IsEmpty(int i) const;	//�ж�Line�Ƿ�Ϊ�� ��i��ΪLF��RG
	int IsEmpty();			//�ж�Line�Ƿ�Ϊ��
	void MakeEmpty();//�������
	void MakeEmpty(int i);//������� �����������
	wchar_t Top(int i);//LG�õ����Ԫ�� RG�õ��Ҳ�Ԫ��
	void Push(const wchar_t c, int i);//����һ���ַ� LF,����RG����
	wchar_t Pop(int p);//ɾ��һ���ַ���p=1ɾ����������൱��del , p=-1ɾ�����ǰ����൱��backspace��
	void Rwrite(const wchar_t &c);//�滻������һ�ַ�
	bool IsFirstL() { return this->pre->pre == nullptr; }
	bool IsLastL() { return this->next->next == nullptr; }

	int GetGend();//ȡgend;
	int Getsize();//ȡsize (�ܴ�С)ֵ

} *line;

class Article {
private:
	line firstL;
	line lastL;
	int lineNum;

public:
	line L;//���У��޷�ɾ��
	int totalnum;
	int totalnumwithoutspace;
	int chinesenum;

	Article();//���� linehead��nextΪ��
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
	void clearWord(); //��յ�ǰArticle
	selectPos Delete(int py, int px, int my, int mx, int flag = U); //ɾ�� �� py�е�px���ַ��Ҳ��� �� my�е�mx���ַ��Ҳ��� ֮��������� ��
	wchar_t* GetStr(int py, int px, int my, int mx); //���� �� py�е�px���ַ��Ҳ��� �� my�е�mx���ַ��Ҳ��� ֮��������ַ�
	void Insert(int &py, int &px, const wchar_t * cc, int flag);//ָ�����λ�� �����ַ���,���Ҹı�py px Ϊ��ǰ����λ��,��Ҫ����ճ����
	void Insert(int &py, const wchar_t * cc);//ͬ�ϲ����ַ���,������ʱΪ���λ���Ѿ�ѡ�ã���Ҫ������
	 /* ���ҹ��� */
	line onSearch(line tmpL, const wchar_t * t);
	int KMP(const wchar_t *s, const wchar_t *t);
	int * getNextVal(const wchar_t *s);
	/*�滻����*/
	line OnReplace(line tmpL, wchar_t * preStr, wchar_t * rpStr); //�滻������preStr�ǲ��ҵĴ���rpStr�Ǵ��滻�ϵĴ�
	/*��������x��������+���������˾��룩���к�y
	returnֵΪ�������º��gstart�������о෵��glen*/
	int GetCharNum(int x, int y, HDC& hdc);

	/*����ջ��ָ�ջ*/
	std::stack<undo> UndoStack;//����ջ
	std::stack<undo> RedoStack;//�ָ�ջ
	void Emptyredo();//�����²���ʱ����ջָ�ջ
	void Emptyundo();//�½��ĵ�ʱ����ճ���ջ

	void GetTotal();//����������
	int GetTotal(selectPos Begin, selectPos End);//����ѡ�������� ������������
};

