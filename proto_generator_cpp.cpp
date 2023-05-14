#include "proto_generator_cpp.h"
#include "proto_lex.h"

void ProtoGeneratorCpp::genHeader(std::string& outStr) {
	outStr += "#pragma once\n\
#include <vector>\n\
#include <string>\n\
#include \"libxpack.h\"\n\n\
namespace protocol {\n\
	class ProtocolBase {\n\
	public:\n\
		uint32_t protocol;\n\
		ProtocolBase(uint32_t proto) {\n\
			protocol = proto;\n\
		}\n\
		virtual ~ProtocolBase() {}\n\
		virtual void pack(packer_t packer) {\n\
			xpack_pack_uint(packer, protocol);\n\
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

	return "";
}

std::string ProtoGeneratorCpp::getTypeDefaultValueString(ProtoMemberType type) {
	switch (type.type) {
		case TN_STRING:
			return "";
		case TN_DOUBLE:
			return " = 0.0f";
		case TN_FLOAT:
			return " = 0.0f";
		case TN_INT32:
			return " = 0";
		case TN_INT64:
			return " = 0";
		case TN_UINT32:
			return " = 0";
		case TN_UINT64:
			return " = 0";
		case TN_BOOL:
			return " = false";
		case TN_BINARY:
			return "";
		case TN_STRUCT:
			return "";
		case TN_ENUM:
			return "= 0";
	}

	return "";
}

std::string ProtoGeneratorCpp::generate(std::vector<ProtoResult>& resultVec) {
	std::string ret;

	ProtoResult result;
	for (int i = 0; i < resultVec.size(); i++){
		result.enumList.insert(result.enumList.end(), resultVec[i].enumList.begin(), resultVec[i].enumList.end());
		result.msgList.insert(result.msgList.end(), resultVec[i].msgList.begin(), resultVec[i].msgList.end());
		result.structList.insert(result.structList.end(), resultVec[i].structList.begin(), resultVec[i].structList.end());
	}
	genHeader(ret);

	genMsgIndex(ret, result.msgList);
	genEnumList(ret, result.enumList);
	genStructList(ret, result.structList);
	genMsgList(ret, result.msgList);

	return ret;
}

void ProtoGeneratorCpp::genReset(std::string& outStr) {
	outStr += "		void reset() {\n";
	outStr += "			*this = {};\n";
	outStr += "		}\n\n";
}

void ProtoGeneratorCpp::genUnpackStruct(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			field != counter ? counter++ : (%s.unpack(packer, nullptr, 0), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str());
}

void ProtoGeneratorCpp::genUnpackEnum(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			field != counter ? counter++ : : (%s = (%s)xpack_unpack_uint(packer), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str(), member.type.name.c_str());
}

void ProtoGeneratorCpp::genUnpackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "				for (int i = 0; i < size; i++) %s.push_back((%s)xpack_unpack_int(packer));\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_ENUM:
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "				for (int i = 0; i < size; i++) %s.push_back((%s)xpack_unpack_uint(packer));\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "				for (int i = 0; i < size; i++) %s.push_back((%s)xpack_unpack_float(packer));\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "				%s.resize(size);\n", member.name.c_str());
			strAppendFmt(outStr, "				for (int i = 0; i < size; i++) {\n");
			strAppendFmt(outStr, "					char *str; size_t len;\n");
			strAppendFmt(outStr, "					xpack_unpack_raw(packer, &str, &len);\n");
			strAppendFmt(outStr, "					%s[i].assign(str, len);\n				}\n", member.name.c_str());
			break;
	}
}

void ProtoGeneratorCpp::genUnpackBaseTypeForArray(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) %s[i] = (%s)xpack_unpack_int(packer);\n", member.type.arrLength.c_str(), member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_ENUM:
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) %s[i] = (%s)xpack_unpack_uint(packer);\n", member.type.arrLength.c_str(), member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) %s[i] = (%s)xpack_unpack_float(packer);\n", member.type.arrLength.c_str(), member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) {\n");
			strAppendFmt(outStr, "					char *str; size_t len;\n");
			strAppendFmt(outStr, "					xpack_unpack_raw(packer, &str, &len);\n");
			strAppendFmt(outStr, "				%s.assign(str, len);\n			}\n", member.type.arrLength.c_str(), member.name.c_str());
			break;
	}
}

void ProtoGeneratorCpp::genUnpackBaseType(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "			field != counter ? counter++ : (%s = (%s)xpack_unpack_int(packer), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "			field != counter ? counter++ : (%s = (%s)xpack_unpack_uint(packer), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "			field != counter ? counter++ : (%s = (%s)xpack_unpack_float(packer), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str(), getTypeString(member.type).c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "			field != counter ? counter++ : (xpack_unpack_raw(packer, &data, &size), %s.assign((char*)data, size), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "			field != counter ? counter++ : (%s = (%s)xpack_unpack_uint(packer), field = (size_t)xpack_unpack_uint(packer), counter = 1);\n", member.name.c_str(), member.type.name);
			break;
	}
}

