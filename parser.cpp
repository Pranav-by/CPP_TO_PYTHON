#include <memory>
#include <stdexcept>
#include "lexer.cpp"

class ExprAST{
public:
    virtual ~ExprAST(){}
    virtual void print() const = 0;
};

class StatementAST{
    public:
    virtual ~StatementAST(){}
    virtual void print() const = 0;
};

using ProgramAST = vector<unique_ptr<StatementAST>> ;

//Defintion of specific AST Node class
class NumberExprAST:public ExprAST{
    string value;
public:
    NumberExprAST(string val):value(val){}
    void print() const override{
        cout << "Number("<<value<<")";
    }
};

class VariableExprAST:public ExprAST{
    string name;
public:
    VariableExprAST(string n):name(n){}
    void print() const override{
        cout << "Variable("<<name<<")";
    }
};

class StringLiteralExprAST:public ExprAST{
    string value;
public:
    StringLiteralExprAST(string val):value(val){}
    void print() const override{
        cout << "String(\""<<value<<"\")";
    }
};

// Binary op e.g. a+b or x > 5
class BinaryExprAST:public ExprAST{
    string op;
    unique_ptr<ExprAST>left;
    unique_ptr<ExprAST>right;
public:
    BinaryExprAST(string o,unique_ptr<ExprAST>l,unique_ptr<ExprAST>r){
        op = o;
        left = move(l);
        right = move(r);
    }
    void print() const override{
        cout << "BinaryOp("<<op<<",";
        left->print();
        cout << ",";
        right->print();
        cout << ")";
    }
};

// Define Specific AST Node classes (statements):
class VarDeclAST:public StatementAST{ //e.g. int x = 10;
    string varType;
    string varName;
    unique_ptr<ExprAST>initialValue; // can be nullptr if no value is assigned
public:
    VarDeclAST(string type,string name,unique_ptr<ExprAST>value){
        varType = type;
        varName = name;
        initialValue = move(value);
    }
    void print() const override{
        cout << "VarDecl(Type: "<<varType << ",Name: "<<varName;
        if(initialValue){
            cout <<",Value: ";
            initialValue->print();
        }
        cout << ")\n";
    }
};

// statement node for assignment e.g. x = 20;
class AssignmentAST:public StatementAST{
    string varName;
    unique_ptr<ExprAST>value;
public:
    AssignmentAST(string name , unique_ptr<ExprAST>val){
        varName = name;
        value = move(val);
    }
    void print() const override{
        cout << "Assignment(Name: "<<varName<<",Value: ";
        value->print();
        cout << ")\n";
    }
};
// For cout:
class CoutStatementAST:public StatementAST{
    public:
    // A cout can have multiple '<<' parts
    vector<unique_ptr<ExprAST>>parts;
    bool hasEndl;

    CoutStatementAST():hasEndl(false){}

    void print() const override{
        cout << "CoutStatement(";
        for(size_t i = 0 ; i < parts.size();++i){
            parts[i]->print();
            if(i<parts.size()-1) cout <<",";
        }
        if(hasEndl){
            if(!parts.empty()) cout << ",";
            cout << "ENDL";
        }
        cout << ")\n";
    }
};
// Placeholder for 'if' statement
class IfStatementAST : public StatementAST {
public:
    unique_ptr<ExprAST>condition;
    vector<unique_ptr<StatementAST>>thenBody;
    vector<unique_ptr<StatementAST>>elseBody;
    vector<pair<unique_ptr<ExprAST>,vector<unique_ptr<StatementAST>>>>elseIfBlocks;

    void print() const override{
        cout << "IfStatement(Condition: ";
        condition->print();
        cout << ", Then: [";
        for(auto &s:thenBody) s->print();
        cout << "]";
        for(auto &block: elseIfBlocks){
            cout << " ElseIf(";
            block.first->print();
            cout <<", [";
            for(auto &s:block.second) s->print();
            cout << "])";
        }
        if(!elseBody.empty()){
            cout << " Else: [";
            for(auto &s:elseBody) s->print();
            cout <<"]";
        }
        cout << ")\n";
    }
};

// Placeholder for 'for' loop
class ForStatementAST : public StatementAST {
public:
    string init,condition,increment;
    vector<unique_ptr<StatementAST>>body;
    void print() const override{
        cout << "ForStatement(Init: "<<init <<", Cond: "<<condition<<", Inc: "<<increment<<", Body: [";
        for(auto &s:body) s->print();
        cout <<"])\n";
    }
};

// Placeholder for 'while' loop
class WhileStatementAST : public StatementAST {
public:
    unique_ptr<ExprAST> condition;
    vector<unique_ptr<StatementAST>> body;
    void print() const override{
        cout << "WhileStatement(Cond: ";
        condition->print();
        cout <<", Body: [";
        for(auto &s:body) s->print();
        cout << "])\n";
    }
};

