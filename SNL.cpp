#pragma warning(disable:4996)
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<string.h>
using namespace std;
#define NUMOFRESERVED 22
#define NUMOFSYMBOLS 20
#define RES 0
#define ID 1
#define NUM 2
#define SYM 3
#define STR 4

#define PLUS 100
#define SUB 101
#define MUL 102
#define DIV 103
#define LT 104
#define LBRACK 105
#define RBRACK 106
#define LSQUBRACK 107
#define RSQUBRACK 108
#define POINT 109
#define SEMI 110
#define LBRACE 111
#define RBRACE 112
#define EOFF 113
#define BLANK 114
#define QUO 115
#define EQU 116
#define INDEX 117
#define ASSI 118
#define COM 119


#define uint unsigned

typedef struct Token {
	int id;
	int ptr;
	int linenum;
} Token;
typedef struct Node {
	char *desc;
	vector<Node*> children;
	Node(const char* str) {
		desc = new char[30];
		strcpy(desc, str);
	}
} Node;

vector<Token>* tokens;
int Token_n = 0;
const char* reserved_words[NUMOFRESERVED] = { "begin","end","program","var",
											"type","procedure","while","endwh",
											"integer","char","array","of",
											"intc","record","if","then",
											"else","fi","do","write",
											"read","return"
};

static const char* symbol_table[NUMOFSYMBOLS] = { "+","-","*","/","<",
												"(",")","[","]",".",
												";","{","}","EOF","BLANK",
												"\'","=","..",":=",","
};
void syntaxError(const char* message);
Node* MatchRES(const char* expected);
Node* MatchSYM(const int expected);
Node* MatchID();
Node* MatchNUM();
bool isRES(const char* res);
bool isSYM(const int res);
bool isID();
bool isNUM();
bool isLineEnd();
Node* Program();
Node* ProgramHead();
Node* DeclarePart();
Node* TypeDec();
Node* TypeDeclaration();
Node* TypeDecList();
Node* TypeDecMore();
Node* TypeId();
Node* TypeName();
Node* BaseType();
Node* StructureType();
Node* ArrayType();
Node* RecType();
Node* FieldDecList();
Node* FieldDecMore();
Node* IdList();
Node* IdMore();
Node* VarDec();
Node* VarDeclaration();
Node* VarDecList();
Node* VarDecMore();
Node* VarIdList();
Node* VarIdMore();
Node* ProcDec();
Node* ProcDeclaration();
Node* ParamList();
Node* ParamDecList();
Node* ParamMore();
Node* Param();
Node* FormList();
Node* FidMore();
Node* ProcDecPart();
Node* ProcBody();
Node* ProgramBody();
Node* StmList();
Node* StmMore();
Node* Stm();
Node* AssCall();
Node* AssignmentRest();
Node* ConditionalStm();
Node* LoopStm();
Node* InputStm();
Node* OutputStm();
Node* ReturnStm();
Node* CallStmRest();
Node* ActParamList();
Node* ActParamMore();
Node* Exp();
Node* Term();
Node* OtherFactor();
Node* OtherTerm();
Node* Factor();
Node* Variable();
Node* VariMore();
Node* FieldVar();
Node* FieldVarMore();
Node* Parse();
Node* CmpOp();
Node* AddOp();
Node* MultOp();

void show_tree(Node* root, int depth, vector<int>* v, bool islast);

int add(vector<Node* >& children, Node* node) {//vector容器在函数传参中需要使用引用传参(或者指针传参？没试过)，不然在函数局部无法进行增删操作
	if (node != NULL) {
		children.push_back(node);
	}
	return 0;
}
#define COMMENT_ERROR -1000
#define STR_ERROR -1001
#define SYM_ERROR -1002
#define NOT_REC_SYMBOL -1003
vector<char*> ID_table;
vector<uint> NUM_table;
vector<char*> STR_table;
int ID_n = 0, NUM_n = 0, STR_n = 0;
int num_of_lines = 1;
FILE * file;



/***************************************************************************************/
/*****************                   词法分析                          *****************/
/***************************************************************************************/