void ProtoGeneratorCpp::genUnpackRepeat(std::string& outStr, ProtoStructMember& member) {
	outStr += "			if (field != counter) counter++;\n";
	outStr += "			else {\n";

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				%s.resize(xpack_unpack_uint(packer));\n", member.name.c_str());
		strAppendFmt(outStr, "				for (int i = 0; i < %s.size(); i++) %s[i].unpack(packer);\n", member.name.c_str(), member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				size = (size_t)xpack_unpack_uint(packer);\n");
		strAppendFmt(outStr, "				for (int i = 0; i < size; i++) %s.push_back((%s)xpack_unpack_uint(packer));\n", member.name.c_str(), member.type.name.c_str());
	} else if (ProtoLex::isBaseType(member.type.type)) {
		strAppendFmt(outStr, "				size = (size_t)xpack_unpack_uint(packer);\n");
		genUnpackBaseTypeForRepeat(outStr, member);
	}
	strAppendFmt(outStr, "				field = (size_t)xpack_unpack_uint(packer), counter = 1;\n");
	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorCpp::genUnpackArray(std::string& outStr, ProtoStructMember& member) {
	outStr += "			if (field != counter) counter++;\n";
	outStr += "			else {\n";

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) %s[i].unpack(packer);\n", member.type.arrLength.c_str(), member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				for (int i = 0; i < %s; i++) %s[i] = (%s)xpack_unpack_uint(packer);\n", member.type.arrLength.c_str(), member.name.c_str(), member.type.name.c_str());
	} else if (ProtoLex::isBaseType(member.type.type)) {
		genUnpackBaseTypeForArray(outStr, member);
	}
	strAppendFmt(outStr, "				field = (size_t)xpack_unpack_uint(packer), counter = 1;\n");
	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorCpp::genUnpack(std::string& outStr, ProtoStruct& s, bool isMsg) {
	strAppendFmt(outStr, "		void unpack(packer_t packer, void* data = nullptr, size_t size = 0) {\n");
	strAppendFmt(outStr, "			reset();\n");
	if (isMsg) {
		strAppendFmt(outStr, "			if (data == nullptr) {\n");
		strAppendFmt(outStr, "				xpack_unpack_init(packer, data, size);\n");
		strAppendFmt(outStr, "			}\n\n");
	}
	strAppendFmt(outStr, "			size_t field = (size_t)xpack_unpack_uint(packer), counter = 1;\n");

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		if (member.fieldRule == TN_REPEATED) {
			if (member.type.arrLength.empty())
				genUnpackRepeat(outStr, member);
			else
				genUnpackArray(outStr, member);
		} else {
			if (member.type.type == TN_ENUM) {
				genUnpackEnum(outStr, member);
			} else if (member.type.type == TN_STRUCT) {
				genUnpackStruct(outStr, member);
			} else if (ProtoLex::isBaseType(member.type.type)) {
				genUnpackBaseType(outStr, member);
			}
		}
	}

	strAppendFmt(outStr, "			field == 0 || xpack_unpack_uint(packer);\n");
	strAppendFmt(outStr, "		}\n\n");
}

void ProtoGeneratorCpp::genPackStruct(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			xpack_pack_uint(packer, field), field = 1;\n");
	strAppendFmt(outStr, "			%s.pack(packer);\n", member.name.c_str());
}

void ProtoGeneratorCpp::genPackEnum(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			(uint32_t)%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_uint(packer, (uint32_t)%s), field = 1);\n", member.name.c_str(), member.name.c_str());
}

void ProtoGeneratorCpp::genPackRepeat(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			%s.size() == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_uint(packer, %s.size()), field = 1);\n", member.name.c_str(), member.name.c_str());
	strAppendFmt(outStr, "			for (int i = 0; i < %s.size(); i++) {\n", member.name.c_str());

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				%s[i].pack(packer);\n", member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				xpack_pack_uint(packer, (uint32_t)%s[i]);\n", member.name.c_str());
	}else if (ProtoLex::isBaseType(member.type.type)){
		genPackBaseTypeForRepeat(outStr, member);
	}

	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorCpp::genPackArray(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_uint(packer, %s), field = 1);\n", member.type.arrLength.c_str(), member.type.arrLength.c_str());
	strAppendFmt(outStr, "			for (int i = 0; i < %s; i++) {\n", member.type.arrLength.c_str());

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				%s[i].pack(packer);\n", member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				xpack_pack_uint(packer, (uint32_t)%s[i]);\n", member.name.c_str());
	} else if (ProtoLex::isBaseType(member.type.type)) {
		genPackBaseTypeForRepeat(outStr, member);
	}

	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorCpp::genPackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "				xpack_pack_int(packer, %s[i]);\n", member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "				xpack_pack_uint(packer, %s[i]);\n", member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "				xpack_pack_float(packer, %s[i]);\n", member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "				xpack_pack_raw(packer, (void*)%s[i].c_str(), %s[i].size());\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "				xpack_pack_uint(packer, (uint32_t)%s[i]);\n", member.name.c_str());
			break;
	}
}