// Parser Class
class Parser{
    vector<Token>tokens;
    int pos;

    Token &current(){
        return tokens[pos]; // current token;
    }

    Token &next(){
        return tokens[pos+1]; // next token
    }

    void advance(){
        if(pos <(int)tokens.size()) ++pos;
    }

    // check if current token matches a type if yes then consume it else throws an error

    void consume(TokenType type , const string &message){
        if(current().type == type) advance();
        else throw runtime_error("Parser Error: "+message+". Got "+ tokenTypeToString(current().type));
    }



    //Parser Function(Recursive Descent)
    unique_ptr<ExprAST> parseExpression();
    unique_ptr<ExprAST> parseTerm();
    unique_ptr<ExprAST> parseFactor();
    unique_ptr<ExprAST> parseAtom();
    unique_ptr<ExprAST> parseComparison();

    unique_ptr<StatementAST> parseVariableDeclaration();
    unique_ptr<StatementAST> parseAssignmentStatement();
    unique_ptr<StatementAST> parseCoutStatement();
    unique_ptr<StatementAST> parseIfStatement();
    unique_ptr<StatementAST> parseForStatement();
    unique_ptr<StatementAST> parseWhileStatement();
    unique_ptr<StatementAST> parseStatement();

    vector<unique_ptr<StatementAST>>parseBlock(); // Block Parsing

public:
    Parser(vector<Token>t):tokens(move(t)),pos(0){}
    ProgramAST parse();
};

//Expression Parsing
unique_ptr<ExprAST> Parser::parseAtom(){ //parse 'x','10',etc
    if(current().type == TokenType::NUMBER_LITERAL){
        string val = current().value;advance();
        return make_unique<NumberExprAST>(val);
    }
    if(current().type == TokenType::IDENTIFIER){
        string name = current().value; advance();
        return make_unique<VariableExprAST>(name);
    }
    if(current().type == TokenType::STRING_LITERAL){
        string val = current().value; advance();
        return make_unique<StringLiteralExprAST>(val);
    }
    throw runtime_error("Expected atom.");
}

//Parse (---) and atom
unique_ptr<ExprAST>Parser::parseFactor(){
    if(current().type == TokenType::OPEN_PAREN){
        advance();
        auto expr = parseComparison();
        consume(TokenType::CLOSE_PAREN,"')'");
        return expr;
    }
    return parseAtom();
}

// parse + , -
unique_ptr<ExprAST>Parser::parseExpression(){
    auto left = parseTerm();
    while(current().type == TokenType::PLUS || current().type == TokenType::MINUS){
        string op = (current().type == TokenType::PLUS)?"+":"-";
        advance();
        auto right = parseTerm();
        left = make_unique<BinaryExprAST>(op,move(left),move(right));
    }
    return left;
}

// parse * , /
unique_ptr<ExprAST>Parser::parseTerm(){
    auto left = parseFactor();
    while(current().type == TokenType::MULTIPLY || current().type == TokenType::DIVIDE){
        string op = (current().type == TokenType::MULTIPLY)?"*":"/";
        advance();
        auto right = parseFactor();
        left = make_unique<BinaryExprAST>(op,move(left),move(right));
    }
    return left;
}

// parse comparison operators
unique_ptr<ExprAST>Parser::parseComparison(){
    auto left = parseExpression();
    while(current().type == TokenType::GREATER_THAN ||
          current().type == TokenType::LESS_THAN ||
          current().type == TokenType::GREATER_THAN_EQUAL ||
          current().type == TokenType::LESS_THAN_EQUAL ||
          current().type == TokenType::DOUBLE_EQUALS ||
          current().type == TokenType::NOT_EQUAL){
        string op = current().value;
        advance();
        auto right = parseExpression();
        left = make_unique<BinaryExprAST>(op, move(left), move(right));
    }
    return left;
}

//Parsing Statement
unique_ptr<StatementAST> Parser::parseVariableDeclaration(){ //int x = 10;
    string type = current().value; advance();
    string name = current().value;
    consume(TokenType::IDENTIFIER,"variable name");
    unique_ptr<ExprAST> val = nullptr;
    if(current().type == TokenType::EQUALS){
        advance();
        val = parseExpression();
    }
    consume(TokenType::SEMICOLON,"';'");
    return make_unique<VarDeclAST>(type , name , move(val));
}
unique_ptr<StatementAST>Parser::parseAssignmentStatement(){ // x = 5;
    string name = current().value;
    advance();
    consume(TokenType::EQUALS,"'='");
    auto val = parseExpression();
    consume(TokenType::SEMICOLON,"';'");
    return make_unique<AssignmentAST>(name,move(val));
}
unique_ptr<StatementAST>Parser::parseCoutStatement(){
    advance(); // consume 'cout';
    auto coutNode = make_unique<CoutStatementAST>();
    while(current().type == TokenType::STREAM_OPERATOR){
        advance();
        if(current().type == TokenType::KEYWORD_ENDL){
            coutNode->hasEndl = true;
            advance();
        }
        else{
            auto expr = parseExpression();
            coutNode->parts.push_back(move(expr));
        }
    }
    consume(TokenType::SEMICOLON,"';'");
    return coutNode;
}

