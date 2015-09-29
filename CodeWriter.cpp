#include "CodeWriter.h"
#include <iostream>

#define R(n) (n)

#define SP			R(0)
#define LCL			R(1)
#define ARG			R(2)
#define BASE_PTR	R(3)
#define PTR(i)		(BASE_PTR + i)
#define BASE_TEMP	R(5)
#define TEMP(i)		(BASE_TEMP + i)

#define LR			R(15) // link register (return address)

using namespace std;

CodeWriter::CodeWriter(const string& outFile) : filename(outFile) {

}

bool CodeWriter::init(bool defaultSys) {
	fout.open(filename);
	
	loadDConst(256);
	storeD(SP); // initialize SP to 256 (start of global stack)
	if (defaultSys)
		sysInit();
	else
		fout << "\t@Sys.init\n"
			 << "\t0;jmp"
			 << endl;
			 
	printCallRoutine();
	
	printRetRoutine();
	
	return (bool)fout;
}

// Print an error message
void CodeWriter::badInstruction(const VMInstruction& inst, const string& file) {
	cerr << "ERROR: Bad instruction: ";
	AnnotateInstruction(cerr, inst, file, inst.line);
}

void CodeWriter::sysInit() {
	// here is where we set up memory segments, then call main
	
	// THIS
	loadDConst(3000);
	storeD(PTR(0));
	
	// THAT
	loadDConst(4000);
	storeD(PTR(1));
	
	fout << "// call Main.main 0 (SYS.INIT)" << endl;
	call("Main.main", "0");
	
	// enter infinite loop
	fout << "(Sys.END)\n"
	     << "\t@Sys.END\n"
	     << "\t0;jmp"
		 << endl;
}

int CodeWriter::write(VMParser& parser) {
	int count = 0;
	VMInstruction inst(0, "", "", "", 0);
	string file = parser.getCurrentFile();
	while (parser.hasMoreInstructions()) {
		parser.popInst(inst);
		fout << "// ";
		AnnotateInstruction(fout, inst, file, inst.line);
		string scope = file;
		scope.erase(file.find("."));
		size_t slash = scope.find_last_of("/\\");
		if (slash != string::npos)
			scope.erase(0, slash + 1);
		if (writeInstruction(inst, scope))
			count++;
		else
			badInstruction(inst, file);
	}
	return count;
}

bool CodeWriter::writeInstruction(const VMInstruction& inst, const string& scope) {
	switch (inst.opcode) {
	case VM_ADD:
		popD();
		peekStack();
		fout << "\tm=d+m" << endl;
		break;
	case VM_SUB:
		popD();
		peekStack();
		fout << "\tm=m-d" << endl;
		break;
	case VM_NEG:
		peekStack();
		fout << "\tm=-m" << endl;
		break;
	case VM_EQ:
		popD();
		peekStack();
		fout << "\td=d-m\n" 
		     << "\tm=-1\n"	// true
			 << "\t@pc+5\n"	// skip next instruction
			 << "\td;jeq\n"	// if they are equal
			 << "\t@sp\n"
		     << "\ta=m-1\n"
			 << "\tm=0"	// false
			 << endl;
		break;
	case VM_GT:
		popD();
		peekStack();
		fout << "\td=m-d\n" 
		     << "\tm=-1\n"	// true
			 << "\t@pc+5\n"	// skip next instruction
			 << "\td;jgt\n"	// if m>d
			 << "\t@sp\n"
		     << "\ta=m-1\n"
			 << "\tm=0"	// false
			 << endl;
		break;
	case VM_LT:
		popD();
		peekStack();
		fout << "\td=m-d\n" 
		     << "\tm=-1\n"	// true
			 << "\t@pc+5\n"	// skip next instruction
			 << "\td;jlt\n"	// if m<d
			 << "\t@sp\n"
		     << "\ta=m-1\n"
			 << "\tm=0"	// false
			 << endl;
		break;
	case VM_AND:
		popD();
		peekStack();
		fout << "\tm=d&m" << endl;
		break;
	case VM_OR:
		popD();
		peekStack();
		fout << "\tm=d|m" << endl;
		break;
	case VM_NOT:
		peekStack();
		fout << "\tm=!m" << endl;
		break;
	case VM_PUSH:
		if (!push(inst.arg1, inst.arg2, scope))
			return false;
		break;
	case VM_POP:
		if (!pop(inst.arg1, inst.arg2, scope))
			return false;
		break;
	case VM_LBL:
		label(inst.arg1, inst.scope);
		break;
	case VM_GOTO:
		goto_(inst.arg1, inst.scope);
		break;
	case VM_IFGOTO:
		if_goto(inst.arg1, inst.scope);
		break;
	case VM_FUNC:
		if (!function(inst.arg1, inst.arg2))
			return false;
		break;
	case VM_CALL:
		if (!call(inst.arg1, inst.arg2))
			return false;
		break;
	case VM_RET:
		ret();
		break;
	case VM_ERR:
	default:
		return false; // don't write anything invalid
	}
	return true;
}