void handle_error(int error_num, const char* str) {
	printf("行号:%d \t", num_of_lines);
	switch (error_num) {
	case COMMENT_ERROR:
		printf("注释未结束 \t");
		break;
	case SYM_ERROR:
		printf("出现未定义的符号，您想输入的是\":=\"吗? \t");
		break;
	case STR_ERROR:
		printf("字符串未结束 \t");
		break;
	case NOT_REC_SYMBOL:
		printf("出现未定义的符号 \t");
		break;
	}
	printf("%s\n", str);
	exit(-1);
}
char getNextChar() {
	char c;
	c = fgetc(file);
	return c;
}

void ungetNextChar() {
	fseek(file, -1, SEEK_CUR);
}

int reservedLookup(char* str) {
	for (int i = 0; i < NUMOFRESERVED; i++) {
		if (strcmp(str, reserved_words[i]) == 0)return i;
	}
	return -1;
}
int str_to_num(char* str) {
	int len = strlen(str);
	int n = 0;
	for (int i = 0; i < len; i++) {
		n = n * 10 + str[i] - '0';
	}
	return n;
}
int addIDTable(char* str) {
	ID_table.push_back(str);
	return ID_n++;
}
int addNUMTable(int num) {
	NUM_table.push_back(num);
	return NUM_n++;
}
int addSTRTable(char* str) {
	STR_table.push_back(str);
	return STR_n++;
}
Token scan() {
	static const char symbols[12] = { '+','-','*','/','<','(',')','[',']',';','=',',' };
	static const int symbols_n[12] = { PLUS,SUB,MUL,DIV,LT,LBRACK,RBRACK,LSQUBRACK,RSQUBRACK,SEMI,EQU,COM };
	Token t;
	t.linenum = num_of_lines;
	char* str = new char[256];
	strcpy(str, "");//字符串要初始化，不然有乱码
	char c[2] = { 'a','\0' };
	c[0] = getNextChar();
LS0:
	if (c[0] >= 'a'&&c[0] <= 'z')goto LS1;
	if (c[0] >= 'A'&&c[0] <= 'Z')goto LS1;
	if (c[0] >= '0'&&c[0] <= '9')goto LS2;
	int indexofsym;
	for (indexofsym = 0; indexofsym < 12; indexofsym++) {
		if (c[0] == symbols[indexofsym]) {
			goto LS3;
		}
	}
	if (c[0] == '.')goto LS4;
	if (c[0] == '{')goto LS5;
	if (c[0] == ':')goto LS6;
	if (c[0] == '\'')goto LS7;
	if (c[0] == '\n')goto LS8;
	if (c[0] == EOF)goto LS9;
	if (c[0] == '\t' || c[0] == ' ')goto LS10;
	goto OTHER;
LS1://保留字&变量名
	strcat(str, c);
	c[0] = getNextChar();
	if (c[0] >= 'a'&&c[0] <= 'z')goto LS1;
	else if (c[0] >= 'A'&&c[0] <= 'Z')goto LS1;
	else if (c[0] >= '0'&&c[0] <= '9')goto LS1;
	ungetNextChar();
	int nres;
	if (nres = reservedLookup(str), nres != -1) {
		t.id = RES;
		t.ptr = nres;
		return t;
	}
	else {
		t.id = ID;
		t.ptr = addIDTable(str);
		return t;
	}
LS2://常数量
	strcat(str, c);
	c[0] = getNextChar();
	if (c[0] >= '0'&&c[0] <= '9')goto LS2;
	ungetNextChar();
	t.id = NUM;
	t.ptr = addNUMTable(str_to_num(str));
	return t;
LS3://分界符等符号
	t.ptr = symbols_n[indexofsym];
	t.id = SYM;
	return t;
LS4:
	strcat(str, c);
	c[0] = getNextChar();
	if (c[0] == '.') {
		t.id = SYM;
		t.ptr = INDEX;
		return t;
	}
	else {
		t.id = SYM;
		t.ptr = POINT;
		ungetNextChar();
		return t;
	}
LS5://注释
	while (c[0] = getNextChar(), c[0] != EOF && c[0] != '}') {
		if (c[0] == '\n' || c[0] == '\r')num_of_lines++;
	}
	if (c[0] == '}') {
		c[0] = getNextChar();
		goto LS0;
	}
	else {
		handle_error(COMMENT_ERROR, "");
	}
LS6:
	strcat(str, c);
	c[0] = getNextChar();
	if (c[0] == '=') {
		t.id = SYM;
		t.ptr = ASSI;//":="双字符分界符
		return t;
	}
	else {
		handle_error(SYM_ERROR, "符号为 \':\'");
	}
LS7://判断形如'hhh'的字符串
	while (c[0] = getNextChar(), c[0] != EOF && c[0] != '\'') {
		strcat(str, c);
	}
	if (c[0] == '\'') {
		t.id = STR;
		t.ptr = addSTRTable(str);
		return t;
	}
	else {
		handle_error(STR_ERROR, "");
	}
LS8://换行
	t.id = -1;
	num_of_lines++;
	return t;
LS9://文件流尾部
	t.id = SYM;
	t.ptr = EOFF;
	return t;
LS10://空格与制表符(\t)词法分析直接跳过
	t.id = -1;
	return t;
OTHER:
	handle_error(NOT_REC_SYMBOL, c);
	t.id = -1;
	return t;
}
void show_token(Token t) {
	if (t.id == STR) {
		printf("<%d %s %s> \n", t.linenum, "STR", STR_table[t.ptr]);
		return;
	}
	if (t.id == NUM) {
		printf("<%d %s %d> \n", t.linenum, "NUM", NUM_table[t.ptr]);
		return;
	}
	if (t.id == ID) {
		printf("<%d %s %s> \n", t.linenum, "ID", ID_table[t.ptr]);
		return;
	}
	if (t.id == RES) {
		printf("<%d %s %s> \n", t.linenum, "RES", reserved_words[t.ptr]);
		return;
	}
	if (t.id == SYM) {
		printf("<%d %s %s> \n", t.linenum, "SYM", symbol_table[t.ptr - 100]);
		return;
	}
	return;
}
vector<Token>*  getTokenlist(const char* filename) {
	tokens->clear();
	file = fopen(filename, "r");
	Token temp;
	while (temp = scan(), !(temp.id == SYM && temp.ptr == EOFF)) {
		if (temp.id != -1) {
			tokens->push_back(temp);
			if (temp.id == RES && strcmp("end", reserved_words[temp.ptr]) == 0) {
				char c;
				if (c = getNextChar(), c == '.') {
					break;
				}
				else {
					ungetNextChar();
				}
			}
		}
	}
	fclose(file);
	return tokens;
}
void show_Token_list() {
	int len = tokens->size();
	for (int i = 0; i < len; i++) {
		show_token(tokens->at(i));
	}
}

