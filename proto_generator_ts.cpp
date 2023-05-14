#include "proto_generator_ts.h"
#include "proto_lex.h"

void ProtoGeneratorTs::genHeader(std::string& outStr) {
	outStr += "import Xpack from '../libxpack-ts/libxpack';\n\n\
export namespace proto {\n\
	export class ProtocolBase {\n\
		private protocol: number;\n\
	    constructor(proto: number) {\n\
	        this.protocol = proto;\n\
	    }\n\n\
	    pack(packer:Xpack){\n\
	        packer.packUint(this.protocol);\n\
	    }\n\
	}\n\n";
}

std::string ProtoGeneratorTs::getTypeString(ProtoMemberType type) {
	switch (type.type) {
		case TN_STRING:
			return "string";
		case TN_DOUBLE:
		case TN_FLOAT:
		case TN_INT32:
		case TN_INT64:
		case TN_UINT32:
		case TN_UINT64:
			return "number";
		case TN_BOOL:
			return "boolean";
		case TN_BINARY:
			return "string";
		case TN_STRUCT:
			return type.name;
		case TN_ENUM:
			return type.name;
	}

	return "";
}

std::string ProtoGeneratorTs::getTypeDefaultValue(ProtoMemberType type){
	switch (type.type) {
	case TN_STRING:
		return "= \"\"";
	case TN_DOUBLE:
	case TN_FLOAT:
	case TN_INT32:
	case TN_INT64:
	case TN_UINT32:
	case TN_UINT64:
	case TN_ENUM:
		return "= 0";
	case TN_BOOL:
		return "= false";
	case TN_BINARY:
		return "";
	case TN_STRUCT:
		return "= new " + type.name + "()";
	}

	return "";
}

std::string ProtoGeneratorTs::generate(std::vector<ProtoResult>& resultVec) {
	std::string ret;

	ProtoResult result;
	for (int i = 0; i < resultVec.size(); i++) {
		result.enumList.insert(result.enumList.begin(), resultVec[i].enumList.begin(), resultVec[i].enumList.end());
		result.msgList.insert(result.msgList.begin(), resultVec[i].msgList.begin(), resultVec[i].msgList.end());
		result.structList.insert(result.structList.begin(), resultVec[i].structList.begin(), resultVec[i].structList.end());
	}
	genHeader(ret);

	genMsgIndex(ret, result.msgList);
	genEnumList(ret, result.enumList);
	genStructList(ret, result.structList);
	genMsgList(ret, result.msgList);

	ret += "}\n";

	return ret;
}

void ProtoGeneratorTs::genUnpackStruct(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) : (this.%s.unpack(packer), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
}

void ProtoGeneratorTs::genUnpackEnum(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) : (this.%s = packer.unpackUint(), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
}

void ProtoGeneratorTs::genUnpackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "				for(let i = 0; i < size; i++) this.%s.push(packer.unpackInt());\n", member.name.c_str());
			break;
		case TN_ENUM:
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "				for(let i = 0; i < size; i++) this.%s.push(packer.unpackUint());\n", member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "				for(let i = 0; i < size; i++) this.%s.push(packer.unpackFloat());\n", member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "				for(let i = 0; i < size; i++) {\n");
			strAppendFmt(outStr, "					this.%s.push(new TextDecoder().decode(packer.unpackRaw()));\n				}\n", member.name.c_str());
			break;
	}
}

void ProtoGeneratorTs::genUnpackBaseType(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) : (this.%s = packer.unpackInt(), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) :  (this.%s = packer.unpackUint(), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) :  (this.%s = packer.unpackFloat(), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) : (this.%s = new TextDecoder().decode(packer.unpackRaw()), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "			field != counter ? (counter++, this.%s %s) :  (this.%s = packer.unpackUint(), field = packer.unpackUint(), counter = 1);\n", member.name.c_str(), getTypeDefaultValue(member.type).c_str(), member.name.c_str());
			break;
	}
}