void CodeWriter::AnnotateInstruction(ostream& out, const VMInstruction& inst, const std::string& file, int line) {
	switch (inst.opcode) {
	case VM_ADD:
		out << "add (" << file << " line " << line << ")" << endl;
		break;
	case VM_SUB:
		out << "sub (" << file << " line " << line << ")" << endl;
		break;
	case VM_NEG:
		out << "neg (" << file << " line " << line << ")" << endl;
		break;
	case VM_EQ:
		out << "eq (" << file << " line " << line << ")" << endl;
		break;
	case VM_GT:
		out << "gt (" << file << " line " << line << ")" << endl;
		break;
	case VM_LT:
		out << "lt (" << file << " line " << line << ")" << endl;
		break;
	case VM_AND:
		out << "and (" << file << " line " << line << ")" << endl;
		break;
	case VM_OR:
		out << "or (" << file << " line " << line << ")" << endl;
		break;
	case VM_NOT:
		out << "not (" << file << " line " << line << ")" << endl;
		break;
	case VM_PUSH:
		out << "push " << inst.arg1 << ' ' << inst.arg2 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_POP:
		out << "pop " << inst.arg1 << ' ' << inst.arg2 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_LBL:
		out << "label " << inst.arg1 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_GOTO:
		out << "goto " << inst.arg1 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_IFGOTO:
		out << "if-goto " << inst.arg1 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_FUNC:
		out << "function " << inst.arg1 << ' ' << inst.arg2 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_CALL:
		out << "call " << inst.arg1 << " (" << file << " line " << line << ")" << endl;
		break;
	case VM_RET:
		out << "ret (" << file << " line " << line << ")" << endl;
		break;
	case VM_ERR:
	default:
		out << "INVALID INSTRUCTION (" << file << " line " << line << ")" << endl;
	}
}

void CodeWriter::close() {
	fout.close();
}

// helper function for index translation
int getIndex(const string& value) {
	int val = -1;
	try {
		val = stoi(value); }
	catch (...) {
		return val;
	}
	return val;
}

// Full translation blocks
bool CodeWriter::push(const std::string& segment, const std::string& index, const std::string& scope) {
	int i = getIndex(index);
	if (i < 0) {
		cerr << "Invalid index/constant: " << index << endl;
		return false;
	}
	int address = 0;
	if (segment == "constant")
		pushConstant(i);
	else if (segment == "local")
		pushIndexed(i, "lcl");
	else if (segment == "temp") {
		if (i > 7) {
			cerr << "TEMP index " << i << " is too large (max 7)" << endl;
			return false;
		}
		pushAddress(TEMP(i));
	}
	else if (segment == "this")
		pushIndexed(i, "this");
	else if (segment == "that")
		pushIndexed(i, "that");
	else if (segment == "argument")
		pushIndexed(i, "arg");
	else if (segment == "static") {
		string staticLbl = scope + ".static" + index;
		fout << "\t@" << staticLbl << '\n'
		     << "\td=m"
			 << endl;
		pushD();
	}
	else if (segment == "pointer") {
		if (i > 1) {
			cerr << "POINTER index " << i << " is too large (max 1)" << endl;
			return false;
		}
		pushAddress(PTR(i));
	}
	else {
		fout << "// INVALID INSTRUCTION" << endl;
		return false;
	}
	return true;
}

