#include <stdio.h>
#include "proto_parser.h"
#include <vector>
#include <string>
#include <fstream>
#include <io.h>
#include "proto_generator_cpp.h"

using namespace std;

void getAllFiles(string path, vector<string>& files) {
	long hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*.proto").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					getAllFiles(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			} else {
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

	ProtoGeneratorCpp g;
	g.generate(resultVec);

	getchar();
	return 0;
}