#ifndef CODE_WRITER_H_INCLUDED
#define CODE_WRITER_H_INCLUDED

#include <string>
#include <fstream>
#include "VMParser.h"

class CodeWriter {
public:
	CodeWriter(const std::string& outFile);
	bool init(bool defaultSys = true);
	int write(VMParser& parser);
	bool writeInstruction(const VMInstruction& inst, const std::string& file);
	// Annotate the assembly listing
	void AnnotateInstruction(std::ostream& out, const VMInstruction& inst, const std::string& file, int line);
	void close();
	
private:
	std::string filename;
	std::ofstream fout;
	void badInstruction(const VMInstruction& inst, const std::string& file);
	// write startup code
	void sysInit();
	// Full translation blocks
	bool push(const std::string& segment, const std::string& index, const std::string& scope);
	bool pop(const std::string& segment, const std::string& index, const std::string& scope);
	bool call(const std::string& function, const std::string& argc);
	void printCallRoutine();
	bool function(const std::string& name, const std::string& localc);
	void ret();
	void printRetRoutine();
	void label(const std::string& label, const std::string& scope);
	void goto_(const std::string& label, const std::string& scope);
	void if_goto(const std::string& label, const std::string& scope);
	// push & pop subroutines
	void pushConstant(int value);
	void pushAddress(int address);
	void popAddress(int address);
	void pushIndexed(int i, const char* label);
	void popIndexed (int i, const char* label);
	// Primitive instruction translation blocks
	inline void loadDMem(int addr);
	inline void loadDConst(int immed);
	inline void storeD(int addr);
	inline void pushD();
	inline void popD();
	inline void getPopAddr();
	inline void peekStack();
	inline void pushTrue();
	inline void pushFalse();
	inline void decStack(int n);
};

#endif // CODE_WRITER_H_INCLUDED
