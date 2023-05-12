#include "proto_generator_cpp_wrapper.h"
#include "libxproto.h"


std::string getTypeString(ProtoMemberType type) {
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
void ProtoGeneratorCppWrapper::genHeader(std::string& outStr) {
	outStr += "#pragma once\n\
#include \"proto.h\"\n\
#include \"property.h\"\n\
#include <stdio.h>\n\n";
}

void genSPlayerBaseData(std::string& outStr, ProtoStruct& s) {
	strAppendFmt(outStr, "class %s_Wrapper {\n", s.name.c_str());
	strAppendFmt(outStr, "private:\n");
	strAppendFmt(outStr, "	protocol::%s& _base;\n", s.name.c_str());
	strAppendFmt(outStr, "public:\n");
	strAppendFmt(outStr, "	%s_Wrapper() = delete;\n", s.name.c_str());
	strAppendFmt(outStr, "	%s_Wrapper(protocol::%s& b) :_base(b) {}\n\n", s.name.c_str(), s.name.c_str());
	for (int i = 0; i < s.memberVec.size(); i++){
		auto& member = s.memberVec[i];
		strAppendFmt(outStr, "	property<%s> %s {[this](){ return _base.%s; }, [this](auto&& value) { _base.%s = value; }};\n", getTypeString(member.type).c_str(), member.name.c_str(), member.name.c_str(), member.name.c_str());
	}
	strAppendFmt(outStr, "};\n\n");
}

void genSPlayerData(std::string& outStr, ProtoStruct& s, std::vector<ProtoStruct>& structVec) {
	strAppendFmt(outStr, "class %s_Wrapper : public protocol::%s {\n", s.name.c_str(), s.name.c_str());
	strAppendFmt(outStr, "public:\n");
	strAppendFmt(outStr, "	%s_Wrapper():\n", s.name.c_str());
	for (int i = 0; i < s.memberVec.size(); i++){
		auto& member = s.memberVec[i];
		strAppendFmt(outStr, "		%s(protocol::%s::%s)",member.name.c_str(), s.name.c_str(), member.name.c_str());
		if (i == s.memberVec.size()-1){
			strAppendFmt(outStr, "{}\n\n");
		} else {
			strAppendFmt(outStr, ",\n");
		}
	}
	
	for (int i = 0; i < s.memberVec.size(); i++) {
		auto& member = s.memberVec[i];
		strAppendFmt(outStr, "	%s_Wrapper %s;\n", member.type.name.c_str(), member.name.c_str());
	}
	strAppendFmt(outStr, "};\n\n");
}

void genSAbilityData(std::string& outStr, ProtoStruct& s) {
	strAppendFmt(outStr, "class %s_Wrapper : private protocol::%s {\n", s.name.c_str(), s.name.c_str());
	strAppendFmt(outStr, "public:\n");
	strAppendFmt(outStr, "	%s_Wrapper() {}\n", s.name.c_str());

	strAppendFmt(outStr, "	struct Elem {\n");
	strAppendFmt(outStr, "		%s_Wrapper& wrapper;\n", s.name.c_str());
	strAppendFmt(outStr, "		int index;\n");
	strAppendFmt(outStr, "		Elem(%s_Wrapper& wrapper, int index) : wrapper(wrapper), index(index) {}\n", s.name.c_str());
	strAppendFmt(outStr, "		operator uint32_t() {\n");
	strAppendFmt(outStr, "			return wrapper.datas[index];\n");
	strAppendFmt(outStr, "		}\n");
	strAppendFmt(outStr, "		uint32_t& operator=(const uint32_t& other) {\n");
	strAppendFmt(outStr, "			return wrapper.datas[index] = other;\n");
	strAppendFmt(outStr, "		}\n");
	strAppendFmt(outStr, "	};\n");

	strAppendFmt(outStr, "	Elem operator[](int index) {");
	strAppendFmt(outStr, "	return Elem(*this, index);");
	strAppendFmt(outStr, "	}\n");

	strAppendFmt(outStr, "};\n\n");
}

std::string ProtoGeneratorCppWrapper::generate(std::vector<ProtoResult>& resultVec) {
	std::string ret;

	ProtoResult result;
	for (int i = 0; i < resultVec.size(); i++) {
		result.enumList.insert(result.enumList.end(), resultVec[i].enumList.begin(), resultVec[i].enumList.end());
		result.msgList.insert(result.msgList.end(), resultVec[i].msgList.begin(), resultVec[i].msgList.end());
		result.structList.insert(result.structList.end(), resultVec[i].structList.begin(), resultVec[i].structList.end());
	}

	genHeader(ret);
	for (int i = 0; i < result.structList.size(); i++) {
		if (result.structList[i].name == "SPlayerData") {
			genSPlayerData(ret, result.structList[i], result.structList);
		} else if (result.structList[i].name == "SPlayerBaseData") {
			genSPlayerBaseData(ret, result.structList[i]);
		} else if (result.structList[i].name == "SAbilityData") {
			genSAbilityData(ret, result.structList[i]);
		}
	}

	return ret;
}