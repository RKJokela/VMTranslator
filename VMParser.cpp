#include "VMParser.h"
#include <fstream>

#define WHITESPACE " \t"
#define COMMENT_MARK "//"

using namespace std;

VMParser::VMParser() : lineCount(0) {

}

bool VMParser::load(std::string& inFile) {
	ifstream fin(inFile);
	if (!fin)
		return false;
	currentFile = inFile;
	lineCount = 0;
	while (fin) {
		char cs[80];
		fin.getline(cs, 80);
		lineCount++;
		if (!cs[0])
			continue;
		string s(cs);
		parseLine(s);
	}
	fin.close();
	return true;
}

bool VMParser::hasMoreInstructions() {
	return (!instructions.empty());
}

void VMParser::popInst(VMInstruction& inst) {
	if (!instructions.empty()) {
		inst = instructions.front();
		instructions.pop_front();
	}
}

const std::string& VMParser::getCurrentFile() { return currentFile; }

inline void skipLeadingWS(string& s) {
	while (s.find_first_of(WHITESPACE) == 0)
		s.erase(0, 1);
}

inline void removeComments(string& s) {
	size_t pos = s.find(COMMENT_MARK);
	if (pos != string::npos)
		s.erase(pos);
}

inline string getNextToken(string& s) {
	string tok = "";
	skipLeadingWS(s);
	size_t pos = s.find_first_of(WHITESPACE);
	//if (pos != string::npos) {
		tok = s.substr(0, pos);
		s.erase(0, pos);
	//}
	return tok;
}

// assumes given string is a single word with no whitespace
int getCommand(const string& s) {
	int ret = VM_ERR;
	
	switch (toupper(s[0])) {
	case 'A':	// add, and
		if (toupper(s[1]) == 'N')
			ret = VM_AND;
		else if (toupper(s[1]) == 'D')
			ret = VM_ADD;
		break;
	case 'C':	// call
		ret = VM_CALL;
		break;
	case 'E':	// eq
		ret = VM_EQ;
		break;
	case 'F':	// func
		ret = VM_FUNC;
		break;
	case 'G':	// gt, goto
		if (toupper(s[1]) == 'T')
			ret = VM_GT;
		else if (toupper(s[1]) == 'O')
			ret = VM_GOTO;
		break;
	case 'I':	// ifgoto
		ret = VM_IFGOTO;
		break;
	case 'L':	// lt, lbl
		if (toupper(s[1]) == 'T')
			ret = VM_LT;
		else if (toupper(s[1]) == 'A')
			ret = VM_LBL;
		break;
	case 'N':	// neg, not
		if (toupper(s[1]) == 'E')
			ret = VM_NEG;
		else if (toupper(s[1]) == 'O')
			ret = VM_NOT;
		break;
	case 'O':	// or
		ret = VM_OR;
		break;
	case 'P':	// push, pop
		if (toupper(s[1]) == 'U')
			ret = VM_PUSH;
		else if (toupper(s[1]) == 'O')
			ret = VM_POP;
		break;
	case 'R':	// ret
		ret = VM_RET;
		break;
	case 'S':	// sub
		ret = VM_SUB;
		break;
	default:
		break;
	}
	
	return ret;
}

void VMParser::parseLine(string& s) {
	removeComments(s);
	string tok = getNextToken(s);
	if (!tok.empty()) {
		int opcode = getCommand(tok);
		string arg1 = getNextToken(s);
		string arg2 = getNextToken(s);
		
		// on function commands, store the function name
		if (opcode == VM_FUNC)
			currentFunc = arg1;
		
		// generate string for unique variable names
		//  format is "file.function" (function can be blank)
		string scope = currentFunc;
		
		instructions.push_back(VMInstruction(opcode, arg1, arg2, scope, lineCount));
	}
}
	