// Parse block{ ... }
vector<unique_ptr<StatementAST>>Parser::parseBlock(){
    vector<unique_ptr<StatementAST>>body;
    consume(TokenType::OPEN_BRACE , "'{'");
    while(current().type != TokenType::CLOSE_BRACE && current().type != TokenType::END_OF_FILE) body.push_back(parseStatement());
    consume(TokenType::CLOSE_BRACE,"'}'");
    return body;
}

// If / Else If / Else
unique_ptr<StatementAST>Parser::parseIfStatement(){
    auto ifNode = make_unique<IfStatementAST>();
    advance(); // consume 'if'
    consume(TokenType::OPEN_PAREN,"'('");
    ifNode->condition = parseComparison();
    consume(TokenType::CLOSE_PAREN,"')'");
    ifNode->thenBody = parseBlock();

    //handle else / else if
    while(current().type == TokenType::KEYWORD_ELSE){
        advance(); // consume 'else'
        if(current().type == TokenType::KEYWORD_IF){
            advance(); // consume else if
            consume(TokenType::OPEN_PAREN,"'('");
            auto cond = parseComparison();
            consume(TokenType::CLOSE_PAREN,"')'");
            auto body = parseBlock();
            ifNode ->elseIfBlocks.push_back({move(cond),move(body)});
        }
        else {ifNode->elseBody = parseBlock(); break;}
    }
    return ifNode;
}

// For Loop
unique_ptr<StatementAST>Parser::parseForStatement(){
    auto forNode = make_unique<ForStatementAST>();
    advance(); // consume 'for'
    consume(TokenType::OPEN_PAREN,"'('");
    string init , cond , inc;
    while(current().type != TokenType::SEMICOLON){init+= current().value + " ";advance();}
    consume(TokenType::SEMICOLON,"';'");
    while(current().type != TokenType::SEMICOLON){cond+=current().value + " ";advance();}
    consume(TokenType::SEMICOLON,"';'");
    while(current().type != TokenType::CLOSE_PAREN){inc+= current().value + " ";advance();}
    consume(TokenType::CLOSE_PAREN,"')'");

    forNode->init = init; forNode ->condition = cond; forNode ->increment = inc;
    forNode -> body = parseBlock();
    return forNode;
}
// While Loop
unique_ptr<StatementAST>Parser::parseWhileStatement(){
    auto node = make_unique<WhileStatementAST>();
    advance(); // consume 'while'
    consume(TokenType::OPEN_PAREN,"'('");
    node->condition = parseComparison();
    consume(TokenType::CLOSE_PAREN,"')'");
    node->body = parseBlock();
    return node;
}

unique_ptr<StatementAST> Parser::parseStatement(){
    switch(current().type){
        case TokenType::KEYWORD_INT:
        case TokenType::KEYWORD_FLOAT: return parseVariableDeclaration();
        case TokenType::IDENTIFIER: return parseAssignmentStatement();
        case TokenType::KEYWORD_COUT: return parseCoutStatement();
        case TokenType::KEYWORD_IF:return parseIfStatement();
        case TokenType::KEYWORD_FOR:return parseForStatement();
        case TokenType::KEYWORD_WHILE:return parseWhileStatement();
        default: throw runtime_error("Unexpected token: "+tokenTypeToString(current().type));
    }
}

ProgramAST Parser::parse(){
    ProgramAST prog;
    while(current().type != TokenType::END_OF_FILE) prog.push_back(parseStatement());
    return prog;
}
int main(){
    string input = R"(
        int x = 10;
        if (x > 5) {
            cout << "x greater than 5" << endl;
        } else if (x == 5) {
            cout << "x equals 5" << endl;
        } else {
            cout << "x less than 5" << endl;
        }

        for (int i = 0; i < 3; i++) {
            cout << i << endl;
        }

        while (x > 0) {
            x = x - 1;
        }
    )";

    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();

    cout << "=========== PARSED AST ===========" << endl;
    for (auto &stmt : program)
        stmt->print();

    return 0;
}