void write_token(FILE* file, Token t) {
	if (t.id == STR) {
		fprintf(file, "<%s %s> \n", "STR", STR_table[t.ptr]);
		return;
	}
	if (t.id == NUM) {
		fprintf(file, "<%s %d> \n", "NUM", NUM_table[t.ptr]);
		return;
	}
	if (t.id == ID) {
		fprintf(file, "<%s %s> \n", "ID", ID_table[t.ptr]);
		return;
	}
	if (t.id == RES) {
		fprintf(file, "<%s %s> \n", "RES", reserved_words[t.ptr]);
		return;
	}
	if (t.id == SYM) {
		fprintf(file, "<%s %s> \n", "SYM", symbol_table[t.ptr - 100]);
		return;
	}
	return;
}
void save_Token_list(const char* filename) {
	FILE* f = fopen(filename, "w");
	int len = tokens->size();
	for (int i = 0; i < len; i++) {
		write_token(f, tokens->at(i));
	}
	fclose(f);
}






/***************************************************************************************/
/*****************                   语法分析                          *****************/
/***************************************************************************************/

void syntaxError(const char* message) {
	if (strcmp(message, "outofrange") == 0) {
		printf("出现越界错误");
		printf("   Token_n:%d    tokens->size:%d\n", Token_n, tokens->size());
		exit(-1);
	}
	printf("行号：%d    %s", tokens->at(Token_n).linenum, message);
	printf("当前token：");
	show_token(tokens->at(Token_n));
	exit(-1);
}
Node* MatchRES(const char* expected) {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id, ptr = tokens->at(Token_n).ptr;
	if (id == RES && strcmp(expected, reserved_words[ptr]) == 0) {
		Node*t = new Node(expected);
		Token_n++;
		return t;
	}
	char* message = new char[100];
	strcpy(message, "match res error.\nexpect: ");
	strcat(message, expected);
	strcat(message, "\n");
	syntaxError(message);
	delete message;
	return NULL;
}
Node* MatchNUM() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == NUM) {
		char Num[32];
		itoa(NUM_table[(tokens->at(Token_n)).ptr], Num, 10);
		Node* t = new Node(Num);
		Token_n++;
		return t;
	}
	syntaxError("match num error\n");
	return NULL;
}
Node* MatchSTR() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == STR) {
		Node* t = new Node(STR_table[(tokens->at(Token_n)).ptr]);
		Token_n++;
		return t;
	}
	syntaxError("match str error\n");
	return NULL;
}
Node* MatchID() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == ID) {
		Node* t = new Node(ID_table[(tokens->at(Token_n)).ptr]);
		Token_n++;
		return t;
	}
	syntaxError("match id error\n");
	return NULL;
}
Node* MatchSYM(const int expected) {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id, ptr = tokens->at(Token_n).ptr;
	if (id == SYM && ptr == expected) {
		Node* t = new Node(symbol_table[ptr - 100]);
		Token_n++;
		return t;
	}
	char* message = new char[100];
	strcpy(message, "match symbol error.\nexpect: ");
	strcat(message, symbol_table[expected - 100]);
	strcat(message, "\n");
	syntaxError(message);
	delete message;
	return NULL;
}
bool isRES(const char* res) {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id, ptr = tokens->at(Token_n).ptr;
	if (id == RES && strcmp(reserved_words[ptr], res) == 0)return true;
	return false;
}
bool isSYM(const int res) {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id, ptr = tokens->at(Token_n).ptr;
	if (id == SYM && ptr == res)return true;
	return false;
}
bool isID() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == ID)return true;
	return false;
}
bool isNUM() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == NUM)return true;
	return false;
}
bool isSTR() {
	if (Token_n >= (int)tokens->size())syntaxError("outofrange");
	int id = tokens->at(Token_n).id;
	if (id == STR)return true;
	return false;
}
Node* Program() {
	Node* t = new Node("Program");
	add(t->children, ProgramHead());
	add(t->children, DeclarePart());
	add(t->children, ProgramBody());
	return t;
}
Node* ProgramHead() {
	Node* t = new Node("ProgramHead");
	add(t->children, MatchRES("program"));
	add(t->children, MatchID());
	return t;
}

