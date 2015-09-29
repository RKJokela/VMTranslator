#include <dirent.h>
#include <iostream>
#include <string>
#include <list>
#include <algorithm>

#include "CodeWriter.h"
#include "VMParser.h"

using namespace std;

#undef main
extern "C" {
int main (int argc, char* argv[]) {
	string source;
	bool useDir = false;
	bool sysFound = false;
	if (argc < 2) {
		// cerr << endl
		     // << "Usage: vmt source, where source is either:\n"
		     // << " a file name ending in .vm, or\n"
			 // << " a directory containing one or more .vm files." << endl;
		// return -1;
		cout << "Enter the source file or directory: ";
		cin >> source;
		cout << endl;
	}
	else
		source = argv[1];
	string outFileName = source;
	list<string> inputFiles;
	if (source.length() > 3 && source.substr(source.length() - 3) == ".vm") {
		cout << "Using single file: " << source << endl;
		outFileName.erase(outFileName.length() - 3);
		inputFiles.push_back(source);
		if (source.find("Sys.vm") == 0)
			sysFound = true;
	}
	else {
		useDir = true;
		cout << "Using directory name: " << source << endl;
		auto d = opendir(source.c_str());
		if (!d) {
			cerr << "Error opening " << source << " directory" << endl;
			return -1;
		}
		dirent* ent = NULL;
		while (ent = readdir(d)) {
			string tmp(ent->d_name);
			if (tmp.length() < 4)
				continue;
			if (tmp.substr(tmp.length() - 3) == ".vm") {
				inputFiles.push_back(tmp);
				if (tmp.find("Sys.vm") != string::npos)
					sysFound = true;
			}
		}
		if (inputFiles.empty()) {
			cerr << "No .vm files found in " << source << " directory" << endl;
			return -1;
		}
		source += "\\";
	}
	
	outFileName += ".asm";
	
	CodeWriter writer(outFileName);
	VMParser reader;
	if (sysFound)
		cout << "Sys.vm found: using user-defined Sys.init()" << endl;
	writer.init(!sysFound);
	while (!inputFiles.empty()) {
		// get next file
		string curFile = (useDir ? source : "") + inputFiles.front();
		// load the instructions
		reader.load(curFile);
		// write the assembly code
		int num = writer.write(reader);
		// check it off the list
		inputFiles.pop_front();
		// print a message
		cout << "Finished processing " << curFile << " (" << num << " code lines)" << endl;
	}
	writer.close();
	
	cout << "Done creating " << outFileName << "." << endl;
	
	return 0;
}
}
