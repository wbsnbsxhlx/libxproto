#ifndef proto_lex_h__
#define proto_lex_h__
#include "libxproto.h"
#include <string>
#include <map>
#include <vector>

class ProtoLex {
public:
	ProtoLex();
	~ProtoLex();

	void setData(std::string content);
	
	void unreadToken(ProtoToken& token);
	ProtoToken readToken();
	void readToken(TokenType type);

	char getChar();
	void ungetChar();

	void takeId(char c, ProtoToken* token);
	void takeNum(char c, ProtoToken* token);
	void takeString(char c, ProtoToken* token);
	void takeNotes(char c, ProtoToken* token);

	void parse_assert(const char *desc);
private:
	std::string _content;
	size_t _pos;
	size_t _line;

	std::vector<ProtoToken> _unreadTokenVec;
};

#endif // proto_lex_h__