#ifndef LEXER_CPP
#define LEXER_CPP
#include <bits/stdc++.h>
using namespace std;

// Lexer Analysis

// 1) Token Types:
enum class TokenType{
    // Keywords
    KEYWORD_INT,
    KEYWORD_FLOAT,
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_FOR,
    KEYWORD_WHILE,
    KEYWORD_COUT,
    KEYWORD_ENDL,
    KEYWORD_MAIN,

    // LITERALS
    IDENTIFIER,
    NUMBER_LITERAL,
    STRING_LITERAL,

    // OPERATORS
    EQUALS,         // =
    DOUBLE_EQUALS,  // ==
    PLUS,           // +
    PLUS_PLUS,      // ++
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    MODULO,         // %
    LESS_THAN,      // <
    LESS_THAN_EQUAL,// <=
    GREATER_THAN,   // >
    GREATER_THAN_EQUAL,// >=
    NOT_EQUAL,          // !=
    STREAM_OPERATOR,    // <<

    //PUNCTUATION
    OPEN_PAREN,    // (
    CLOSE_PAREN,   // )
    OPEN_BRACE,    // {
    CLOSE_BRACE,   // }
    SEMICOLON,     // ;

    //SPECIAL
    END_OF_FILE,
    UNKNOWN     // Error
};

// 2) Function:

string tokenTypeToString(TokenType type){
    switch(type){
        case TokenType::KEYWORD_INT: return "KEYWORD_INT";
        case TokenType::KEYWORD_FLOAT: return "KEYWORD_FLOAT";
        case TokenType::KEYWORD_IF: return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case TokenType::KEYWORD_FOR: return "KEYWORD_FOR";
        case TokenType::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case TokenType::KEYWORD_COUT: return "KEYWORD_COUT";
        case TokenType::KEYWORD_MAIN: return "KEYWORD_MAIN";
        case TokenType::KEYWORD_ENDL: return "KEYWORD_ENDL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER_LITERAL: return "NUMBER_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::EQUALS: return "EQUALS";
        case TokenType::DOUBLE_EQUALS: return "DOUBLE_EQUALS";
        case TokenType::PLUS: return "PLUS";
        case TokenType::PLUS_PLUS: return "PLUS_PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        case TokenType::LESS_THAN: return "LESS_THAN";
        case TokenType::LESS_THAN_EQUAL: return "LESS_THAN_EQUAL";
        case TokenType::GREATER_THAN: return "GREATER_THAN";
        case TokenType::GREATER_THAN_EQUAL: return "GREATER_THAN_EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::STREAM_OPERATOR: return "STREAM_OPERATOR";
        case TokenType::OPEN_PAREN: return "OPEN_PAREN";
        case TokenType::CLOSE_PAREN: return "CLOSE_PAREN";
        case TokenType::OPEN_BRACE: return "OPEN_BRACE";
        case TokenType::CLOSE_BRACE: return "CLOSE_BRACE";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "INVALID_TOKEN";
    }
}

//Token Structure:
struct Token{
    TokenType type;
    string value;
    
    Token(TokenType t,string v): type(t),value(v){}
};

//Lexer Class:
class Lexer{
private:
    string sourceCode;
    int currentPos;
    map<string,TokenType> keywords;

    char peek(){                                        // look next char
        if(currentPos + 1 >= sourceCode.length()){
            return '\0'; // End of File
        }
        return sourceCode[currentPos + 1];
    }

    void skipComment(){ 
        //single line comment
        while(currentPos < sourceCode.length() && sourceCode[currentPos] != '\n'){
            currentPos++;
        }
    }

    Token readIdentifier(){
        string value;
        while(currentPos < sourceCode.length() && (isalnum(sourceCode[currentPos]) || sourceCode[currentPos] == '_')){
            value += sourceCode[currentPos];
            currentPos++;
        }

        if(keywords.count(value)) return Token(keywords[value],value);
        return Token(TokenType::IDENTIFIER,value);
    }

    Token readNumber(){
        string value;
        bool hasDecimal = false;
        while(currentPos < sourceCode.length() && (isdigit(sourceCode[currentPos]) || sourceCode[currentPos] =='.')){
            if(sourceCode[currentPos] == '.'){
                if(hasDecimal) break; // only one decimal point allowed
                hasDecimal = true;
            }
            value += sourceCode[currentPos];
            currentPos++;
        }
        return Token(TokenType::NUMBER_LITERAL,value);
    }

    Token readStringLiteral(){
            string value;
            currentPos++; // skip opening quote
            while(currentPos < sourceCode.length() && sourceCode[currentPos]!='"'){
                value += sourceCode[currentPos];
                currentPos++;
            }
            currentPos++;
            return Token(TokenType::STRING_LITERAL,value);
    }
public:
    Lexer(const string& source): sourceCode(source),currentPos(0){

        keywords["int"] = TokenType::KEYWORD_INT;
        keywords["float"] = TokenType::KEYWORD_FLOAT;
        keywords["if"] = TokenType::KEYWORD_IF;
        keywords["else"] = TokenType::KEYWORD_ELSE;
        keywords["for"] = TokenType::KEYWORD_FOR;
        keywords["while"] = TokenType::KEYWORD_WHILE;
        keywords["cout"] = TokenType::KEYWORD_COUT;
        keywords["main"] = TokenType::KEYWORD_MAIN;
        keywords["endl"] = TokenType::KEYWORD_ENDL;
    }

