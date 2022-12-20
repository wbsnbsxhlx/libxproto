#include "proto_generator_cpp.h"

void ProtoGeneratorCpp::genHeader(std::string& outStr) {
	outStr += "#pragma once\n\
#include <vector>\n\
#include \"libxpack.h\"\n\n\
			  \
namespace protocol {\
\
	class ProtocolBase {\
	public:\
		uint32_t protocol;\
		\
		ProtocolBase(int32_t proto) {\
			protocol = proto;\
		}\
		\
		virtual ~ProtocolBase() {}\
		\
		virtual void Pack(packer_t packer) {\
			pack_pack_uint(packer, protocol);\
		}\
	};\
	\
}\n\n";
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

void ProtoGeneratorCpp::genStruct(std::string& outStr, ProtoStruct& s, bool isMsg) {
	char buf[4096];
	if (isMsg){
		sprintf_s(buf, 4096, "	class %s : public ProtocolBase {\n", s.name);
	} else {
		sprintf_s(buf, 4096, "	class %s {\n", s.name);
	}
	outStr += buf;
	outStr += "	public:\n";

	for (int i = 0; i < s.memberVec.size(); i++){
		ProtoStructMember member = s.memberVec[i];
		gen_member(outStr, member);
	}

	if (isMsg)
	{
		sprintf_s(buf, 4096, "	%s() : ProtocolBase((uint32_t)e_%s) {}\n", s.name, s.name);
	} else {
		sprintf_s(buf, 4096, "	%s(){}\n", s.name);
	}
	outStr += buf;

	gen_pack(outStr, s, isMsg);
	gen_unpack(outStr, s, isMsg);

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

}
