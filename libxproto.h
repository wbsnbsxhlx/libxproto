#ifndef libxproto_h__
#define libxproto_h__

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include <assert.h>

enum KeyWord {
	KW_NULL,
	KW_REQUIRED,
	KW_REPEATED,
	KW_FLOAT,
	KW_DOUBLE,
	KW_INT32,
	KW_INT64,
	KW_UINT32,
	KW_UINT64,
	KW_BOOL,
	KW_STRING,
	KW_ENUM,
	KW_MESSAGE,
	KW_STRUCT,
	KW_BINARY,
	KW_ARRAY
};

enum TokenType {
	TN_NULL,
	TN_END,
	TN_COLON,
	TN_OPEN_PAREN,
	TN_CLOSE_PAREN,
	TN_OPEN_BRACE,
	TN_CLOSE_BRACE,
	TN_OPEN_CURLY_BRACE,
	TN_CLOSE_CURLY_BRACE,
	TN_COMMA,
	TN_POINT,
	TN_SEMICOLON,
	TN_EQUAL,
	TN_STAR,
	TN_REQUIRED,
	TN_REPEATED,
	TN_FLOAT,
	TN_DOUBLE,
	TN_INT32,
	TN_INT64,
	TN_UINT32,
	TN_UINT64,
	TN_BOOL,
	TN_STRING,
	TN_ENUM,
	TN_MESSAGE,
	TN_STRUCT,
	TN_BINARY,
	TN_ARRAY,
	TN_IDENTIFIER
};

struct ProtoToken {
	int line;
	TokenType type;
	std::string str;
	int64_t n;
	double f;

	ProtoToken()
		:line(-1),
		type(TN_NULL),
		n(0),
		f(0.0) {}
};

struct ProtoMemberType {
	TokenType type;
	std::string name;

};

struct ProtoStructMember {
	int tag;
	TokenType fieldRule;
	ProtoMemberType type;
	std::string name;

};

struct ProtoEnumItem {
	std::string name;
	int value;
};

struct ProtoEnum {
	std::string name;
	std::vector<ProtoEnumItem> itemVec;
};

struct ProtoStruct {
	TokenType type;
	std::string name;
	std::string group;
	std::vector<ProtoStructMember> memberVec;

	void addMember(ProtoStructMember& member) {
		if (member.tag == memberVec.size() + 1) {
			memberVec.push_back(member);
		} else {
			printf("struct %s member %s tag %d error", name, member.name, member.tag);
			assert(0);
		}
	}
};

struct ProtoResult {
	std::vector<ProtoStruct> msgList;
	std::vector<ProtoStruct> structList;
	std::vector<ProtoEnum> enumList;
};

void strAppendFmt(std::string& str, const char* formatTxt, ...);
#endif // libxproto_h__
