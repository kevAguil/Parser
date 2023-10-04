/* Implementation of Recursive-Descent Parser
 * parser.cpp
 * Programming Assignment 2
 * Spring 2023
*/

#include "parser.h"

map<string, bool> defVar;
map<string, Token> SymTable;

namespace Parser {
    bool pushed_back = false;
    LexItem    pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << error_count << ". Line # " << line << ": " << msg << endl;
}



//Prog ::= StmtList
bool Prog(istream& in, int& line){
    bool status = false;
    status = StmtList(in, line);
    if (!status) {
        ParseError(line, "Missing Statement List in Prog");
        return false;
    }
    if(ErrCount() == 0){
        cout << "(DONE)" << endl;
        return true;
    }
    return status;
}// End of Prog




//StmtList ::= Stmt ;{ Stmt; }
bool StmtList(istream& in, int& line){
    bool status = false;
    
    status = Stmt(in, line);
    if(!status){
        ParseError(line, "Missing Statement in program");
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != SEMICOL){
        ParseError(line, "Missing Semicolon");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if (tok == RBRACES) {
        Parser::PushBackToken(tok);
        return true;
    }
    if (tok != DONE) {
        Parser::PushBackToken(tok);
        return StmtList(in, line);
    }
    
    return true;
}// End of StmtList




//Stmt ::= AssignStme | WriteLnStmt | IfStmt
bool Stmt(istream& in, int& line) {
    bool status = false;
    LexItem tok = Parser::GetNextToken(in, line);
    
    
    
    if (tok == IF) {
        status = IfStmt(in, line);
        if (!status) {
            ParseError(line, "Invalid IF Statement");
            return false;
        }
    }

    else if (tok == WRITELN) {
        status = WritelnStmt(in, line);
        if (!status) {
            ParseError(line, "Invalid  WRITELN Statement");
            return false;
        }
    }

    else {
        Parser::PushBackToken(tok);
        status = AssignStmt(in, line);
        if (!status) {
            ParseError(line, "Invalid Assignment Statement");
            return false;
        }
    }

    return true;
}//End of Stmt




//WritelnStmt:= WRITELN (ExpreList)
bool WritelnStmt(istream& in, int& line) {
    LexItem t;
    //cout << "in Writeln Stmt" << endl;
    
    t = Parser::GetNextToken(in, line);
    if( t != LPAREN ) {
        
        ParseError(line, "Missing Left Parenthesis of Writeln Statement");
        return false;
    }
    
    bool ex = ExprList(in, line);
    
    if( !ex ) {
        ParseError(line, "Missing expression list after Print");
        return false;
    }
    
    t = Parser::GetNextToken(in, line);
    if(t != RPAREN ) {
        
        ParseError(line, "Missing Right Parenthesis of Writeln Statement");
        return false;
    }
    //Evaluate: print out the list of expressions values

    return true;
}//End of WritelnStmt





//IfStmt:=IF (Expr) {StmtList} [ELSE {StmtList}]
bool IfStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    bool status = false;
    
    if( tok != LPAREN ) {
    
        ParseError(line, "Missing Left Parenthesis in If Statement");
        return false;
    }

    status = Expr(in, line);
    if( !status ) {
        ParseError(line, "Missing expression in If Statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if( tok != RPAREN ) {
    
        ParseError(line, "Missing Right Parenthesis in If Statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if( tok != LBRACES ) {
    
        ParseError(line, "Missing Left Brace in If Statement");
        return false;
    }

    status = StmtList(in, line);
    if( !status ) {
        ParseError(line, "Missing statement list in If Statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if( tok != RBRACES ) {
    
        ParseError(line, "Missing Right Brace in If Statement");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if( tok == ELSE ) {
        tok = Parser::GetNextToken(in, line);
        if( tok != LBRACES ) {
            ParseError(line, "Missing Left Brace in Else Statement");
            return false;
        }
    
        status = StmtList(in, line);
        if( !status ) {
            ParseError(line, "Missing statement list in Else Statement");
            return false;
        }
    
        tok = Parser::GetNextToken(in, line);
        if( tok != RBRACES ) {
            ParseError(line, "Missing Right Brace in Else Statement");
            return false;
        }
    }
    else{
        Parser::PushBackToken(tok);
    }

    return true;
}//End of IfStmt






//AssignStmt ::= Var = Expr
bool AssignStmt(istream& in, int& line) {
    bool status = false;

    status = Var(in, line);
    if (!status) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    string str = tok.GetLexeme();
    tok = Parser::GetNextToken(in, line);
    if (tok != ASSOP) {
        defVar[str] = false;
        Parser::PushBackToken(tok);
        return true;
    }

    status = Expr(in, line);
    if (!status) {
        ParseError(line, "Missing Expression in Assign Statement");
        return false;
    }

    defVar[str] = true;
    return true;
}//End of AssignStmt




//Var ::= NIDENT | SIDENT
bool Var(istream& in, int& line){
    
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok != NIDENT && tok != SIDENT) {
        Parser::PushBackToken(tok);
        return false;
    }
    
    Parser::PushBackToken(tok);
    return true;
}//End of Var




//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    //cout << "in ExprList and before calling Expr" << endl;
    status = Expr(in, line);
    if(!status){
        ParseError(line, "Missing Expression");
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok == COMMA) {
        //cout << "before calling ExprList" << endl;
        status = ExprList(in, line);
        //cout << "after calling ExprList" << endl;
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}//End of ExprList




//Expr ::= RelExpr [(-eq|==) RelExpr ]
bool Expr(istream& in, int& line) {
    bool status = false;
    
    status = RelExpr(in, line);
    if(!status){
        return false;
    }
    
    LexItem tok = Parser:: GetNextToken(in, line);
    
    if(tok == NEQ || tok == SEQ){
        status = RelExpr(in, line);
        if(!status){
            ParseError(line, "Missing Real Expression after"+ tok.GetLexeme());
            return false;
        }
    }
    else{
        Parser:: PushBackToken(tok);
       
    }
    return true;
}//End of Expr




//RelExpr ::= AddExpr [ ( -lt | -gt | < | > ) AddExpr ]
bool RelExpr(istream& in, int& line){
    bool status = false;
    
    status = AddExpr(in, line);
    if(!status){
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == SLTHAN || tok == SGTHAN || tok == NLTHAN || tok == NGTHAN){
        status = AddExpr(in, line);
        if(!status){
            ParseError(line, "Missing Add Expression after "+ tok.GetLexeme());
            return false;
        }
    }
    else{
        Parser::PushBackToken(tok);
    }
    
    return true;
}//End of RelExpr



//AddExpr :: MultExpr { ( + | - | .) MultExpr }
bool AddExpr(istream& in, int& line){
    bool status = false;
    
    status = MultExpr(in, line);
    if(!status){
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    while(tok == PLUS || tok == MINUS || tok == CAT ){
        
        status = MultExpr(in, line);
        if(!status){
            ParseError(line, "Missing Mult Expression after " + tok.GetLexeme());
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        
    }
    Parser::PushBackToken(tok);
    return true;
}//End of AddExpr



//MultExpr ::= ExponExpr { ( * | / | **) ExponExpr }
bool MultExpr(istream& in, int& line){
    bool status = false;
    
    status = ExponExpr(in, line);
    if(!status){
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    while(tok == MULT || tok == DIV || tok == SREPEAT){
        
        status = ExponExpr(in, line);
        if(!status){
            ParseError(line, "Missing Expon Expression after "+ tok.GetLexeme());
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        
    }
    Parser::PushBackToken(tok);
    return true;
}//End of MultExpr



//ExponExpr ::= UnaryExpr { ^ UnaryExpr }
bool ExponExpr(istream& in, int& line){
    bool status = false;
    
    status = UnaryExpr(in, line);
    if (!status){
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    while(tok == EXPONENT){
        
        status = UnaryExpr(in, line);
        if(!status){
            ParseError(line, "Missing Unary expression after " + tok.GetLexeme());
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}//End of ExponExpr



//UnaryExpr ::= [( - | + )] PrimaryExpr
bool UnaryExpr(istream& in, int& line){
    bool status = false;
    LexItem tok = Parser::GetNextToken(in, line);
    
    if(tok == PLUS){
        status = PrimaryExpr(in, line, 1);
        return status;
    }
    else if (tok == MINUS){
        status = PrimaryExpr(in, line, 0);
        return status;
    }
    else{
        Parser::PushBackToken(tok);

    }
  
    return PrimaryExpr(in,line,1);
}//End of UnaryExpr



//PrimaryExpr ::= IDENT | SIDENT | NIDENT | ICONST | RCONST | SCONST | (Expr)
bool PrimaryExpr(istream& in, int& line, int sign) {
    LexItem tok  = Parser::GetNextToken(in, line);

    if (tok == IDENT || tok == ICONST || tok == RCONST || tok == SCONST) {
        return true;
    }

    else if (tok == SIDENT || tok == NIDENT) {
        if (defVar[tok.GetLexeme()] != true) {
            ParseError(line, "Undefined Variable");
            return false;
        }
        return true;
    }

    else if (tok == LPAREN) {
        bool status = Expr(in, line);
        if (!status){
            ParseError(line, "Missing expression after Left Parenthesis");
            return false;
        }
        if (Parser::GetNextToken(in, line) == RPAREN)
            return status;
        else
        {
            
            ParseError(line, "Missing right Parenthesis after expression");
            return false;
        }
    }
    
    else {
        return false;
    }
}//End of PrimaryExpr

