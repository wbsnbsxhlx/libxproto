#pragma once
#include "proto_generator.h"

class ProtoGeneratorCppWrapper : public ProtoGenerator {
public:
	virtual std::string generate(std::vector<ProtoResult>& resultVec);


	std::string getTypeString(ProtoMemberType type){}

	void genHeader(std::string& outStr);
	void genPackStruct(std::string& outStr, ProtoStructMember& member){}
	void genPackEnum(std::string& outStr, ProtoStructMember& member){}
	void genPackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member){}
	void genPackBaseType(std::string& outStr, ProtoStructMember& member){}
	void genPackRepeat(std::string& outStr, ProtoStructMember& member){}

	void genUnpackStruct(std::string& outStr, ProtoStructMember& member){}
	void genUnpackEnum(std::string& outStr, ProtoStructMember& member){}
	void genUnpackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member){}
	void genUnpackBaseType(std::string& outStr, ProtoStructMember& member){}
	void genUnpackRepeat(std::string& outStr, ProtoStructMember& member){}

	void genReset(std::string& outStr){}
	void genPack(std::string& outStr, ProtoStruct& s, bool isMsg){}
	void genUnpack(std::string& outStr, ProtoStruct& s, bool isMsg){}

	void genEnum(std::string& outStr, ProtoEnum& e){}
	void genStruct(std::string& outStr, ProtoStruct& s, bool isMsg){}
	void genMember(std::string& outStr, ProtoStructMember& member){}

	void genMsgIndex(std::string& outStr, std::vector<ProtoStruct>& msgVec){}
	void genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec){}
	void genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec){}
	void genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec){}
};