bool CodeWriter::pop(const std::string& segment, const std::string& index, const std::string& scope) {
	int i = getIndex(index);
	if (i < 0) {
		cerr << "Invalid index/constant: " << index << endl;
		return false;
	}
	int address = 0;
	if (segment == "local")
		popIndexed(i, "lcl");
	else if (segment == "temp") {
		if (i > 7) {
			cerr << "TEMP index " << i << " is too large (max 7)" << endl;
			return false;
		}
		popAddress(TEMP(i));
	}
	else if (segment == "this")
		popIndexed(i, "this");
	else if (segment == "that")
		popIndexed(i, "that");
	else if (segment == "argument")
		popIndexed(i, "arg");
	else if (segment == "static") {
		string staticLbl = scope + ".static" + index;
		popD();
		fout << "\t@" << staticLbl << '\n'
		     << "\tm=d"
			 << endl;
	}
	else if (segment == "pointer") {
		if (i > 1) {
			cerr << "POINTER index " << i << " is too large (max 1)" << endl;
			return false;
		}
		popAddress(PTR(i));
	}
	else {
		fout << "// INVALID INSTRUCTION" << endl;
		return false;
	}
	//popAddress(address);
	return true;
}

bool CodeWriter::call(const std::string& function, const std::string& argc) {
	int n = getIndex(argc);
	if (n < 0) {
		cerr << "Invalid index/constant: " << argc << endl;
		return false;
	}
	
	// set the temp registers needed by the CALL routine
	loadDConst(n + 1);
	storeD(TEMP(0));	// ARG offset
	fout << "\t@" << function << '\n'
	     << "\td=a"
		 << endl;
	storeD(TEMP(2));	// function address
	fout << "\t@pc+6\n"
	     << "\td=a\n"
		 << "\t@" << TEMP(1) << '\n'
		 << "\tm=d\n"
		 << "\t@$_CALL_$\n"
		 << "\t0;jmp"
		 << endl;
	
	//	save D to LR before continuing
	storeD(LR);
	
	return true;
}

void CodeWriter::printCallRoutine() {
	// method:
	//	(args are already pushed on the stack in order of index)
	//	push ARG
	//	set ARG to (SP - argc - 1)
	//	push LCL
	//	push PTR(0)
	//	push PTR(1)
	//	push LR
	//	set LCL to SP
	//	set LR to (address after jump)
	//	unconditional jump to function
	//	clobber args - decrementStack(argc)
	
	//	THIS ROUTINE NEEDS THE FOLLOWING INPUTS:
	//	temp(0): contains argc + 1 (number of function arguments + 1)
	//	temp(1): return address - this should point to the storeD(LR) line in call()
	//	temp(2): address of function - load the label of called function here
	
	//	label for all "call" instructions to jump to (save hundreds of repeated lines!)
	fout << "($_CALL_$)" << endl;
	//	push ARG
	pushAddress(ARG);
	//	set ARG to (SP - TEMP(0))
	loadDMem(SP);
	fout << "\t@" << TEMP(0) << '\n'
	     << "\td=d-m"
		 << endl;
	storeD(ARG);
	//	push LCL
	pushAddress(LCL);
	//	push PTR(0)
	pushAddress(PTR(0));
	//	push PTR(1)
	pushAddress(PTR(1));
	//	push LR
	pushAddress(LR);
	//	set LCL to SP
	loadDMem(SP);
	storeD(LCL);
	//	set LR to temp(1) (address after jump)
	fout << "\t@" << TEMP(1) << '\n'
	     << "\td=m"
		 << endl;
	storeD(LR);
	//	unconditional jump to function (address in TEMP(2))
	fout << "\t@" << TEMP(2) << '\n'
	     << "\ta=m;jmp"
		 << endl;
		 
}

bool CodeWriter::function(const std::string& name, const std::string& localc) {
	// method:
	//	(ARG is setup already - func should know how many args it has)
	//	make label
	//	while (localc--)
	//		push 0
	int n = getIndex(localc);
	if (n < 0) {
		cerr << "Invalid index/constant: " << localc << endl;
		return false;
	}
	fout << '(' << name << ')' << endl;
	while (n--)
		pushFalse();
		
	return true;
}

void CodeWriter::ret() {
	// method:
	//	Just a stub to call the $_RET_$ code! Huge code size optimization :D
	fout << "\t@$_RET_$\n"
	     << "\t0;jmp"
		 << endl;
}