    // function to have list of tockens:

    vector<Token>tokenize(){
        vector<Token>tokens;

        while(currentPos < sourceCode.length()){
            char currentChar = sourceCode[currentPos];

            // 1. skip Whitespace
            if(isspace(currentChar)){
                currentPos++;
                continue;
            }
            
            // 2. skip Comments
            if(currentChar == '/' && peek() == '/'){
                skipComment();
                continue;
            }

            // 3. Identifiers and Keywords
            if(isalpha(currentChar) || currentChar == '_'){
                tokens.push_back(readIdentifier());
                continue;
            }
            // 4. Numbers
            if(isdigit(currentChar)){
                tokens.push_back(readNumber());
                continue;
            }
            // 5. String Literals
            if(currentChar == '"'){
                tokens.push_back(readStringLiteral());
                continue;
            }
            // 6. Operators and Punctuation
             if (currentChar == '=') {
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::DOUBLE_EQUALS, "=="));
                    currentPos += 2;
                } else {
                    tokens.push_back(Token(TokenType::EQUALS, "="));
                    currentPos++;
                }
            } else if (currentChar == '+') {
                if (peek() == '+') {
                    tokens.push_back(Token(TokenType::PLUS_PLUS, "++"));
                    currentPos += 2;
                } else {
                    tokens.push_back(Token(TokenType::PLUS, "+"));
                    currentPos++;
                }
            } else if (currentChar == '<') {
                if (peek() == '<') {
                    tokens.push_back(Token(TokenType::STREAM_OPERATOR, "<<"));
                    currentPos += 2;
                } else if (peek() == '=') {
                    tokens.push_back(Token(TokenType::LESS_THAN_EQUAL, "<="));
                    currentPos += 2;
                } else {
                    tokens.push_back(Token(TokenType::LESS_THAN, "<"));
                    currentPos++;
                }
            } else if (currentChar == '>') {
                if (peek() == '=') {
                    tokens.push_back(Token(TokenType::GREATER_THAN_EQUAL, ">="));
                    currentPos += 2;
                } else {
                    tokens.push_back(Token(TokenType::GREATER_THAN, ">"));
                    currentPos++;
                }
            } else if (currentChar == '!') {
                 if (peek() == '=') {
                    tokens.push_back(Token(TokenType::NOT_EQUAL, "!="));
                    currentPos += 2;
                } else {
                    tokens.push_back(Token(TokenType::UNKNOWN, "!"));
                    currentPos++;
                }
            } else if (currentChar == '-') {
                tokens.push_back(Token(TokenType::MINUS, "-"));
                currentPos++;
            } else if (currentChar == '*') {
                tokens.push_back(Token(TokenType::MULTIPLY, "*"));
                currentPos++;
            } else if (currentChar == '/') {
                tokens.push_back(Token(TokenType::DIVIDE, "/"));
                currentPos++;
            } else if (currentChar == '%') { 
                tokens.push_back(Token(TokenType::MODULO, "%"));
                currentPos++;
            } else if (currentChar == '(') {
                tokens.push_back(Token(TokenType::OPEN_PAREN, "("));
                currentPos++;
            } else if (currentChar == ')') {
                tokens.push_back(Token(TokenType::CLOSE_PAREN, ")"));
                currentPos++;
            } else if (currentChar == '{') {
                tokens.push_back(Token(TokenType::OPEN_BRACE, "{"));
                currentPos++;
            } else if (currentChar == '}') {
                tokens.push_back(Token(TokenType::CLOSE_BRACE, "}"));
                currentPos++;
            } else if (currentChar == ';') {
                tokens.push_back(Token(TokenType::SEMICOLON, ";"));
                currentPos++;
            } else {
                // If we don't recognize the character, it's an UNKNOWN token.
                tokens.push_back(Token(TokenType::UNKNOWN, string(1, currentChar)));
                currentPos++;
            }
        }

        // Add one final End-Of-File token so the Parser knows we're done.
        tokens.push_back(Token(TokenType::END_OF_FILE, ""));
        return tokens;
    }


};
int main() {
    // Sample source code for tokenization
    string sourceCode = R"(
    int main() {
        int a = 10;
        float b = 3.14;
        if (a > b) {
            cout << "a is greater than b" << endl;
        } else {
            cout << "a is not greater than b" << endl;
        }
        return 0;
    }
    )";
    
    // Create a lexer instance with the source code
    Lexer lexer(sourceCode);

    // Tokenize the source code
    vector<Token> tokens = lexer.tokenize();

    // Print out each token and its type
    for (const auto& token : tokens) {
        cout << "Token: " << token.value << ", Type: " << tokenTypeToString(token.type) << endl;
    }

    return 0;
}

#endif 