#include <stdio.h>
#include "proto_parser.h"
#include <vector>
#include <string>
#include <fstream>
#include <io.h>
#include "proto_generator_cpp.h"
#include "proto_generator_ts.h"
#include "proto_generator_cpp_wrapper.h"

using namespace std;

void getAllFiles(string path, vector<string>& files) {
	intptr_t hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					getAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			} else if(strstr(fileinfo.name, ".proto") != NULL){
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 5){
		printf("params error!");
		return -1;
	}

	char *protoDir = argv[1];
	char *outLang = argv[2];
	char *outDir = argv[3];
	char *outFile = argv[4];

	vector<string> files;
	getAllFiles(protoDir, files);

	vector<ProtoResult> resultVec;
	for (int i = 0; i < files.size(); i++){
		ProtoResult r = proto_parse(files[i].c_str());
		resultVec.push_back(r);
	}

	ProtoGenerator* g = nullptr;

	if (strcmp(outLang, "-cpp") == 0){
		g = new ProtoGeneratorCpp();
	}else if (strcmp(outLang, "-ts") == 0) {
		g = new ProtoGeneratorTs();
	}else if (strcmp(outLang, "-cppwrapper") == 0){
		g = new ProtoGeneratorCppWrapper();
	}

	if (g == nullptr)
	{
		return -1;
	}

	std::string content = g->generate(resultVec);

	char path[256];
	sprintf_s(path, "%s\\%s", outDir, outFile);

	ofstream of;
	of.open(path, ios::trunc);
	of << content << endl;
	of.close();

	return 0;
}