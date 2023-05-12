#include "proto_parser.h"
#include "proto_lex.h"
#include <fstream>
#include <assert.h>
#include <set>

std::string getFileContent(const char *filename) {
	std::ifstream ifile;
	ifile.open(filename, std::ios::in | std::ios::binary);

	ifile.seekg(0, std::ios::end);
	std::streampos ps = ifile.tellg();
	ifile.seekg(0, 0);
	int len = ps;

	char* buf = new char[len + 1];
	ifile.read(buf, len);
	buf[len] = '\0';

	std::string ret(buf);

	delete[] buf;
	ifile.close();

	return ret;
}

bool proto_parse_member(ProtoLex &lex, ProtoStruct& msg) {
	ProtoStructMember member;
	ProtoToken token = lex.readToken();

	if (token.type == TN_REQUIRED || token.type == TN_REPEATED) {
		member.fieldRule = token.type;
	} else {
		lex.unreadToken(token);
		return false;
	}

	token = lex.readToken();
	if (ProtoLex::isBaseType(token.type)) {
		member.type.type = token.type;
		member.type.name = token.str;
	} else if (token.type == TN_ENUM) {
		member.type.type = token.type;
		ProtoToken t = lex.readToken();
		member.type.name = t.str;
	}else if (token.type == TN_IDENTIFIER) {
		member.type.type = TN_STRUCT;
		member.type.name = token.str;
	} else {
		lex.parse_assert("lex error");
	}

	token = lex.readToken();
	if (token.type != TN_IDENTIFIER) {
		lex.parse_assert("should identifier");
	}
	member.name = token.str;

	token = lex.readToken();
	if (member.fieldRule == TN_REPEATED && token.type == TN_OPEN_BRACE){
		ProtoToken t = lex.readToken();
		member.type.arrLength = t.str;
		lex.readToken(TN_CLOSE_BRACE);
	} else {
		lex.unreadToken(token);
	}

	lex.readToken(TN_EQUAL);

	token = lex.readToken();
	if (token.type != TN_INT64) {
		lex.parse_assert("should identifier");
	}
	member.tag = token.n;

	lex.readToken(TN_SEMICOLON);

	msg.addMember(member);
	return true;
}

ProtoStruct proto_parse_struct(ProtoLex &lex) {
	ProtoToken token = lex.readToken();
	if (token.type != TN_IDENTIFIER) {
		lex.parse_assert("should identifier");
	}

	ProtoStruct ret;
	ret.type = TN_STRUCT;
	ret.name = token.str;

	lex.readToken(TN_OPEN_CURLY_BRACE);
	while (proto_parse_member(lex, ret));
	lex.readToken(TN_CLOSE_CURLY_BRACE);

	return ret;
}

ProtoStruct proto_parse_message(ProtoLex &lex) {
	ProtoToken token = lex.readToken();
	if (token.type != TN_IDENTIFIER) {
		lex.parse_assert("should identifier");
	}

	ProtoStruct ret;
	ret.type = TN_MESSAGE;
	ret.name = token.str;

	lex.readToken(TN_OPEN_BRACE);
	token = lex.readToken();
	if (token.type != TN_IDENTIFIER) {
		lex.parse_assert("should identifier");
	}
	ret.group = token.str;
	lex.readToken(TN_CLOSE_BRACE);

	lex.readToken(TN_OPEN_CURLY_BRACE);
	while (proto_parse_member(lex, ret));
	lex.readToken(TN_CLOSE_CURLY_BRACE);

	return ret;
}

ProtoEnum proto_parse_enum(ProtoLex &lex) {
	ProtoToken token = lex.readToken();
	if (token.type != TN_IDENTIFIER) {
		lex.parse_assert("should identifier");
	}

	ProtoEnum ret;
	ret.name = token.str;

	lex.readToken(TN_OPEN_CURLY_BRACE);
	while (true) {
		token = lex.readToken();
		if (token.type != TN_IDENTIFIER)
		{
			lex.unreadToken(token);
			break;
		}

		lex.readToken(TN_EQUAL);
		ProtoToken t = lex.readToken();
		if (t.type != TN_INT64) {
			lex.parse_assert("should number");
		}
		lex.readToken(TN_SEMICOLON);
		ret.itemVec.push_back({ token.str, (int)t.n });
	}
	lex.readToken(TN_CLOSE_CURLY_BRACE);

	return ret;
}

ProtoResult proto_parse(const char *filename) {
	std::string content = getFileContent(filename);

	ProtoLex lex;
	lex.setData(content);

	ProtoResult ret;

	while (true) {
		ProtoToken token = lex.readToken();
		if (token.type == TN_END) {
			break;
		} else if (token.type == TN_ENUM) {
			ProtoEnum e = proto_parse_enum(lex);
			ret.enumList.push_back(e);
		} else if (token.type == TN_STRUCT) {
			ProtoStruct stru = proto_parse_struct(lex);
			ret.structList.push_back(stru);
		} else if (token.type == TN_MESSAGE) {
			ProtoStruct stru = proto_parse_message(lex);
			ret.msgList.push_back(stru);
		} else {
			assert(0);
		}
	}

	return ret;
}