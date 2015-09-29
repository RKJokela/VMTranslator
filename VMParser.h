#ifndef VM_PARSER_H_INCLUDED
#define VM_PARSER_H_INCLUDED

#include <string>
#include <list>

enum VMCommand {
	// This type signifies an error reading the instruction
	VM_ERR = -1,
	// Arithmetic
	VM_ADD = 0,
	VM_SUB,
	VM_NEG,
	// Logical
	VM_EQ,
	VM_GT,
	VM_LT,
	VM_AND,
	VM_OR,
	VM_NOT,
	// Memory
	VM_PUSH,
	VM_POP,
	// Program flow
	VM_LBL,
	VM_GOTO,
	VM_IFGOTO,
	// Function calling
	VM_FUNC,
	VM_CALL,
	VM_RET
};

// when coding instructions that do not use arg1 and/or arg2, they will be ignored
struct VMInstruction {
	VMCommand opcode;
	std::string arg1;
	std::string arg2;
	std::string scope;
	int line;
	
	VMInstruction(int code, const std::string& a1, const std::string& a2, const std::string& sc, int l) :
		opcode((VMCommand)code), arg1(a1), arg2(a2), scope(sc), line(l) {}
	void operator=(const VMInstruction& right) {
		opcode = right.opcode; arg1 = right.arg1; arg2 = right.arg2; scope = right.scope; line = right.line; }
};

class VMParser {
public:
	VMParser();
	bool load(std::string& inFile);
	bool hasMoreInstructions();
	void popInst(VMInstruction& inst);
	const std::string& getCurrentFile();
	
private:
	int lineCount;
	void parseLine(std::string& s);
	std::string currentFile;
	std::string currentFunc;
	std::list<VMInstruction> instructions;
};

#endif // VM_PARSER_H_INCLUDED
