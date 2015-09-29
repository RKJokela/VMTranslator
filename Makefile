RMDIR = rmdir /S /Q
MOVE  = move /Y

DIR_SRC = .
DIR_BIN = bin
DIR_LIB = lib
DIR_OBJ = obj
DIR_INSTALL = .
INC_PATH = include
TARGET = vmt.exe
TARGET_PATH = $(DIR_BIN)\$(TARGET)

CFLAGS = -c -nologo -EHsc -Zi -I$(INC_PATH)
# CFLAGS += -DDEBUG
LFLAGS = /debug /subsystem:CONSOLE /nologo /out:$(TARGET_PATH)

SRC_FILES = \
	CodeWriter.cpp \
	VMParser.cpp \
	main.cpp
	
OBJ_FILES = \
	$(DIR_OBJ)\CodeWriter.obj \
	$(DIR_OBJ)\VMParser.obj \
	$(DIR_OBJ)\main.obj
	
all : create_dirs $(TARGET_PATH)

{$(DIR_SRC)}.cpp{$(DIR_OBJ)}.obj ::
	@echo Compiling...
	cl $(CFLAGS) -Fo$(DIR_OBJ)\ $<

$(TARGET_PATH) : $(OBJ_FILES)
	@echo Linking $(TARGET)...
	link $(LFLAGS) $(SDL_LIBS) $(OBJ_FILES)

create_dirs :
	@if not exist $(DIR_BIN) mkdir $(DIR_BIN)
	@if not exist $(DIR_OBJ) mkdir $(DIR_OBJ)

clean :
	@if exist $(DIR_BIN) $(RMDIR) $(DIR_BIN)
	@if exist $(DIR_OBJ) $(RMDIR) $(DIR_OBJ)

install : all
	copy /B /Y $(TARGET_PATH) $(DIR_INSTALL)
	
rebuild : clean create_dirs $(TARGET_PATH)