void CodeWriter::printRetRoutine() {
	// method:
	//	(return value has been pushed)
	//	generate label
	fout << "($_RET_$)" << endl;
	//	pop R14 (store it for later)
	popAddress(R(14));
	//	clobber local vars - set SP = LCL
	loadDMem(LCL);
	storeD(SP);
	//	(start popping things in reverse of call())
	//	pop LR to temp register
	popAddress(TEMP(7));
	//	pop PTR(1)
	popAddress(PTR(1));
	//	pop PTR(0)
	popAddress(PTR(0));
	//	pop LCL
	popAddress(LCL);
	//	pop R13 (old ARG)
	popAddress(R(13));
	//	set SP = ARG (clobber current args)
	loadDMem(ARG);
	storeD(SP);
	//	set ARG = R13
	loadDMem(R(13));
	storeD(ARG);
	//	push R14 (return value is now on top of stack)
	pushAddress(R(14));
	//	save temp register (calling function's LR)
	loadDMem(TEMP(7));
	//	finally, unconditional jump to LR
	fout << "\t@lr\n"
	     << "\ta=m\n"
	     << "\t0;jmp"
		 << endl;
}

void CodeWriter::label(const string& label, const string& scope) {
	string lbl = scope + "$" + label;
	fout << '(' << lbl << ')' << endl;
}

void CodeWriter::goto_(const string& label, const string& scope) {
	string lbl = scope + "$" + label;
	fout << "\t@" << lbl << '\n'
	     << "\t0;jmp"
		 << endl;
}

void CodeWriter::if_goto(const string& label, const string& scope) {
	string lbl = scope + "$" + label;
	popD();
	fout << "\t@" << lbl << '\n'
	     << "\td;jne"
		 << endl;
}

// push & pop subroutines
void CodeWriter::pushConstant(int value) {
	loadDConst(value);
	pushD();
}

void CodeWriter::pushAddress(int address) {
	loadDMem(address);
	pushD();
}

void CodeWriter::pushIndexed(int i, const char* label) {
	// get value out of indexed memory segment
	fout << "\t@" << i << '\n'
		 << "\td=a\n"
		 << "\t@" << label << '\n'
		 << "\ta=d+m\n"
		 << "\td=m"
		 << endl;
	// push it on the stack
	pushD();
}

void CodeWriter::popAddress(int address) {
	popD();
	storeD(address);
}

void CodeWriter::popIndexed(int i, const char* label) {
	// calc address to store popped value
	fout << "\t@" << i << '\n'
		 << "\td=a\n"
		 << "\t@" << label << '\n'
		 << "\td=d+m"
		 << endl;
	// hold address in r13 temporarily
	storeD(R(13));
	// pop the value
	popD();
	// finally, store it in the address
	fout << "\t@13\n"
	     << "\ta=m\n"
		 << "\tm=d"
		 << endl;
}

// Primitive instruction translation blocks
inline void CodeWriter::loadDMem(int addr) {
	fout << "\t@" << addr << '\n'
	     << "\td=m"
		 << endl;
}
inline void CodeWriter::loadDConst(int immed) {
	bool neg = (immed < 0);
	fout << "\t@" 
	     << (neg?-immed:immed) << '\n'
	     << (neg?"\td=-a":"\td=a")
		 << endl;
}
inline void CodeWriter::storeD(int addr) {
	fout << "\t@" << addr << '\n'
	     << "\tm=d"
		 << endl;
}
inline void CodeWriter::pushD() {
	fout << "\t@sp\n"
		 << "\tm=m+1\n"
	     << "\ta=m-1\n"
		 << "\tm=d"
		 << endl;
}
inline void CodeWriter::popD() {
	getPopAddr();
	fout << "\td=m" << endl;
}
inline void CodeWriter::getPopAddr() {
	fout << "\t@sp\n"
	     << "\tam=m-1"
		 << endl;
}
inline void CodeWriter::peekStack() {
	fout << "\t@sp\n"
		 << "\ta=m-1"
		 << endl;
}
inline void CodeWriter::pushTrue() {
	fout << "\td=-1" << endl;
	pushD();
}
inline void CodeWriter::pushFalse() {
	fout << "\td=0" << endl;
	pushD();
}

inline void CodeWriter::decStack(int n) {
	loadDConst(n);
	fout << "\t@sp\n"
	     << "\tm=m-d"
		 << endl;
}
