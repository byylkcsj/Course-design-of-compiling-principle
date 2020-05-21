#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <ctype.h>
#include <cstdlib>

using namespace std;


/*本程序中：
主函数使用了cin.getline( 第一个字符指针(char*),字符个数(int),结束符'#'(char) );

词法分析函数使用了isalpha库函数
判断字符是否为英文字母
若大写字母返回1，小写字母返回2，非字母返回0

词法分析函数使用了isdigit库函数
判断是否为十进制数字
若阿拉伯数字0-9返回非零值，否则返回0

*/
#define CodeMaxLength 500	 //最大代码长度
#define VarMaxNumber 100	 //变量最大个数
#define ConstMaxNumber 100	//常量最大个数
#define KeywordMaxNumber 33	//关键字数量
#define OpSingleMaxNumber 8	//单目运算符最大个数
#define OpDoubleMaxNumber 4	//双目运算符最大个数
#define EndMaxNumber 11	//界符最大个数

typedef struct ShowTable//某个标识符的展示表
{
	int Index;          //某个标识符展示表的下标
	//注：会有Index相同但type不同的table（详见词法分析函数的常数与标识符两部分）
	char CharacterInTableName[20];	//某标识符的名称
	int type;
	/*
	该标识符的类型：
	1 关键字
	2 变量标识符
	3 整数
	4 小数
	5 单目运算符
	6 双目运算符
	7 分界符
	8 无法识别的其他字符
	*/
	int line;	           //该标识符在第几行
}Table;

int TableNum = 0;  //展示表总数
char Word[VarMaxNumber][20]; //变量表，用于存放变量名
char Digit[VarMaxNumber][20]; //数字表，用于存放数字
int VarNum = 0;   //变量表的下标
int ConstNum = 0;	 //常量表的下标
bool errorFlag = 0; //错误标志
int TableIndex = -1;  //Table表的下标索引

Table* table = new Table[CodeMaxLength];

//关键字	
const char* const Keyword[KeywordMaxNumber] = {
	"and",
	"array",
	"begin",
	"case",
	"char",
	"constant",
	"do",
	"else",
	"end",
	"false",
	"for",
	"if",
	"input",
	"integer",
	"not",
	"of",
	"or",
	"output",
	"packed",
	"procedure",
	"program",
	"read",
	"real",
	"repeat",
	"set",
	"then",
	"to",
	"type",
	"until",
	"var",
	"while",
	"with",
	"prn" };

// 单目运算符号
const char OpSingle[] =
{ '+','-','*','/','=','#','<','>' };

//双目运算符号
const char* OpDouble[] =
{ "<=",">=",":=","<>" };

// 界符
const char End[] =
{ '(', ')' , ',' , ';' , '.' , '[' , ']' , ':' , '{' , '}' , '"' };

//词法错误显示函数，错误1变量名过长，错误2小数点，错误3常量过长
void error(char str[20], int nLine, int errorType)
{
	errorFlag = 1;
	cout << " \n错误 :    ";
	switch (errorType)
	{
	case 1:
		cout << "第" << nLine - 1 << "行" << str << " 变量名过长\n";
		break;
	case 2:
		cout << "第" << nLine - 1 << "行" << str << " 小数点错误！\n";
		break;
	case 3:
		cout << "第" << nLine - 1 << "行" << str << " 常量过长\n";
		break;
	}//switch分支结束	
}//error函数结束

/*
词法分析
输入的字符（串）可能为
空格
tab
换行符
标识符（关键字或非关键字）
常数
运算符
*/

