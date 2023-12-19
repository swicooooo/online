#pragma

#include <string>
#include <cstring>
#include <memory>

namespace{
namespace{
enum TokenKind{
    ADD='+',
    SUB='-',
    MUL='*',
    DIV='/',
    DIG,
    EOC,
};
struct Token
{
    TokenKind kind;
    std::string content;
};

class lexer
{
private:
    std::string_view _sourceCode;
    char _curChar;
    int _curPos;
    std::unique_ptr<Token> _token;
public:
    lexer(std::string_view source):_sourceCode(),_curPos(0),_curChar(0),_token(new Token){
        _sourceCode=source.substr(source.find_first_not_of(' '),source.find_last_not_of(' ')-source.find_first_not_of(' ')+1);
    }
    inline std::string GetNextToken(){return _token->content;}
    bool GetNextChar(){
        if(_curPos==_sourceCode.size())
            return false;
        _curChar=_sourceCode[_curPos];
        while(isspace(_curChar)) _curChar=_sourceCode[++_curPos];
        switch (_curChar)
        {
        case '+':
            _token->kind=TokenKind::ADD;
            _token->content=_curChar;
            _curPos++;
            break;
        case '-':
            _token->kind=TokenKind::SUB;
            _token->content=_curChar;
            _curPos++;
            break;
        case '*':
            _token->kind=TokenKind::MUL;
            _token->content=_curChar;
            _curPos++;
            break;
        case '/':
            _token->kind=TokenKind::DIV;
            _token->content=_curChar;
            _curPos++;
            break;
        default:
            if(isdigit(_curChar)){
                _token->content.clear();
                do{
                    _token->kind=TokenKind::DIG;
                    _token->content+=_curChar;
                    _curChar=_sourceCode[++_curPos];
                } while (isdigit(_curChar));
            }else{
                printf("current character is invalidated!");
                return false;
            }
        }
        return true;
    }
};
}    
}