Node* DeclarePart() {
	Node* t = new Node("DeclarePart");
	add(t->children, TypeDeclaration());
	add(t->children, VarDeclaration());
	add(t->children, ProcDeclaration());
	return t;
}
Node* TypeDec() {
	Node* t = new Node("TypeDec");
	add(t->children, MatchRES("type"));
	add(t->children, TypeDecList());
	return t;
}
Node* TypeDeclaration() {
	Node*t = NULL;
	if (isRES("type")) {
		t = new Node("TypeDeclaration");
		add(t->children, TypeDec());
	}
	return t;
}
Node* TypeDecList() {
	Node* t = new Node("TypeDecList");
	add(t->children, TypeId());
	add(t->children, MatchSYM(EQU));
	add(t->children, TypeName());
	add(t->children, MatchSYM(SEMI));
	add(t->children, TypeDecMore());
	return t;
}
Node* TypeDecMore() {
	Node*t = NULL;
	if (isID()) {
		t = new Node("TypeDecMore");
		add(t->children, TypeDecList());
	}
	return t;
}
Node* TypeId() {
	Node* t = new Node("TypeId");
	add(t->children, MatchID());
	return t;
}
Node* TypeName() {
	Node* t = new Node("TypeName");
	if (isRES("integer") || isRES("char")) {
		add(t->children, BaseType());
		return t;
	}
	if (isRES("array") || isRES("record")) {
		add(t->children, StructureType());
		return t;
	}
	add(t->children, MatchID());
	return t;
}
Node* BaseType() {
	Node*t = NULL;
	if (isRES("integer")) {
		t = new Node("BaseType");
		add(t->children, MatchRES("integer"));
		return t;
	}
	if (isRES("char")) {
		t = new Node("BaseType");
		add(t->children, MatchRES("char"));
		return t;
	}
	syntaxError("not a base type\n");
	return t;
}
Node* StructureType() {
	Node*t = NULL;
	if (isRES("array")) {
		t = new Node("StructureType");
		add(t->children, ArrayType());
		return t;
	}
	if (isRES("record")) {
		t = new Node("StructureType");
		add(t->children, RecType());
		return t;
	}
	return t;
}
Node* ArrayType() {
	Node* t = new Node("ArrayType");
	add(t->children, MatchRES("array"));
	add(t->children, MatchSYM(LSQUBRACK));
	add(t->children, MatchNUM());
	add(t->children, MatchSYM(INDEX));
	add(t->children, MatchNUM());
	add(t->children, MatchSYM(RSQUBRACK));
	add(t->children, MatchRES("of"));
	add(t->children, BaseType());
	return t;
}
Node* RecType() {
	Node* t = new Node("RecType");
	add(t->children, MatchRES("record"));
	add(t->children, FieldDecList());
	add(t->children, MatchRES("end"));
	return t;
}
Node* FieldDecList() {
	Node*t = NULL;
	if (isRES("integer") || isRES("char")) {
		t = new Node("FieldDecList");
		add(t->children, BaseType());
		add(t->children, IdList());
		add(t->children, MatchSYM(SEMI));
		add(t->children, FieldDecMore());
		return t;
	}
	if (isRES("array")) {
		t = new Node("FieldDecList");
		add(t->children, ArrayType());
		add(t->children, IdList());
		add(t->children, MatchSYM(SEMI));
		add(t->children, FieldDecMore());
		return t;
	}
	syntaxError("field declare list error\n");
	return t;
}
Node* FieldDecMore() {
	Node*t = NULL;
	if (isRES("integer") || isRES("char") || isRES("array")) {
		t = new Node("FieldDecMore");
		add(t->children, FieldDecList());
	}
	return t;
}
Node* IdList() {
	Node* t = new Node("IdList");
	add(t->children, MatchID());
	add(t->children, IdMore());
	return t;
}
Node* IdMore() {
	Node*t = NULL;
	if (isSYM(COM)) {
		t = new Node("IdMore");
		add(t->children, MatchSYM(COM));
		add(t->children, IdList());
	}
	return t;
}
Node* VarDec() {
	Node* t = new Node("VarDec");
	add(t->children, MatchRES("var"));
	add(t->children, VarDecList());
	return t;
}
Node* VarDeclaration() {
	Node*t = NULL;
	if (isRES("var")) {
		t = new Node("VarDeclaration");
		add(t->children, VarDec());
	}
	return t;
}
Node* VarDecList() {
	Node* t = new Node("VarDecList");
	add(t->children, TypeName());
	add(t->children, VarIdList());
	add(t->children, MatchSYM(SEMI));
	add(t->children, VarDecMore());
	return t;
}
Node* VarDecMore() {
	Node*t = NULL;
	if (isRES("integer") || isRES("char") || isRES("array") || isRES("record") || isID()) {
		t = new Node("VarDecMore");
		add(t->children, VarDecList());
	}
	return t;
}
Node* VarIdList() {
	Node* t = new Node("VarIdList");
	add(t->children, MatchID());
	add(t->children, VarIdMore());
	return t;
}
Node* VarIdMore() {
	Node*t = NULL;
	if (isSYM(COM)) {
		t = new Node("VarIdMore");
		add(t->children, MatchSYM(COM));
		add(t->children, VarIdList());
	}
	return t;
}
Node* ProcDec() {
	Node* t = new Node("ProcDec");
	add(t->children, MatchRES("procedure"));
	add(t->children, MatchID());
	add(t->children, MatchSYM(LBRACK));
	add(t->children, ParamList());
	add(t->children, MatchSYM(RBRACK));
	add(t->children, MatchSYM(SEMI));
	add(t->children, DeclarePart());
	add(t->children, ProcBody());
	add(t->children, ProcDeclaration());
	return t;
}
Node* ProcDeclaration() {
	Node*t = NULL;
	if (isRES("procedure")) {
		t = new Node("ProcDeclaration");
		add(t->children, ProcDec());
	}
	return t;
}
Node* ParamList() {
	Node*t = NULL;
	if (isRES("integer") || isRES("char") || isRES("array") || isRES("record") || isID() || isRES("var")) {
		t = new Node("ParamList");
		add(t->children, ParamDecList());
	}
	return t;
}
Node* ParamDecList() {
	Node* t = new Node("ParamDecList");
	add(t->children, Param());
	add(t->children, ParamMore());
	return t;
}
Node* ParamMore() {
	Node*t = NULL;
	if (isSYM(SEMI)) {
		t = new Node("ParamMore");
		add(t->children, MatchSYM(SEMI));
		add(t->children, ParamDecList());
	}
	return t;
}
Node* Param() {
	Node*t = NULL;
	if (isRES("integer") || isRES("char") || isRES("array") || isRES("record") || isID()) {
		t = new Node("Param");
		add(t->children, TypeName());
		add(t->children, FormList());
		return t;
	}
	if (isRES("var")) {
		t = new Node("Param");
		add(t->children, MatchRES("var"));
		add(t->children, TypeName());
		add(t->children, FormList());
		return t;
	}
	syntaxError("param error\n");
	return t;
}
Node* FormList() {
	Node* t = new Node("FormList");
	add(t->children, MatchID());
	add(t->children, FidMore());
	return t;
}
Node* FidMore() {
	Node*t = NULL;
	if (isSYM(COM)) {
		t = new Node("FidMore");
		add(t->children, MatchSYM(COM));
		add(t->children, FormList());
	}
	return t;
}
Node* ProcDecPart() {
	Node* t = new Node("ProcDecPart");
	add(t->children, DeclarePart());
	return t;
}
Node* ProcBody() {
	Node* t = new Node("ProcBody");
	add(t->children, ProgramBody());
	return t;
}
Node* ProgramBody() {
	Node* t = new Node("ProgramBody");
	add(t->children, MatchRES("begin"));
	add(t->children, StmList());
	add(t->children, MatchRES("end"));
	return t;
}
Node* StmList() {
	Node* t = new Node("StmList");
	add(t->children, Stm());
	add(t->children, StmMore());
	return t;
}
Node* StmMore() {
	Node*t = NULL;
	if (isSYM(SEMI)) {
		t = new Node("StmMore");
		add(t->children, MatchSYM(SEMI));
		add(t->children, StmList());
	}
	return t;
}
Node* Stm() {
	Node* t = new Node("Stm");
	if (isRES("if")) {
		add(t->children, ConditionalStm());
		return t;
	}
	if (isRES("while")) {
		add(t->children, LoopStm());
		return t;
	}
	if (isRES("read")) {
		add(t->children, InputStm());
		return t;
	}
	if (isRES("write")) {
		add(t->children, OutputStm());
		return t;
	}
	if (isRES("return")) {
		add(t->children, ReturnStm());
		return t;
	}
	if (isID()) {
		add(t->children, MatchID());
		add(t->children, AssCall());
		return t;
	}
	delete t;
	return NULL;
}
Node* AssCall() {
	Node*t = NULL;
	if (isSYM(LSQUBRACK) || isSYM(POINT) || isSYM(ASSI)) {
		t = new Node("AssCall");
		add(t->children, AssignmentRest());
		return t;
	}
	if (isSYM(LBRACK)) {
		t = new Node("AssCall");
		add(t->children, CallStmRest());
		return t;
	}
	syntaxError("ass call error\n");
	return t;
}
Node* AssignmentRest() {
	Node* t = new Node("AssignmentRest");
	if (isSYM(LSQUBRACK) || isSYM(POINT)) {
		add(t->children, VariMore());
	}
	add(t->children, MatchSYM(ASSI));
	add(t->children, Exp());
	return t;
}
Node* ConditionalStm() {
	Node* t = new Node("ConditionalStm");
	add(t->children, MatchRES("if"));
	add(t->children, Exp());
	if (isSYM(LT))add(t->children, MatchSYM(LT));
	else if (isSYM(EQU))add(t->children, MatchSYM(EQU));
	else syntaxError("condition error\n");
	add(t->children, Exp());
	add(t->children, MatchRES("then"));
	add(t->children, StmList());
	add(t->children, MatchRES("else"));
	add(t->children, StmList());
	add(t->children, MatchRES("fi"));
	return t;
}
Node* LoopStm() {
	Node* t = new Node("LoopStm");
	add(t->children, MatchRES("while"));
	add(t->children, Exp());
	if (isSYM(LT))add(t->children, MatchSYM(LT));
	else if (isSYM(EQU))add(t->children, MatchSYM(EQU));
	else syntaxError("condition error\n");
	add(t->children, Exp());
	add(t->children, MatchRES("do"));
	add(t->children, StmList());
	add(t->children, MatchRES("endwh"));
	return t;
}
Node* InputStm() {
	Node* t = new Node("InputStm");
	add(t->children, MatchRES("read"));
	add(t->children, MatchSYM(LBRACK));
	add(t->children, MatchID());
	add(t->children, MatchSYM(RBRACK));
	return t;
}
Node* OutputStm() {
	Node* t = new Node("OutputStm");
	add(t->children, MatchRES("write"));
	add(t->children, MatchSYM(LBRACK));
	add(t->children, Exp());
	add(t->children, MatchSYM(RBRACK));
	return t;
}
Node* ReturnStm() {
	Node* t = new Node("ReturnStm");
	add(t->children, MatchRES("return"));
	return t;
}
Node* CallStmRest() {
	Node* t = new Node("CallStmRest");
	add(t->children, MatchSYM(LBRACK));
	add(t->children, ActParamList());
	add(t->children, MatchSYM(RBRACK));
	return t;
}
Node* ActParamList() {
	Node*t = NULL;
	if (isSYM(LBRACK) || isNUM() || isID()) {
		t = new Node("ActParamList");
		add(t->children, Exp());
		add(t->children, ActParamMore());
	}
	return t;
}
Node* ActParamMore() {
	Node*t = NULL;
	if (isSYM(COM)) {
		t = new Node("ActParamMore");
		add(t->children, MatchSYM(COM));
		add(t->children, ActParamList());
	}
	return t;
}
Node* Exp() {
	Node* t = new Node("Exp");
	add(t->children, Term());
	add(t->children, OtherTerm());
	return t;
}
Node* OtherTerm() {
	Node*t = NULL;
	if (isSYM(PLUS) || isSYM(SUB)) {
		t = new Node("OtherTerm");
		add(t->children, AddOp());
		add(t->children, Exp());
	}
	return t;
}
Node* Term() {
	Node* t = new Node("Term");
	add(t->children, Factor());
	add(t->children, OtherFactor());
	return t;
}
Node* OtherFactor() {
	Node*t = NULL;
	if (isSYM(MUL) || isSYM(DIV)) {
		t = new Node("OtherFactor");
		add(t->children, MultOp());
		add(t->children, Term());
	}
	return t;
}
Node* Factor() {
	Node* t = new Node("Factor");
	if (isNUM()) {
		add(t->children, MatchNUM());
		return t;
	}
	if (isSTR()) {
		add(t->children, MatchSTR());
		return t;
	}
	if (isSYM(LBRACK)) {
		add(t->children, MatchSYM(LBRACK));
		add(t->children, Exp());
		add(t->children, MatchSYM(RBRACK));
		return t;
	}
	if (isID()) {
		add(t->children, Variable());
		return t;
	}
	delete t;
	syntaxError("factor error\n");
	return NULL;
}
Node* Variable() {
	Node* t = new Node("Variable");
	add(t->children, MatchID());
	add(t->children, VariMore());
	return t;
}
Node* VariMore() {
	Node*t = NULL;
	if (isSYM(LSQUBRACK)) {
		t = new Node("VariMore");
		add(t->children, MatchSYM(LSQUBRACK));
		add(t->children, Exp());
		add(t->children, MatchSYM(RSQUBRACK));
	}
	if (isSYM(POINT)) {
		t = new Node("VariMore");
		add(t->children, MatchSYM(POINT));
		add(t->children, FieldVar());
	}
	return t;
}
Node* FieldVar() {
	Node* t = new Node("FieldVar");
	add(t->children, MatchID());
	add(t->children, FieldVarMore());
	return t;
}
Node* FieldVarMore() {
	Node*t = NULL;
	if (isSYM(LSQUBRACK)) {
		t = new Node("FieldVarMore");
		add(t->children, MatchSYM(LSQUBRACK));
		add(t->children, Exp());
		add(t->children, MatchSYM(RSQUBRACK));
	}
	return t;
}
Node* CmpOp() {
	Node*t = NULL;
	if (isSYM(LT)) {
		t = new Node("CmpOp");
		add(t->children, MatchSYM(LT));
		return t;
	}
	if (isSYM(EQU)) {
		t = new Node("CmpOp");
		add(t->children, MatchSYM(EQU));
		return t;
	}
	syntaxError("cmpop error\n");
	return t;
}
Node* AddOp() {
	Node*t = NULL;
	if (isSYM(PLUS)) {
		t = new Node("AddOp");
		add(t->children, MatchSYM(PLUS));
		return t;
	}
	if (isSYM(SUB)) {
		t = new Node("AddOp");
		add(t->children, MatchSYM(SUB));
		return t;
	}
	syntaxError("cmpop error\n");
	return t;
}
Node* MultOp() {
	Node*t = NULL;
	if (isSYM(MUL)) {
		t = new Node("MultOp");
		add(t->children, MatchSYM(MUL));
		return t;
	}
	if (isSYM(DIV)) {
		t = new Node("MultOp");
		add(t->children, MatchSYM(DIV));
		return t;
	}
	syntaxError("cmpop error\n");
	return t;
}
Node* Parse() {
	num_of_lines = 1;
	Node* root = Program();
	return root;
}
void show_tree(Node* root, int depth, vector<int>* v, bool islast) {
	if (root == NULL)return;
	printf("\n");//v为打印树形结构时的辅助容器
	for (int i = 0; i < depth; i++) {//行打印，v->at(i)==1的是存在分支的列
		if (v->at(i) == 1)printf(" │");
		else printf("  ");
	}
	if (islast) {//无兄弟节点，到达底部，该列(层)不需再打印" │"，故将v->at(depth)置0
		printf(" └─ %s", root->desc);
		v->at(depth) = 0;//该行再无节点
	}
	else printf(" ├─ %s", root->desc);//还有兄弟节点
	if (depth + 1 == (int)v->size())v->push_back(1);//当前深度已达历史最大深度，为下次调用tree，需要v辅助容器大小+1
	v->at(depth + 1) = 1;//下次(更深层)调用tree时，若无兄弟节点则会被重新置0，若有兄弟节点，则在depth+2时按行打印会在depth+1列打印出" │"
	int len = root->children.size();//孩子节点个数(孩子之间互为兄弟，其实也就是该列有几个分支/兄弟)
	for (int i = 0; i < len; i++) {
		if (i == len - 1)show_tree(root->children.at(i), depth + 1, v, true);//最后一个孩子节点(该列到底)
		else show_tree(root->children.at(i), depth + 1, v, false);//仍有兄弟节点
	}
}

int main() {
	tokens = new vector<Token>();
	getTokenlist("cs1.txt");
	save_Token_list("tokens.txt");
	//打印显示
	printf("TOKENS:\n\n");
	show_Token_list();
	printf("\n\n\n\n语法树:\n\n");
	vector<int>* v = new vector<int>();
	v->push_back(0);
	show_tree(Parse(), 0, v, true);
	return 0;
}