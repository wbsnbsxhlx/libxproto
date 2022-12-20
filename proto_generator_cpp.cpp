#include "proto_generator_cpp.h"
#include "proto_lex.h"

void ProtoGeneratorCpp::genHeader(std::string& outStr) {
	outStr += "#pragma once\n\
#include <vector>\n\
#include \"libxpack.h\"\n\n\
namespace protocol {\n\
	class ProtocolBase {\n\
	public:\n\
		uint32_t protocol;\n\
		ProtocolBase(uint32_t proto) {\n\
			protocol = proto;\n\
		}\n\
		virtual ~ProtocolBase() {}\n\
		virtual void Pack(packer_t packer) {\n\
			pack_pack_uint(packer, protocol);\n\
		}\n\
	};\n\
}\n\n";
}

std::string ProtoGeneratorCpp::getTypeString(ProtoMemberType type) {
	switch (type.type) {
		case TN_STRING:
			return "std::string";
		case TN_DOUBLE:
			return "double";
		case TN_FLOAT:
			return "float";
		case TN_INT32:
			return "int32_t";
		case TN_INT64:
			return "int64_t";
		case TN_UINT32:
			return "uint32_t";
		case TN_UINT64:
			return "uint64_t";
		case TN_BOOL:
			return "bool";
		case TN_BINARY:
			return "std::string";
		case TN_STRUCT:
			return type.name;
		case TN_ENUM:
			return type.name;
	}
}

std::string ProtoGeneratorCpp::generate(std::vector<ProtoResult>& resultVec) {
	std::string ret;

	ProtoResult result;
	for (int i = 0; i < resultVec.size(); i++){
		result.enumList.insert(result.enumList.begin(), resultVec[i].enumList.begin(), resultVec[i].enumList.end());
		result.msgList.insert(result.msgList.begin(), resultVec[i].msgList.begin(), resultVec[i].msgList.end());
		result.structList.insert(result.structList.begin(), resultVec[i].structList.begin(), resultVec[i].structList.end());
	}
	genHeader(ret);

	genMsgIndex(ret, result.msgList);
	genEnumList(ret, result.enumList);
	genStructList(ret, result.structList);
	genMsgList(ret, result.msgList);

	return ret;
}

