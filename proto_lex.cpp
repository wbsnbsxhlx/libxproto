#include "proto_lex.h"
#include <assert.h>
#include <set>

std::map<std::string, TokenType> s_TokenTypeNameMap = {
		{ "required", TN_REQUIRED },
		{ "repeated", TN_REPEATED },
		{ "float", TN_FLOAT },
		{ "double", TN_DOUBLE },
		{ "int32", TN_INT32 },
		{ "int64", TN_INT64 },
		{ "uint32", TN_UINT32 },
		{ "uint64", TN_UINT64 },
		{ "bool", TN_BOOL },
		{ "string", TN_STRING },
		{ "message", TN_MESSAGE },
		{ "struct", TN_STRUCT },
		{ "enum", TN_ENUM },
		{ "binary", TN_BINARY },
		{ "array", TN_ARRAY },
};

std::set<TokenType> s_TypeSet = {
	TN_FLOAT,
	TN_DOUBLE,
	TN_INT32,
	TN_INT64,
	TN_UINT32,
	TN_UINT64,
	TN_BOOL,
	TN_STRING,
	TN_BINARY
};

bool ProtoLex::isBaseType(TokenType type) {
	return s_TypeSet.count(type) > 0;
}

ProtoLex::ProtoLex()
	:_pos(0),
	_line(1) {}

ProtoLex::~ProtoLex() {

}

void ProtoLex::setData(std::string content) {
	_pos = 0;
	_line = 1;
	_content = content;
	_unreadTokenVec.clear();
}

void ProtoLex::parse_assert(const char *desc) {
	printf("[line: %d] error: %s\n", _line, desc);
	assert(0);
}

char ProtoLex::getChar() {
	if (_pos == _content.size()) {
		return 0;
	}

	assert(_pos >= 0 && _pos < _content.size());

	return _content[_pos++];
}

void ProtoLex::ungetChar() {
	assert(_pos > 0);
	_pos--;
}

void ProtoLex::takeString(char c, ProtoToken* token) {
	while (true) {
		c = getChar();
		if (c == 0 || c == '\r' || c == '\n') {
			parse_assert("take string error");
		} else if (c == '"') {
			token->type = TN_STRING;
			return;
		} else {
			token->str.push_back(c);
		}
	}
}

void ProtoLex::takeNotes(char c, ProtoToken* token) {
	c = getChar();
	if (c != '/') {
		parse_assert("error notes");
	}
	while (true) {
		c = getChar();
		if (c == 0) {
			break;
		} else if (c == '\n') {
			_line++;
			token->line++;
		}
	}
}

void ProtoLex::takeId(char c, ProtoToken* token) {
	while (c != 0) {
		token->str.push_back(c);
		c = getChar();
		if (!isalpha(c) && c != '_' && !isdigit(c)) {
			ungetChar();
			break;
		}
	}

	if (s_TokenTypeNameMap.count(token->str) > 0) {
		token->type = s_TokenTypeNameMap[token->str];
	} else {
		token->type = TN_IDENTIFIER;
	}
}

void ProtoLex::takeNum(char c, ProtoToken* token) {
	bool dot = false;
	std::string str;
	while (true) {
		str.push_back(c);
		c = getChar();
		if (c == 0) {
			break;
		} else if (!isdigit(c)) {
			if (!dot && c == 'c') {
				dot = true;
				continue;
			}
			ungetChar();
			break;
		}
	}

	if (dot) {
		token->type = TN_DOUBLE;
		token->f = atof(str.c_str());
	} else {
		token->type = TN_INT64;
		token->n = atoi(str.c_str());
	}
}

ProtoToken ProtoLex::readToken() {
	if (_unreadTokenVec.size() > 0) {
		ProtoToken token = _unreadTokenVec.back();
		_unreadTokenVec.pop_back();
		return token;
	}

	ProtoToken token;

	while (true) {
		char c = getChar();
		token.line = _line;

		if (c == 0) {
			token.type = TN_END;
			break;
		} else if (c == '\n') {
			_line++;
			continue;
		} else if (c == '\r') {
			char n = getChar();
			_line++;
			if (n != '\n') {
				ungetChar();
			}
			continue;
		} else if (c == '\t' || c == ' ') {
			continue;
		} else if (isalpha(c)) {
			takeId(c, &token);
		} else if (isdigit(c)) {
			takeNum(c, &token);
		} else if (c == '"') {
			takeString(c, &token);
		} else if (c == '/') {
			takeNotes(c, &token);
			continue;
		} else if (c == ':') {
			token.type = TN_COLON;
		} else if (c == '*') {
			token.type = TN_STAR;
		} else if (c == '(') {
			token.type = TN_OPEN_PAREN;
		} else if (c == ')') {
			token.type = TN_CLOSE_PAREN;
		} else if (c == '[') {
			token.type = TN_OPEN_BRACE;
		} else if (c == ']') {
			token.type = TN_CLOSE_BRACE;
		} else if (c == '{') {
			token.type = TN_OPEN_CURLY_BRACE;
		} else if (c == '}') {
			token.type = TN_CLOSE_CURLY_BRACE;
		} else if (c == ',') {
			token.type = TN_COMMA;
		} else if (c == '.') {
			token.type = TN_POINT;
		} else if (c == ';') {
			token.type = TN_SEMICOLON;
		} else if (c == '=') {
			token.type = TN_EQUAL;
		}
		break;
	}
	return token;
}

void ProtoLex::readToken(TokenType type) {
	if (readToken().type != type) {
		parse_assert("lex error");
	}
}

void ProtoLex::unreadToken(ProtoToken& token) {
	_unreadTokenVec.push_back(token);
}
