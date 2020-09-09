JCCFOLDER  = ../../jcc
OBJSCAN    = $(JCCFOLDER)/objscan
PRELINK    = $(JCCFOLDER)/prelink
#CC         = $(JCCFOLDER)/jcc
CC         = wine $(JCCFOLDER)/jcc.exe
JCCINCS    = $(JCCFOLDER)/include
JCCOBJS    = $(JCCFOLDER)/objs
JCCLIBLIST = $(JCCOBJS)/liblst.txt


SRC_DIRS ?= ../src ../printf ../lstring ../rac ../dynit ../fss
INC_DIRS ?= $(JCCINCS) ../inc ../printf ../rac ../dynit ../fss ../cross

VPATH = $(SRC_DIRS)

SRCS := $(shell find $(SRC_DIRS) -name *.c )
INCS := $(shell find $(INC_DIRS) -type d)
OBJS := $(addsuffix .obj, $(notdir $(basename $(SRCS))))

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

D_FLAGS  ?= -D__MVS__
CC_FLAGS ?= $(INC_FLAGS) $(D_FLAGS) -o -fstk -list=list.out

JCC ?= $(CC) $(CC_FLAGS)
RM  := rm

all: $(TARGET)

$(TARGET): link

link: assemble
        @echo '# tbd - calling the mvs link job'

assemble: transfer
        @echo "# tbd - calling the mvs assemble job"

transfer: local_prelink
        @echo "# tbd - sending brexx.objp to the MVS system"

local_prelink: $(OBJS)
        @echo "# prelining JCC objects into brexx.objp"
        @$(PRELINK) -s $(JCCOBJS) brexx.objp  $(OBJS)

%.obj: %.c
        @echo "# compiling " $^
        $(JCC) $<
        @echo "# done"

.PHONY: clean

clean:
        @$(RM) $(OBJS)