void ProtoGeneratorCpp::genPackBaseType(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "			%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_int(packer, %s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "			%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_uint(packer, %s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "			%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_float(packer, %s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "			%s.size() == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_raw(packer, (void*)%s.c_str(), %s.size()), field = 1);\n", member.name.c_str(), member.name.c_str(), member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "			(uint32_t)%s == 0 ? field++ : (xpack_pack_uint(packer, field), xpack_pack_uint(packer, (uint32_t)%s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
	}
}

void ProtoGeneratorCpp::genPack(std::string& outStr, ProtoStruct& s, bool isMsg) {
	strAppendFmt(outStr, "		void pack(packer_t packer) {\n");
	if (isMsg) {
		strAppendFmt(outStr, "			ProtocolBase::pack(packer);\n");
	}
	strAppendFmt(outStr, "			size_t field = 1;\n");

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		if (member.fieldRule == TN_REPEATED) {
			if (member.type.arrLength.empty())
				genPackRepeat(outStr, member);
			else
				genPackArray(outStr, member);
		} else {
			if (member.type.type == TN_ENUM) {
				genPackEnum(outStr, member);
			} else if (member.type.type == TN_STRUCT) {
				genPackStruct(outStr, member);
			} else if (ProtoLex::isBaseType(member.type.type)) {
				genPackBaseType(outStr, member);
			}
		}
	}

	strAppendFmt(outStr, "			xpack_pack_uint(packer, 0);\n");
	strAppendFmt(outStr, "		}\n\n");
}

void ProtoGeneratorCpp::genMember(std::string& outStr, ProtoStructMember& member) {
	if (member.fieldRule == TN_REPEATED) {
		if (member.type.arrLength.empty()) {
			strAppendFmt(outStr, "		std::vector<%s> %s;\n", getTypeString(member.type).c_str(), member.name.c_str());
		} else {
			strAppendFmt(outStr, "		%s %s[%s] = {};\n", getTypeString(member.type).c_str(), member.name.c_str(), member.type.arrLength.c_str());
		}
	} else {
		strAppendFmt(outStr, "		%s %s%s;\n", getTypeString(member.type).c_str(), member.name.c_str(), getTypeDefaultValueString(member.type).c_str());
	}
}

void ProtoGeneratorCpp::genStruct(std::string& outStr, ProtoStruct& s, bool isMsg) {
	if (isMsg) {
		strAppendFmt(outStr, "	class %s : public ProtocolBase {\n", s.name.c_str());
	} else {
		strAppendFmt(outStr, "	class %s {\n", s.name.c_str());
	}
	strAppendFmt(outStr, "	public:\n");

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		genMember(outStr, member);
	}

	outStr += "\n";

	if (isMsg) {
		strAppendFmt(outStr, "		%s() : ProtocolBase((uint32_t)e%s) {}\n", s.name.c_str(), s.name.c_str());
	}

	genReset(outStr);
	genPack(outStr, s, isMsg);
	genUnpack(outStr, s, isMsg);

	strAppendFmt(outStr, "	};\n\n");
}

void ProtoGeneratorCpp::genEnum(std::string& outStr, ProtoEnum& e) {
	strAppendFmt(outStr, "	enum %s {\n", e.name.c_str());

	for (int i = 0; i < e.itemVec.size(); i++){
		strAppendFmt(outStr, "		%s_%s = %d,\n", e.name.c_str(), e.itemVec[i].name.c_str(), e.itemVec[i].value);
	}

	strAppendFmt(outStr, "	};\n\n");
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

	strAppendFmt(outStr, "namespace protocol {\n");

	int i = 0;
	for (auto iter = mapGroup.begin(); iter != mapGroup.end(); ++iter, ++i){
		strAppendFmt(outStr, "	enum %s{\n", iter->first.c_str());
		strAppendFmt(outStr, "		e%s_BEGIN = %d,\n", iter->first.c_str(), i * 1000);

		for (int j = 0; j < iter->second.size(); j++) {
			strAppendFmt(outStr, "		e%s = %d,\n", iter->second[j].c_str(), i * 1000 + j + 1);
		}

		strAppendFmt(outStr, "		e%s_END,\n", iter->first.c_str());
		strAppendFmt(outStr, "	};\n");
	}
	strAppendFmt(outStr, "}\n\n");
}

void ProtoGeneratorCpp::genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec) {
	strAppendFmt(outStr, "namespace protocol {\n");

	for (int i = 0; i < enumVec.size(); i++) {
		genEnum(outStr, enumVec[i]);
	}

	strAppendFmt(outStr, "}\n\n");
}

void ProtoGeneratorCpp::genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec) {
	strAppendFmt(outStr, "namespace protocol {\n");
	for (int i = 0; i < structVec.size(); i++){
		genStruct(outStr, structVec[i], false);
	}
	strAppendFmt(outStr, "}\n\n");
}

void ProtoGeneratorCpp::genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec) {
	strAppendFmt(outStr, "namespace protocol {\n");
	for (int i = 0; i < msgVec.size(); i++) {
		genStruct(outStr, msgVec[i], true);
	}
	strAppendFmt(outStr, "}\n\n");
}