void LexicalAnalyzer(char character[], int charLength, int nLine)
{
	int characterIndex = 0;//字符序号
	while (characterIndex < charLength) //对输入的字符扫描
	{


		//忽略空格和tab
		while (character[characterIndex] == ' ' || character[characterIndex] == 9) //tab为9
			characterIndex++;


		//遇到换行符，行数加1
		while (character[characterIndex] == 10)//换行符为10
		{
			nLine++; characterIndex++;
		}

		//字母开头可能是变量也可能是关键字
		if (isalpha(character[characterIndex]))
			//指针指向第一个字符，以字母开头，有可能是变量也有可能是关键字
		{
			char str[256];
			int strLen = 0;
			//是字母、下划线
			while (isalpha(character[characterIndex]) || character[characterIndex] == '_') {
				str[strLen++] = character[characterIndex];//当前字符串字符被赋值为代码段中的下一个字符
				characterIndex++;//下一个字符的序号
				while (isdigit(character[characterIndex]))//不是第一位，可以为数字
				{
					str[strLen++] = character[characterIndex];
					characterIndex++;
				}
			}//这个单词（标识符）结束了

			str[strLen] = 0; //字符串重新置零

			//如果名字超长
			if (strlen(str) > 20) //标识符的名字超过规定长度报错
			{
				error(str, nLine, 1);
			}//该行出现1型错误即变量名过长

           //如果名字没有超过长度，则有可能是关键字，也有可能是变量标识符
			else
			{
				int i;
				for (i = 0; i < KeywordMaxNumber; i++) //与关键字匹配
		   //如果是关键字则写入table表中
					if (strcmp(str, Keyword[i]) == 0)
					{
						strcpy(table[TableNum].CharacterInTableName, str);//把该标识符的名称写入展示表
						table[TableNum].type = 1;  //该标识符是1型（是个关键字）
						table[TableNum].line = nLine;//该标识符在这一行
						table[TableNum].Index = i;//该标识符序号为i
						TableNum++;//展示表数量加一
						break;//若这个单词是某个关键字，就不用再继续匹配了，可以跳出循环（去看下一个单词吧）
					}
				/*这里与前面的变量定义完全相同：
			int TableNum = 0;  //展示表总数
			char Word[VarMaxNumber][20]; //变量表，用于存放变量名
			char Digit[VarMaxNumber][20]; //数字表，用于存放数字
			int VarNum = 0;   //变量表的下标
			int ConstNum = 0;	 //常量表的下标
			bool errorFlag = 0; //错误标志
			int TableIndex = -1;  //Table表的下标索引
			int BeginEndProgram = 0;
			//正确情况下应该为0，遇到begin加1，遇到end减1，<0则缺少Begin，>0则缺少End
			int ifCount = 0; //遇到if加1
				*/

				//否则是变量
				if (i >= KeywordMaxNumber) //如果不是关键字则把它作为变量标识符
				{
					table[TableNum].Index = VarNum;//这是第VarNum个变量
					strcpy(Word[VarNum++], str);//这里同时又变量数加一,把这个变量名存入变量表
					table[TableNum].type = 2; //这个标识符是2型（是个变量）
					strcpy(table[TableNum].CharacterInTableName, str);//该标识符名称写入展示表
					table[TableNum].line = nLine;//记录行数
					TableNum++;//展示表数加一
				}
			}
		}

		//数字开头为常数
		else if (isdigit(character[characterIndex])) //以数字开头
		{
			int floatnum = 0;
			char str[256];
			int strLen = 0;
			//数字和小数点
			while (isdigit(character[characterIndex]) || character[characterIndex] == '.') {
				//floatnum小数点的个数，0时为整数 1时为小数 >=2时出错
				if (character[characterIndex] == '.')
					floatnum++;//只要遇到一个小数点，就把小数点数量加一
				str[strLen++] = character[characterIndex];//读下一个字符
				characterIndex++;//字符序号加一
			}//这个常数结束了

			str[strLen] = 0;//字符串重新置零

			if (strlen(str) > 20)
			{
				error(str, nLine, 3);//常量过长，错误类型3
			}
			if (floatnum == 0)
			{
				table[TableNum].type = 3; //整数
			}
			if (floatnum == 1)
			{
				table[TableNum].type = 4; //小数
			}
			if (floatnum > 1)
			{
				error(str, nLine, 2);//多个小数点，错误类型2
			}

			/*这里与前面的变量定义完全相同：
		int TableNum = 0;  //展示表总数
		char Word[VarMaxNumber][20]; //变量表，用于存放变量名
		char Digit[VarMaxNumber][20]; //数字表，用于存放数字
		int VarNum = 0;   //变量表的下标
		int ConstNum = 0;	 //常量表的下标
		bool errorFlag = 0; //错误标志
		int TableIndex = -1;  //Table表的下标索引
		int BeginEndProgram = 0;
		//正确情况下应该为0，遇到begin加1，遇到end减1，<0则缺少Begin，>0则缺少End
		int ifCount = 0; //遇到if加1
			*/
			table[TableNum].Index = ConstNum;//这是第ConstNum个常数
			strcpy(Digit[ConstNum++], str);//常数值存入数字表
			strcpy(table[TableNum].CharacterInTableName, str);//常数存入这个对应的展示表
			table[TableNum].line = nLine;//记录行数
			TableNum++;//展示表数加一
		}

		//其他开头可能是运算符也可能是界符
		else
		{
			//用来区分是不是无法识别的标识符，0为运算符，1为界符
			int errorFlag;

			char str[3];
			str[0] = character[characterIndex];
			str[1] = character[characterIndex + 1];
			str[2] = '\0';
			int i;
			for (i = 0; i < OpDoubleMaxNumber; i++)

				//如果这个字符串和第i个双目运算符匹配
				if (strcmp(str, OpDouble[i]) == 0)
				{
					errorFlag = 0;//则它是运算符
					table[TableNum].type = 6;//双目运算符类型6
					strcpy(table[TableNum].CharacterInTableName, str);
					table[TableNum].line = nLine;
					table[TableNum].Index = i;//第i个双目运算符
					TableNum++;
					characterIndex = characterIndex + 2;
					break;//这个含有两个字符的串肯定是双目运算符了，可以看下一个字符了
				}

			//跟全部4个双目运算符都没有发生匹配，此时i已经等于4
			if (i >= OpDoubleMaxNumber)
			{

				//判断是否为单目运算符
				for (int k = 0; k < OpSingleMaxNumber; k++)
					if (OpSingle[k] == character[characterIndex])
					{
						errorFlag = 0;//则它是运算符
						table[TableNum].type = 5;//单目运算符类型5
						table[TableNum].CharacterInTableName[0] = character[characterIndex];
						table[TableNum].CharacterInTableName[1] = 0;
						table[TableNum].line = nLine;
						table[TableNum].Index = k;//第k个单目运算符
						TableNum++;
						characterIndex++;
						break;
					}


				//跟全部8个单目运算符都没有发生匹配
				//再判断是否为分界符
				for (int j = 0; j < EndMaxNumber; j++)
					if (End[j] == character[characterIndex])
					{
						errorFlag = 1;//则它是分界符
						table[TableNum].line = nLine;
						table[TableNum].CharacterInTableName[0] = character[characterIndex];
						table[TableNum].CharacterInTableName[1] = 0;
						table[TableNum].Index = j;//第j个分界符
						table[TableNum].type = 7;//分界符为类型7
						TableNum++;
						characterIndex++;
					}


				//无法识别字符
				//开头的不是字母、数字、运算符、界符
				if (errorFlag != 0 && errorFlag != 1) {
					char str[256];
					int strLen = -1;
					str[strLen++] = character[characterIndex];
					characterIndex++;
					while (*character != ' ' || *character != 9 || character[characterIndex] != 10)//不是空格tab换行
					{
						str[strLen++] = character[characterIndex];
						characterIndex++;
					}
					str[strLen] = 0;
					table[TableNum].type = 8;//无法识别的字符类型8
					strcpy(table[TableNum].CharacterInTableName, str);
					table[TableNum].line = nLine;
					table[TableNum].Index = -2;//无法识别的字符的展示表的序号一律为-2
					TableNum++;
				}
			}
		}

	}
}


/********
******把十进制小数转为16进制****
**************/
/*void Trans(double x, int p)  //把十进制小数转为16进制
{
	int i = 0;                  //控制保留的有效位数
	while (i < p)
	{
		if (x == 0)              //如果小数部分是0
			break;            //则退出循环
		else
		{
			int k = int(x * 16);  //取整数部分
			x = x * 16 - int(k);    //得到小数部分
			if (k <= 9)
				cout << k;
			else
				cout << char(k + 55);
		};
		i++;
	};
};*/



int main()
{
	ifstream in;
	ofstream out;
	char in_file_name[26], out_file_name[26];
	char characterinput[CodeMaxLength];//最多可以输入500个字符（不是单词）

	cin.getline(characterinput, CodeMaxLength, '#');
	//从头读到最后，以#为结束符

	int nLine = 1;


	cout << "词法错误" << endl;
	//先词法分析
	LexicalAnalyzer(characterinput, strlen(characterinput), nLine);
	//for(int i = 0; i < TableNum;i ++)
   //		cout << table[i].type<< "		"<< table[i].symbol<< "		"<<table[i].Index<<"		"<<table[i].line<< endl;


	return 0;
}
