#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
/*

   _____              _        _                   __  __
  / ____|     /\     | |      | |          /\     |  \/  |
 | (___      /  \    | |      | |         /  \    | \  / |
  \___ \    / /\ \   | |      | |        / /\ \   | |\/| |
  ____) |  / ____ \  | |____  | |____   / ____ \  | |  | |
 |_____/  /_/    \_\ |______| |______| /_/    \_\ |_|  |_|

     Free Palestine
 Ahmed Sallam 20210614
 Ahmed Alaa 20200029
*/



using namespace std;

/*
{ Sample program
  in TINY language
  compute factorial
}

read x; {input an integer}
if 0<x then {compute only if x>=1}
  fact:=1;
  repeat
    fact := fact * x;
    x:=x-1
  until x=0;
  write fact {output factorial}
end
*/

// sequence of statements separated by ;
// no procedures - no declarations
// all variables are integers
// variables are declared simply by assigning values to them :=
// if-statement: if (boolean) then [else] end
// repeat-statement: repeat until (boolean)
// boolean only in if and repeat conditions < = and two mathematical expressions
// math expressions integers only, + - * / ^
// I/O read write
// Comments {}

////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char* a, const char* b)
{
    return strcmp(a, b)==0;
}

bool StartsWith(const char* a, const char* b)
{
    int nb=strlen(b);
    return strncmp(a, b, nb)==0;
}

void Copy(char* a, const char* b, int n=0)
{
    if(n>0) {strncpy(a, b, n); a[n]=0;}
    else strcpy(a, b);
}

