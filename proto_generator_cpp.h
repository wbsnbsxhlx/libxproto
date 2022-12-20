#ifndef proto_generator_cpp_h__
#define proto_generator_cpp_h__
#include <string>
#include "libxproto.h"

class ProtoGeneratorCpp {
public:
	std::string generate(std::vector<ProtoResult>& resultVec);

	std::string getTypeString(ProtoMemberType type);

	void genHeader(std::string& outStr);

	void genPackBaseType(std::string& outStr, ProtoStructMember& member);

	void genPack(std::string& outStr, ProtoStruct& s, bool isMsg);
	void genUnpack(std::string& outStr, ProtoStruct& s, bool isMsg);

	void genEnum(std::string& outStr, ProtoEnum& e);
	void genStruct(std::string& outStr, ProtoStruct& s, bool isMsg);
	void genMember(std::string& outStr, ProtoStructMember& member);

	void genMsgIndex(std::string& outStr, std::vector<ProtoStruct>& msgVec);
	void genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec);
	void genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec);
	void genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec);
};

#endif // proto_generator_cpp_h__
