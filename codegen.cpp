#include <memory>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include "parser.cpp"

class CodeGenrator{
private:
    string indent(int n);
    string exprToPy(const ExprAST &expr);
    string varDeclToPy(const VarDeclAST &v);
    string assignToPy(const AssignmentAST &a);
    string coutToPy(const CoutStatementAST &c);
    string ifToPy(const IfStatementAST &i , int indentLevel);
    string forToPy(const ForStatementAST &f , int indentLevel);
    string whileToPy(const WhileStatementAST &w, int indentLevel);
    string toPython(const StatementAST &stmt,int indentLevel);
public:
    string generate(const ProgramAST &program);
};

string CodeGenrator::indent(int n){ // string consisting of n spaces
    return string(n,' ');
}

string CodeGenrator::exprToPy(const ExprAST &expr){
    if(auto n = dynamic_cast<const NumberExprAST*>(&expr)) return n->toPython();
    else if (auto v = dynamic_cast<const VariableExprAST*>(&expr)) return v->toPython();
    else if (auto s = dynamic_cast<const StringLiteralExprAST*>(&expr)) return s->toPython();
    else if (auto b = dynamic_cast<const BinaryExprAST*>(&expr)) return b->toPython();
    else return "/*unkown_expr*/";
}

// varDecl to Py  e.g. int x = 10; => "x = 10"  ,  "int y"; => "y = None"
string CodeGenrator::varDeclToPy(const VarDeclAST &v){
    if(v.initialValue) return v.varName + " = " + v.initialValue->toPython();
    else return v.varName + " = None"; 
}

//assignToPy e.g. x = 5 =>same
string CodeGenrator::assignToPy(const AssignmentAST&a){
    return a.varName +" = "+a.value->toPython();
}

// coutToPy e.g. cout << "Hi" << x << endl; => print("Hi",x)
string CodeGenrator::coutToPy(const CoutStatementAST &c){
    ostringstream ss;
    ss << "print(";
    for(size_t i = 0 ; i < c.parts.size();++i){
        ss << c.parts[i]->toPython();
        if(i < c.parts.size() -1) ss << ", ";
    }
    ss << ")";
    return ss.str();
}

//ifToPy
// 1. emit: indent + "if" + condition + ":\n"
// 2. For each stmt in then Body: recursively convert with idndent 4
// 3. For each else-if block : emit "elif cond: " and its body
// 4. If elseBody exists: emit "else: " and its body
string CodeGenrator::ifToPy(const IfStatementAST &i , int indentLevel){
    ostringstream ss;
    string pad = indent(indentLevel);
    // Emit the 'if' line
    ss << pad << "if " << i.condition->toPython() << ":\n";

    //Then-block: each statement inside thenBody is indented furthur
    for(auto &s: i.thenBody) ss << toPython(*s,indentLevel+4) << "\n";
    
    //Else-if block-emit as 'elif'
    for(auto &eif:i.elseIfBlocks){
        ss << pad << "elif " << eif.first->toPython() << ":\n";
        for(auto &s:eif.second) ss << toPython(*s,indentLevel+4) << "\n";
    }

    // Else block
    if(!i.elseBody.empty()){
        ss << pad << "else:\n";
        for(auto &s:i.elseBody) ss << toPython(*s,indentLevel + 4) << "\n";
    }
    return ss.str();
}

//forToPy
string CodeGenrator::forToPy(const ForStatementAST &f , int indentLevel){
    string pad = indent(indentLevel);

    string init = f.init;
    string cond = f.condition;
    string inc = f.increment;

    //inline helper to trim whitespace
    auto trim =[](string t){
        while(!t.empty() && isspace((unsigned char)t.front()) ) t.erase(t.begin());
        while(!t.empty() && isspace((unsigned char)t.back())) t.pop_back();
        return t;
    };
    init = trim(init);
    cond = trim(cond);
    inc = trim(inc);

    string var,start,endExpr;
    size_t eq = init.find('=');

    //Extract variable and starting value (if pattern contains '=')
    if(eq != string::npos){
        string left = trim(init.substr(0,eq));   // left part e.g. "int i"
        string right = trim(init.substr(eq+1));  // right part e.g. "0"

        // remove "int" or "float" if present
        if(left.find("int ") == 0) left = left.substr(4);
        else if(left.find("float ") == 0) left = left.substr(6);

        var = trim(left);
        start = right;
    }

    //Extract end value from condition like i < 3
    size_t opPos = cond.find('<');
    if(opPos == string::npos) opPos = cond.find('>');
    if(opPos != string::npos) endExpr = trim(cond.substr(opPos+1));
    else endExpr = "/*cond*/";

    int step = 1;
    if(inc.find("--") != string::npos) step = -1;

    ostringstream ss;
    ss << pad << "for " << var << " in range(" << start << ", " << endExpr;
    if(step != 1) ss << ", " << step;
    ss << "):\n";
    for(auto &s:f.body) ss << toPython(*s,indentLevel+4) << "\n";
    return ss.str();
}


// whileToPy  e.g. while (x>0){x=x-1;} => while (x>0):\n    x=(x-1)
string CodeGenrator::whileToPy(const WhileStatementAST &w,int indentLevel){
    ostringstream ss;
    string pad = indent(indentLevel);
    ss << pad << "while " << w.condition->toPython() << ":\n";
    for(auto &s:w.body) ss << toPython(*s,indentLevel+4) << "\n";
    return ss.str();
}

// detect node type and call corresponding helper
string CodeGenrator::toPython(const StatementAST &stmt,int indentLevel){
    ostringstream out;
    if(auto v = dynamic_cast<const VarDeclAST*>(&stmt)) out << indent(indentLevel) << varDeclToPy(*v);
    else if(auto a = dynamic_cast<const AssignmentAST*>(&stmt)) out << indent(indentLevel) << assignToPy(*a);
    else if(auto c = dynamic_cast<const CoutStatementAST*>(&stmt)) out << indent(indentLevel) << coutToPy(*c);
    else if(auto i = dynamic_cast<const IfStatementAST*>(&stmt)) out << ifToPy(*i,indentLevel);
    else if(auto f = dynamic_cast<const ForStatementAST*>(&stmt)) out << forToPy(*f,indentLevel);
    else if(auto w = dynamic_cast<const WhileStatementAST*>(&stmt)) out << whileToPy(*w,indentLevel);
    else out << indent(indentLevel) << "# unknown stmt\n";
    return out.str();
}

// generate() - top-level driver
string CodeGenrator::generate(const ProgramAST &program){
    ostringstream out;
    for(auto &stmt: program) out << toPython(*stmt,0) << "\n";
    return out.str();
}

// main driver - read from file input.cpp and export output.py
int main(){
    ifstream inputFile("input.txt");
    if(!inputFile.is_open()){
        cerr << "Error: could not open input.txt" << endl;
        return 1;
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    string input = buffer.str();
    inputFile.close();

    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();

    CodeGenrator gen;
    string pythonCode = gen.generate(program);

    ofstream outFile("output.py");
    if(!outFile.is_open()){
        cerr << "Error: could not create output.py" << endl;
        return 1;
    }
    outFile << pythonCode;
    outFile.close();

    cout << "Conversion successful! Generated output.py file.\n";
    return 0;
}