void AllocateAndCopy(char** a, const char* b)
{
    if(b==0) {*a=0; return;}
    int n=strlen(b);
    *a=new char[n+1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile
{
    FILE* file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char* str) {file=0; if(str) file=fopen(str, "r"); cur_line_size=0; cur_ind=0; cur_line_num=0;}
    ~InFile(){if(file) fclose(file);}

    void SkipSpaces()
    {
        while(cur_ind<cur_line_size)
        {
            char ch=line_buf[cur_ind];
            if(ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char* str)
    {
        while(true)
        {
            SkipSpaces();
            while(cur_ind>=cur_line_size) {if(!GetNewLine()) return false; SkipSpaces();}

            if(StartsWith(&line_buf[cur_ind], str))
            {
                cur_ind+=strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine()
    {
        cur_ind=0; line_buf[0]=0;
        if(!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size=strlen(line_buf);
        if(cur_line_size==0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char* GetNextTokenStr()
    {
        SkipSpaces();
        while(cur_ind>=cur_line_size) {if(!GetNewLine()) return 0; SkipSpaces();}
        return &line_buf[cur_ind];
    }

    void Advance(int num)
    {
        cur_ind+=num;
    }
};

struct OutFile
{
    FILE* file;
    OutFile(const char* str) {file=0; if(str) file=fopen(str, "w");}
    ~OutFile(){if(file) fclose(file);}

    void Out(const char* s)
    {
        fprintf(file, "%s\n", s); fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo
{
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char* in_str, const char* out_str, const char* debug_str)
            : in_file(in_str), out_file(out_str), debug_file(debug_str)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType{
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    ASSIGN, EQUAL, LESS_THAN,
    PLUS, MINUS, TIMES, DIVIDE, POWER,
    SEMI_COLON,
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    ID, NUM,
    ENDFILE, ERROR
};

// Used for debugging only /////////////////////////////////////////////////////////
const char* TokenTypeStr[]=
        {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",
                "EndFile", "Error"
        };

struct Token
{
    TokenType type;
    char str[MAX_TOKEN_LEN+1];

    Token(){str[0]=0; type=ERROR;}
    Token(TokenType _type, const char* _str) {type=_type; Copy(str, _str);}
};

const Token reserved_words[]=
        {
                Token(IF, "if"),
                Token(THEN, "then"),
                Token(ELSE, "else"),
                Token(END, "end"),
                Token(REPEAT, "repeat"),
                Token(UNTIL, "until"),
                Token(READ, "read"),
                Token(WRITE, "write")
        };
const int num_reserved_words=sizeof(reserved_words)/sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[]=
        {
                Token(ASSIGN, ":="),
                Token(EQUAL, "="),
                Token(LESS_THAN, "<"),
                Token(PLUS, "+"),
                Token(MINUS, "-"),
                Token(TIMES, "*"),
                Token(DIVIDE, "/"),
                Token(POWER, "^"),
                Token(SEMI_COLON, ";"),
                Token(LEFT_PAREN, "("),
                Token(RIGHT_PAREN, ")"),
                Token(LEFT_BRACE, "{"),
                Token(RIGHT_BRACE, "}")
        };
const int num_symbolic_tokens=sizeof(symbolic_tokens)/sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch){return (ch>='0' && ch<='9');}
inline bool IsLetter(char ch){return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));}
inline bool IsLetterOrUnderscore(char ch){return (IsLetter(ch) || ch=='_');}

void GetNextToken(CompilerInfo* pci, Token* ptoken)
{
    ptoken->type=ERROR;
    ptoken->str[0]=0;

    int i;
    char* s=pci->in_file.GetNextTokenStr();
    if(!s)
    {
        ptoken->type=ENDFILE;
        ptoken->str[0]=0;
        return;
    }

    for(i=0;i<num_symbolic_tokens;i++)
    {
        if(StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if(i<num_symbolic_tokens)
    {
        if(symbolic_tokens[i].type==LEFT_BRACE)
        {
            pci->in_file.Advance(strlen(symbolic_tokens[i].str));
            if(!pci->in_file.SkipUpto(symbolic_tokens[i+1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type=symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    }
    else if(IsDigit(s[0]))
    {
        int j=1;
        while(IsDigit(s[j])) j++;

        ptoken->type=NUM;
        Copy(ptoken->str, s, j);
    }
    else if(IsLetterOrUnderscore(s[0]))
    {
        int j=1;
        while(IsLetterOrUnderscore(s[j])) j++;

        ptoken->type=ID;
        Copy(ptoken->str, s, j);

        for(i=0;i<num_reserved_words;i++)
        {
            if(Equals(ptoken->str, reserved_words[i].str))
            {
                ptoken->type=reserved_words[i].type;
                break;
            }
        }
    }

    int len=strlen(ptoken->str);
    if(len>0) pci->in_file.Advance(len);
}

enum NodeKind{
    IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
    OPER_NODE, NUM_NODE, ID_NODE
};

// Used for debugging only /////////////////////////////////////////////////////////
const char* NodeKindStr[]=
        {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
        };

enum ExprDataType {VOID, INTEGER, BOOLEAN};

// Used for debugging only /////////////////////////////////////////////////////////
const char* ExprDataTypeStr[]=
        {
                "Void", "Integer", "Boolean"
        };

#define MAX_CHILDREN 3

struct TreeNode
{
    TreeNode* child[MAX_CHILDREN];
    TreeNode* sibling; // used for sibling statements only

    NodeKind node_kind;

    union{TokenType oper; int num; char* id;}; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only


    TreeNode() {int i; for(i=0;i<MAX_CHILDREN;i++) child[i]=0; sibling=0; expr_data_type=VOID;}
};


TreeNode* mathexprFun(CompilerInfo *compInfo, Token *next_token);

// newexpr -> ( mathexpr ) | number | identifier
TreeNode* newexprFun(CompilerInfo* compInfo, Token *next_token)
{
    TreeNode* subTree=new TreeNode;
    // Case Type is "Identifier" in "( mathexpr ) | number | identifier"
     if(next_token->type==ID)
    {

        // Copy data in next token in subTree id
        AllocateAndCopy(&subTree->id, next_token->str);
        GetNextToken(compInfo, next_token);
        // Make type of node in subTree is ID_NODE
        subTree->node_kind=ID_NODE;
        return subTree;
    }// Case Type is "number" in "( mathexpr ) | number | identifier"
    else if(next_token->type==NUM)
    {

        // Copy data in next token in subTree num
        subTree->num = stoi(next_token->str);
        // Make type of node in subTree is NUM_NODE
        subTree->node_kind=NUM_NODE;
        GetNextToken(compInfo, next_token);

        return subTree;
    }

   // Case Type is LEFT_PAREN ( mathexpr ) in  "( mathexpr ) | number | identifier"
   else if(next_token->type == LEFT_PAREN)
    {
        GetNextToken(compInfo, next_token);
        subTree= mathexprFun(compInfo, next_token);
        GetNextToken(compInfo, next_token);
        return subTree;
    }
   // When newexpr not ID Or NUM or LEFT_PAREN
   else{
         return nullptr ;
   }

}

// factor -> newexpr { ^ newexpr }    right associative
TreeNode* factorFun(CompilerInfo* compInfo, Token *next_token)
{
   // Check first part "newexpr" in newexpr { ^ newexpr }
    TreeNode* subTree= newexprFun(compInfo, next_token);
   // Check second part "^" in newexpr { ^ newexpr }
    if(next_token->type==POWER)
    {
        TreeNode* subTreeTemp=new TreeNode;
       // Store the type of next token
        subTreeTemp->oper=next_token->type;
        //Store the tree of newexp part as first child of the tree
        subTreeTemp->child[0]=subTree;
        GetNextToken(compInfo, next_token);
        //Store the tree of factorFun as second child of the tree
        subTreeTemp->child[1]= factorFun(compInfo, next_token);
        // Make type of node in subTree is OPER_NODE
        subTreeTemp->node_kind=OPER_NODE;
        return subTreeTemp;
    }
    return subTree;
}

// term -> factor { (*|/) factor }    left associative
TreeNode* TermFun(CompilerInfo* compInfo, Token *next_token)
{
    // Check first part "factor" in factor { (*|/) factor }
    TreeNode* subTree= factorFun(compInfo, next_token);
    //check second part (*|/) in factor { (*|/) factor
    while(next_token->type==TIMES || next_token->type==DIVIDE)
    {
        TreeNode* subTreeTemp=new TreeNode;
        // Store the type of next token
        subTreeTemp->oper=next_token->type;
        //Store the tree of factor part as first child of the tree
        subTreeTemp->child[0]=subTree;
        GetNextToken(compInfo, next_token);
        //Store the tree of factorFun as second child of the tree
        subTreeTemp->child[1]= factorFun(compInfo, next_token);
        // Make type of node in subTree is OPER_NODE
        subTreeTemp->node_kind=OPER_NODE;
        subTree=subTreeTemp;
    }

    return subTree;
}

// mathexpr -> term { (+|-) term }    left associative
TreeNode* mathexprFun(CompilerInfo* compInfo, Token *next_token)
{
    // Check first part "term" in term { (+|-) term }
    TreeNode* subTree= TermFun(compInfo, next_token);
    //check second part (+|-) in term { (+|-) term }
    while(next_token->type==PLUS || next_token->type==MINUS)
    {
        TreeNode* subTreeTemp=new TreeNode;
        // Store the type of next token
        subTreeTemp->oper=next_token->type;
        //Store the tree of term part as first child of the tree
        subTreeTemp->child[0]=subTree;
        GetNextToken(compInfo, next_token);
        //Store the tree of term part as second child of the tree
        subTreeTemp->child[1]= TermFun(compInfo, next_token);
        // Make type of node in subTree is OPER_NODE
        subTreeTemp->node_kind=OPER_NODE;
        subTree=subTreeTemp;
    }
    return subTree;
}

// expr -> mathexpr [ (<|=) mathexpr ]
TreeNode* exprFun(CompilerInfo* compInfo, Token *next_token )
{

    // Check first part "mathexpr" in  mathexpr [ (<|=) mathexpr ]
    TreeNode* subTree= mathexprFun(compInfo, next_token);
    // Check second part (<|=) in  mathexpr [ (<|=) mathexpr ]
    if( next_token->type==LESS_THAN ||next_token->type==EQUAL )
    {
        TreeNode* subTreeTemp=new TreeNode;
        // Store the type of next token
        subTreeTemp->oper=next_token->type;
        //Store the tree of mathexpr part as first child of the tree
        subTreeTemp->child[0]=subTree;
        GetNextToken(compInfo, next_token);
        //Store the tree of mathexpr part as second child of the tree
        subTreeTemp->child[1]= mathexprFun(compInfo, next_token);
        subTreeTemp->node_kind=OPER_NODE;
        return subTreeTemp;
    }
    return subTree;
}

// writestmt -> write expr
TreeNode* writestmtFun(CompilerInfo* compInfo, Token *next_token)
{

    TreeNode* subTree=new TreeNode;

    // Check first part "write" in "write expr"
    if(next_token->type == WRITE){
        GetNextToken(compInfo, next_token);
    }
    // Check second part "expr" in First child in subTree child
    // in "write expr"
    subTree->child[0]= exprFun(compInfo, next_token);
    // Make type of node in subTree is WRITE_NODE
    subTree->node_kind=WRITE_NODE;
    return subTree;
}

// readstmt -> read identifier
TreeNode* readstmtFun(CompilerInfo* compInfo, Token *next_token)
{

    TreeNode* subTree=new TreeNode;
    // Check first part "read" in "read identifier"
    if(next_token->type == READ){
        GetNextToken(compInfo, next_token);
    }
    // Check second part "identifier" in "read identifier"
    if(next_token->type == ID){
        AllocateAndCopy(&subTree->id, next_token->str);
        GetNextToken(compInfo, next_token);
    }
    // Make type of node in subTree is READ_NODE
    subTree->node_kind=READ_NODE;
    return subTree;
}

// assignstmt -> identifier := expr
TreeNode* assignstmtFun(CompilerInfo* compInfo, Token *next_token)
{

    TreeNode* subTree=new TreeNode;
    // Check first part "identifier" in "identifier := expr"
    if(next_token->type == ID){
        // Copy data in next token in subTree id
        AllocateAndCopy(&subTree->id, next_token->str);
        GetNextToken(compInfo, next_token);
    }
    // Check second part  ":="  in "identifier := expr"
    if(next_token->type == ASSIGN){
        GetNextToken(compInfo, next_token);
    }
    // Check third part "expr" in First child in subTree child
    // in "identifier := expr"
    subTree->child[0]= exprFun(compInfo, next_token);
    // Make type of node in subTree is ASSIGN_NODE
    subTree->node_kind=ASSIGN_NODE;
    return subTree;
}

TreeNode* stmtseqFun(CompilerInfo *compInfo, Token *next_token);

// repeatstmt -> repeat stmtseq until expr
TreeNode* repeatstmtFun(CompilerInfo* pci, Token *next_token)
{

    TreeNode* subTree=new TreeNode;

    //Check first part  "repeat"  in repeat stmtseq until expr
    if(next_token->type == REPEAT){
        GetNextToken(pci, next_token);
    }
    //Check second part  "stmtseq" and put it in first subTree child  in
    // "repeat stmtseq until expr"
    subTree->child[0]= stmtseqFun(pci, next_token);
    //Check third part "UNTIL" in repeat stmtseq until expr
    if(next_token->type == UNTIL){
        GetNextToken(pci, next_token);
    }
    //Check fourth part  "exp" and put it in second subTree child  in
    // "repeat stmtseq until expr"
    subTree->child[1]= exprFun(pci, next_token);
    // Make type of node in subTree is REPEAT_NODE
    subTree->node_kind=REPEAT_NODE;
    return subTree;
}

// ifstmt -> if exp then stmtseq [ else stmtseq ] end
TreeNode* ifstmtFun(CompilerInfo* compInfo, Token *next_token)
{
    TreeNode* subTree=new TreeNode;

     //Check first part  "if"  in "if exp then stmtseq [ else stmtseq ] end"
    if(next_token->type == IF){
        GetNextToken(compInfo, next_token);
    }
    //Check second part "exp" and put it in first subTree child
    subTree->child[0]= exprFun(compInfo, next_token);
    //Check third part "then" in "if exp then stmtseq [ else stmtseq ] end"
    if(next_token->type == THEN){
        GetNextToken(compInfo, next_token);
    }
    //Check fourth part "stmtseq" and put it in second subTree child
    subTree->child[1]= stmtseqFun(compInfo, next_token);
    //Check fifth part "else" in "if exp then stmtseq [ else stmtseq ] end"
    if(next_token->type==ELSE) {
            GetNextToken(compInfo, next_token);
            //Check sixth part "stmtseq" and put it in third subTree child
           subTree->child[2]= stmtseqFun(compInfo, next_token);
    }
    //Check sixth part "END" in stmtseq [ else stmtseq ] end
    if(next_token->type == END){
        GetNextToken(compInfo, next_token);
    }
    // Make type of node in subTree is IF_NODE
    subTree->node_kind=IF_NODE;
    return subTree;
}

// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
TreeNode* stmtFun(CompilerInfo* compInfo, Token *next_token)
{

    TreeNode* subTree= nullptr;
    // If next token type is IF statement
    if(next_token->type==IF) subTree= ifstmtFun(compInfo, next_token);
        // If next token type is IDENTIFIER
    else if(next_token->type==ID) subTree= assignstmtFun(compInfo, next_token);

    // If next token type is REPEAT statement
    else if(next_token->type==REPEAT) subTree= repeatstmtFun(compInfo, next_token);
       // If next token type is WRITE statment
    else if(next_token->type==WRITE) subTree= writestmtFun(compInfo, next_token);
        // If next token type is READ statment
    else if(next_token->type==READ) subTree= readstmtFun(compInfo, next_token);
    // if stmt not IF , REPEAT , ID , WRITE ,READ
    else{
        return nullptr;
    }
    return subTree;
}

// stmtseq -> stmt { ; stmt }
TreeNode* stmtseqFun(CompilerInfo* compInfo, Token *next_token)
{
    //First part stmt
    TreeNode* left_part= stmtFun(compInfo, next_token);
    //Second part {; stmt}
    TreeNode* right_part=left_part;

    //  Will work until reach to ENDFILE or END of if or ELSE or UNTIL
    while(!(next_token->type==ENDFILE || next_token->type==ELSE   ||
            next_token->type==UNTIL|| next_token->type==END  ))
    {
       // To skip when meet SEMI_COLON
        if(next_token->type == SEMI_COLON){
            GetNextToken(compInfo, next_token);
        }
        // Call stmtFun  stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
        TreeNode* next_tree= stmtFun(compInfo, next_token);
        right_part->sibling=next_tree;
        right_part=next_tree;
    }

    return left_part;
}

//void PrintTree(TreeNode* node, int sh=0)
//{
//    int i, NSH=3;
//    for(i=0;i<sh;i++) printf(" ");
//
//    printf("[%s]", NodeKindStr[node->node_kind]);
//
//    if(node->node_kind==OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
//    else if(node->node_kind==NUM_NODE) printf("[%d]", node->num);
//    else if(node->node_kind==ID_NODE || node->node_kind==READ_NODE || node->node_kind==ASSIGN_NODE) printf("[%s]", node->id);
//
//    if(node->expr_data_type!=VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);
//
//    printf("\n");
//
//    for(i=0;i<MAX_CHILDREN;i++) if(node->child[i]) PrintTree(node->child[i], sh+NSH);
//    if(node->sibling) PrintTree(node->sibling, sh);
//}
void PrintTreeToFile(FILE* outputFile, TreeNode* node,int sh = 0) {
    int i, NSH = 3;
    for (i = 0; i < sh; i++) {
        fprintf(outputFile, " ");
    }

    fprintf(outputFile, "[%s]", NodeKindStr[node->node_kind]);

    if (node->node_kind == OPER_NODE) {
        fprintf(outputFile, "[%s]", TokenTypeStr[node->oper]);
    } else if (node->node_kind == NUM_NODE) {
        fprintf(outputFile, "[%d]", node->num);
    } else if (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE) {
        fprintf(outputFile, "[%s]", node->id);
    }

    if (node->expr_data_type != VOID) {
        fprintf(outputFile, "[%s]", ExprDataTypeStr[node->expr_data_type]);
    }

    fprintf(outputFile, "\n");

    for (i = 0; i < MAX_CHILDREN; i++) {
        if (node->child[i]) {
            PrintTreeToFile(outputFile, node->child[i], sh + NSH);
        }
    }

    if (node->sibling) {
        PrintTreeToFile(outputFile, node->sibling, sh);
    }
}

int main()
{
    CompilerInfo compiler_info("inFile.txt", "otFile.txt", "deFile.txt");
    // Here start parsing process and make start from first part
    // in Bnf grammar "program -> stmtseq"
    Token *next_token = new Token;
    GetNextToken(&compiler_info,next_token);
    TreeNode* syntax_tree= stmtseqFun(&compiler_info, next_token);
    FILE* outputFile = fopen("otFile.txt", "w");
    PrintTreeToFile(outputFile,syntax_tree,0);
    return 0;
}