void ProtoGeneratorCpp::genPackBaseType(std::string& outStr, ProtoStructMember& member) {
	char buf[40960];
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			sprintf_s(buf, 4096, "			%s == 0 ? field++ : pack_pack_uint(packer, field), pack_pack_int(packer, %s), field = 1;\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			sprintf_s(buf, 4096, "			%s == 0 ? field++ : pack_pack_uint(packer, field), pack_pack_uint(packer, %s), field = 1;\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			sprintf_s(buf, 4096, "			%s == 0 ? field++ : pack_pack_uint(packer, field), pack_pack_float(packer, %s), field = 1;\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_STRING:
			sprintf_s(buf, 4096, "			%s.size() == 0 ? field++ : pack_pack_uint(packer, field), pack_pack_raw(packer, (void*)%s.c_str(), %s.size()), field = 1;\n", member.name.c_str(), member.name.c_str(), member.name.c_str());
			break;
		case TN_ENUM:
			sprintf_s(buf, 4096, "			(uint32_t)%s == 0 ? field++ : pack_pack_uint(packer, field), pack_pack_uint(packer, (uint32_t)%s), field = 1;\n", member.name.c_str(), member.name.c_str());
			break;
	}
	outStr += buf;
}

void ProtoGeneratorCpp::genPack(std::string& outStr, ProtoStruct& s, bool isMsg) {
	char buf[4096];
	outStr += "		void Pack(packer_t packer) {\n";
	if (isMsg) {
		outStr += "			ProtocolBase::Pack(packer);\n";
	}
	outStr += "			size_t field = 1;\n";

	for (int i = 0; i < s.memberVec.size(); i++)
	{
		ProtoStructMember member = s.memberVec[i];
		if (member.fieldRule == TN_REPEATED)
		{
			//genPackRepeat(outStr, member);
		} else {
			if (member.type.type == TN_ENUM)
			{
				//genPackEnum(outStr, member, isMsg);
			} else if (member.type.type == TN_STRUCT)
			{
				//genPackStruct(outStr, member, isMsg);
			} else if (ProtoLex::isBaseType(member.type.type))
			{
				genPackBaseType(outStr, member);
			}
		}
	}

	outStr += "		}\n\n";
}

void ProtoGeneratorCpp::genUnpack(std::string& outStr, ProtoStruct& s, bool isMsg) {

}

void ProtoGeneratorCpp::genMember(std::string& outStr, ProtoStructMember& member) {
	char buf[4096];
	if (member.fieldRule == TN_REPEATED){
		sprintf_s(buf, 4096, "		std::vector<%s> %s;\n", getTypeString(member.type).c_str(), member.name.c_str());
	} else {
		sprintf_s(buf, 4096, "		%s %s;\n", getTypeString(member.type).c_str(), member.name.c_str());
	}
	outStr += buf;
}

void ProtoGeneratorCpp::genStruct(std::string& outStr, ProtoStruct& s, bool isMsg) {
	char buf[4096];
	if (isMsg){
		sprintf_s(buf, 4096, "	class %s : public ProtocolBase {\n", s.name.c_str());
	} else {
		sprintf_s(buf, 4096, "	class %s {\n", s.name.c_str());
	}
	outStr += buf;
	outStr += "	public:\n";

	for (int i = 0; i < s.memberVec.size(); i++){
		ProtoStructMember member = s.memberVec[i];
		genMember(outStr, member);
	}

	outStr += "\n";

	if (isMsg)
	{
		sprintf_s(buf, 4096, "		%s() : ProtocolBase((uint32_t)e_%s) {}\n", s.name.c_str(), s.name.c_str());
	} else {
		sprintf_s(buf, 4096, "		%s(){}\n", s.name.c_str());
	}
	outStr += buf;

	genPack(outStr, s, isMsg);
	genUnpack(outStr, s, isMsg);

	outStr += "	};\n\n";
}

void ProtoGeneratorCpp::genEnum(std::string& outStr, ProtoEnum& e) {
	outStr += "	enum " + e.name + "{\n";

	char buf[4096];
	for (int i = 0; i < e.itemVec.size(); i++)
	{
		sprintf_s(buf, 4096, "		%s_%s = %d,\n", e.name.c_str(), e.itemVec[i].name.c_str(), e.itemVec[i].value);
		outStr += buf;
	}

	outStr += "	};\n\n";
}

void ProtoGeneratorCpp::genMsgIndex(std::string& outStr, std::vector<ProtoStruct>& msgVec) {
	std::map<std::string, std::vector<std::string>> mapGroup;

	for (int i = 0; i < msgVec.size(); i++){
		std::string group = msgVec[i].group;
		if (mapGroup.count(group) == 0) {
			std::vector<std::string> vec;
			mapGroup[group] = vec;
		}
		mapGroup[group].push_back(msgVec[i].name);
	}

	char buf[4096];
	outStr += "namespace protocol{\n";
	int i = 0;
	for (auto iter = mapGroup.begin(); iter != mapGroup.end(); ++iter, ++i){
		outStr += "	enum " + iter->first + "{\n";
		sprintf_s(buf, 4096, "		e%s_BEGIN = %d,\n", iter->first.c_str(), i*1000);
		outStr += buf;

		for (int j = 0; j < iter->second.size(); j++)
		{
			sprintf_s(buf, 4096, "		e%s = %d,\n", iter->second[j].c_str(), i * 1000 + j + 1);
			outStr += buf;
		}

		sprintf_s(buf, 4096, "		e%s_END,\n", iter->first.c_str());
		outStr += buf;

		outStr += "	};\n";
	}
	outStr += "}\n\n";
}

void ProtoGeneratorCpp::genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec) {
	outStr += "namespace protocol{\n";

	for (int i = 0; i < enumVec.size(); i++) {
		genEnum(outStr, enumVec[i]);
	}

	outStr += "}\n\n";
}

void ProtoGeneratorCpp::genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec) {
	outStr += "namespace protocol{\n";
	for (int i = 0; i < structVec.size(); i++){
		genStruct(outStr, structVec[i], false);
	}
	outStr += "}\n\n";
}

void ProtoGeneratorCpp::genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec) {
	outStr += "namespace protocol{\n";
	for (int i = 0; i < msgVec.size(); i++) {
		genStruct(outStr, msgVec[i], true);
	}
	outStr += "}\n\n";
}
