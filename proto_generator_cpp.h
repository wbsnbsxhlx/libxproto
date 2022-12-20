#ifndef proto_generator_cpp_h__
#define proto_generator_cpp_h__
#include <string>
#include "libxproto.h"

class ProtoGeneratorCpp {
public:
	std::string generate(std::vector<ProtoResult>& resultVec);

	void genHeader(std::string& outStr);

	void genEnum(std::string& outStr, ProtoEnum& e);
	void genStruct(std::string& outStr, ProtoStruct& s, bool isMsg);

	void genMsgIndex(std::string& outStr, std::vector<ProtoStruct>& msgVec);
	void genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec);
	void genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec);
	void genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec);
};

#endif // proto_generator_cpp_h__