void ProtoGeneratorTs::genUnpackRepeat(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			if (field != counter) (counter++, this.%s = []);\n", member.name.c_str());
	outStr += "			else {\n";
	strAppendFmt(outStr, "				var size = packer.unpackUint();\n");

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				for(let i = 0; i < size; i++) {\n");
		strAppendFmt(outStr, "					var item = new %s();\n", member.type.name.c_str());
		strAppendFmt(outStr, "					item.unpack(packer);\n");
		strAppendFmt(outStr, "					this.%s.push(item)\n				}\n", member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				for (int i = 0; i < size; i++) this.%s.push(packer.unpackUint());\n", member.name.c_str());
	} else if (ProtoLex::isBaseType(member.type.type)) {
		genUnpackBaseTypeForRepeat(outStr, member);
	}
	strAppendFmt(outStr, "				field = packer.unpackUint();\n");
	strAppendFmt(outStr, "				counter = 1;\n");
	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorTs::genUnpack(std::string& outStr, ProtoStruct& s, bool isMsg) {
	strAppendFmt(outStr, "		unpack(packer:Xpack) {\n");
	strAppendFmt(outStr, "			var field = packer.unpackUint(), counter = 1;\n");

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		if (member.fieldRule == TN_REPEATED) {
			genUnpackRepeat(outStr, member);
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

	strAppendFmt(outStr, "			field == 0 || packer.unpackUint();\n");
	strAppendFmt(outStr, "		}\n");
}

void ProtoGeneratorTs::genPackStruct(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "				packer.packUint(field), field = 1;\n");
	strAppendFmt(outStr, "				this.%s.pack(packer);\n", member.name.c_str());
}

void ProtoGeneratorTs::genPackEnum(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			this.%s == 0 ? field++ : (packer.packUint(field), packer.packUint(this.%s), field = 1);\n", member.name.c_str(), member.name.c_str());
}

void ProtoGeneratorTs::genPackRepeat(std::string& outStr, ProtoStructMember& member) {
	strAppendFmt(outStr, "			this.%s.length == 0 ? field++ : (packer.packUint(field), packer.packUint(this.%s.length), field = 1);;\n", member.name.c_str(), member.name.c_str());
	strAppendFmt(outStr, "			for (let i = 0; i < this.%s.length; i++) {\n", member.name.c_str());

	if (member.type.type == TN_STRUCT) {
		strAppendFmt(outStr, "				this.%s[i].pack(packer);\n", member.name.c_str());
	} else if (member.type.type == TN_ENUM) {
		strAppendFmt(outStr, "				packer.packUint(this.%s[i]);\n", member.name.c_str());
	} else if (ProtoLex::isBaseType(member.type.type)) {
		genPackBaseTypeForRepeat(outStr, member);
	}

	strAppendFmt(outStr, "			}\n\n");
}

void ProtoGeneratorTs::genPackBaseTypeForRepeat(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "				packer.packInt(this.%s[i]);\n", member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "				packer.packUint(this.%s[i]);\n", member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "				packer.packFloat(this.%s[i]);\n", member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "				packer.packRaw(new TextEncoder().encode(this.%s[i]));\n", member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "				packer.packUint(this.%s[i]);\n", member.name.c_str());
			break;
	}
}

void ProtoGeneratorTs::genPackBaseType(std::string& outStr, ProtoStructMember& member) {
	switch (member.type.type) {
		case TN_INT32:
		case TN_INT64:
			strAppendFmt(outStr, "			this.%s == 0 ? field++ : (packer.packUint(field), packer.packInt(this.%s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_UINT32:
		case TN_UINT64:
			strAppendFmt(outStr, "			this.%s == 0 ? field++ : (packer.packUint(field), packer.packUint(this.%s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_FLOAT:
		case TN_DOUBLE:
			strAppendFmt(outStr, "			this.%s == 0 ? field++ : (packer.packUint(field), packer.packFloat(this.%s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
		case TN_STRING:
			strAppendFmt(outStr, "			this.%s.length == 0 ? field++ : (packer.packUint(field), packer.packRaw(new TextEncoder().encode(this.%s)), field = 1);\n", member.name.c_str(), member.name.c_str(), member.name.c_str());
			break;
		case TN_ENUM:
			strAppendFmt(outStr, "			this.%s == 0 ? field++ : (packer.packUint(field), packer.packUint(this.%s), field = 1);\n", member.name.c_str(), member.name.c_str());
			break;
	}
}

void ProtoGeneratorTs::genPack(std::string& outStr, ProtoStruct& s, bool isMsg) {
	strAppendFmt(outStr, "		pack(packer:Xpack) {\n");
	if (isMsg) {
		strAppendFmt(outStr, "			super.pack(packer);\n");
	}
	strAppendFmt(outStr, "			var field = 1;\n");

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		if (member.fieldRule == TN_REPEATED) {
			genPackRepeat(outStr, member);
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

	strAppendFmt(outStr, "			packer.packUint(0);\n");
	strAppendFmt(outStr, "		}\n\n");
}

void ProtoGeneratorTs::genMember(std::string& outStr, ProtoStructMember& member) {
	if (member.fieldRule == TN_REPEATED) {
		strAppendFmt(outStr, "		%s:Array<%s>;\n", member.name.c_str(), getTypeString(member.type).c_str());
	} else {
		strAppendFmt(outStr, "		%s:%s;\n", member.name.c_str(), getTypeString(member.type).c_str());
	}
}

void ProtoGeneratorTs::genStruct(std::string& outStr, ProtoStruct& s, bool isMsg) {
	if (isMsg) {
		strAppendFmt(outStr, "	export class %s extends ProtocolBase {\n", s.name.c_str());
	} else {
		strAppendFmt(outStr, "	export class %s {\n", s.name.c_str());
	}

	for (int i = 0; i < s.memberVec.size(); i++) {
		ProtoStructMember member = s.memberVec[i];
		genMember(outStr, member);
	}

	outStr += "\n";

	if (isMsg) {
		strAppendFmt(outStr, "		constructor() {super(%s.e%s);}\n", s.group.c_str(), s.name.c_str());
	}

	genPack(outStr, s, isMsg);
	genUnpack(outStr, s, isMsg);

	strAppendFmt(outStr, "	}\n\n");
}

void ProtoGeneratorTs::genEnum(std::string& outStr, ProtoEnum& e) {
	strAppendFmt(outStr, "	export enum %s {\n", e.name.c_str());

	for (int i = 0; i < e.itemVec.size(); i++) {
		strAppendFmt(outStr, "		%s = %d,\n", e.itemVec[i].name.c_str(), e.itemVec[i].value);
	}

	strAppendFmt(outStr, "	}\n\n");
}

void ProtoGeneratorTs::genMsgIndex(std::string& outStr, std::vector<ProtoStruct>& msgVec) {
	std::map<std::string, std::vector<std::string>> mapGroup;

	for (int i = 0; i < msgVec.size(); i++) {
		std::string group = msgVec[i].group;
		if (mapGroup.count(group) == 0) {
			std::vector<std::string> vec;
			mapGroup[group] = vec;
		}
		mapGroup[group].push_back(msgVec[i].name);
	}

	int i = 0;
	for (auto iter = mapGroup.begin(); iter != mapGroup.end(); ++iter, ++i) {
		strAppendFmt(outStr, "	export enum %s{\n", iter->first.c_str());
		strAppendFmt(outStr, "		e%s_BEGIN = %d,\n", iter->first.c_str(), i * 1000);

		for (int j = 0; j < iter->second.size(); j++) {
			strAppendFmt(outStr, "		e%s = %d,\n", iter->second[j].c_str(), i * 1000 + j + 1);
		}

		strAppendFmt(outStr, "		e%s_END,\n", iter->first.c_str());
		strAppendFmt(outStr, "	}\n\n");
	}
}

void ProtoGeneratorTs::genEnumList(std::string& outStr, std::vector<ProtoEnum>& enumVec) {
	for (int i = 0; i < enumVec.size(); i++) {
		genEnum(outStr, enumVec[i]);
	}
}

void ProtoGeneratorTs::genStructList(std::string& outStr, std::vector<ProtoStruct>& structVec) {
	for (int i = 0; i < structVec.size(); i++) {
		genStruct(outStr, structVec[i], false);
	}
}

void ProtoGeneratorTs::genMsgList(std::string& outStr, std::vector<ProtoStruct>& msgVec) {
	for (int i = 0; i < msgVec.size(); i++) {
		genStruct(outStr, msgVec[i], true);
	}
}
