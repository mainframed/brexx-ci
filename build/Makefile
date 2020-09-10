TARGET      := brexx.objp

JCCFOLDER   := ../../jcc
OBJSCAN     := $(JCCFOLDER)/objscan
PRELINK     := $(JCCFOLDER)/prelink
#CC          := $(JCCFOLDER)/jcc
CC          := wine $(JCCFOLDER)/jcc.exe
JCCINCS     := $(JCCFOLDER)/include
JCCOBJS     := $(JCCFOLDER)/objs
JCCLIBLIST  := $(JCCOBJS)/liblst.txt
RDRPREP     := rdrprep
TK4HOST     := localhost
TK4PORT     := 3505
TK4USER     := HERC01
TK4PASS     := CUL8TR
TK4PRINTER  := 
NC          := nc
SLEEP       := 20
# by default it will send to MSGCLASS A or H, set
# CUSTOMCLASS below if you want to use a different class
CUSTOMCLASS := 

########################################

SRC_DIRS := ../src ../lstring ../rac ../dynit ../fss
INC_DIRS := $(JCCINCS) ../inc ../rac ../dynit ../fss ../cross
ASM_DIRS := ../mvs/asm
MAC_DIRS := ../mvs/maclib
VPATH = $(SRC_DIRS)

SRCS := $(shell find $(SRC_DIRS) -name *.c )
INCS := $(shell find $(INC_DIRS) -type d)
OBJS := $(addsuffix .obj, $(notdir $(basename $(SRCS))))

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
D_FLAGS   := -D__MVS__
CC_FLAGS  := $(INC_FLAGS) $(D_FLAGS) -o -fstk -list=list.out

JCC := $(CC) $(CC_FLAGS)
RM  := rm

# Set the MSGCLASS to A if we have a local printer
MSGCLASS = H
ifneq ("$(wildcard $(TK4PRINTER))","")
MSGCLASS = A
endif

ifeq ($(CUSTOMCLASS),)
CUSTOMCLASS = $(MSGCLASS)
endif

.PHONY: clean all

all: $(TARGET)

$(TARGET): link

link: prelink
	@echo "# Generating link job using  USER=$(TK4USER) PASS=$(TK4PASS) MSGCLASS=$(CUSTOMCLASS)"
	@./link.sh brexx.objp $(TK4USER) $(TK4PASS) $(CUSTOMCLASS) > link.jcl
	@echo "# Converting to EBCDIC"
	@$(RDRPREP) link.jcl
	@echo "# Submiting  link job to reader at $(TK4HOST):$(TK4PORT)"
	@$(NC) -w1 $(TK4HOST) $(TK4PORT) < reader.jcl
# tbd : all these grep stuff
	
assemble: prelink
	@echo "# Generating JCL from $(ASM_DIRS) $(MAC_DIRS) MSGCLASS=$(CUSTOMCLASS)"
	@./assemble-link.sh $(ASM_DIRS) $(MAC_DIRS) brexx.objp $(CUSTOMCLASS) > assemble-link.jcl
	@echo "# Converting to EBCDIC"
	@$(RDRPREP) assemble-link.jcl
	@echo "# Submiting to reader at $(TK4HOST):$(TK4PORT)"
	@$(NC) -w1 $(TK4HOST) $(TK4PORT) < reader.jcl
ifeq ($(CUSTOMCLASS), A) 
	@sleep $(SLEEP)
	@echo "# Printing results $(TK4HOST):$(TK4PORT)"
	@grep "BRUPASLN   BRDELETE" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRCREATE" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRUPASM" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRUPMACL" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRXSVC" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   IRXEXCOM" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RACAUTH" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RACCHECK" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RACLOGIN" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXABEND" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXCPUTIM" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXIKJ441" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXINIT" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXNJE38" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXPTIME" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXSTIME" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXSVC" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXTERM" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXTSO" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXVSAM" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXVSMIO1" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXVSMIO2" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXWAIT" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   RXWTO" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   VSMIOSUB" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BROBJSCN" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   COPYOBJS" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   COPYINDX" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRLNAUTH" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   BRLINK" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   LINKAUTH" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   LINK" $(TK4PRINTER) |tail -1
	@grep "BRUPASLN   ALIASES" $(TK4PRINTER) |tail -1
else
	@echo "# Job submitted, check output spool on $(TK4HOST) for results"
endif

prelink: $(OBJS)
	@echo "# prelinking JCC objects into brexx.objp"
	@$(PRELINK) -s $(JCCOBJS) brexx.objp $(OBJS) obj/rxmvsext.obj >> prelink.log 2>&1
	

%.obj: %.c
	@echo "# compiling " $<
	@$(JCC) $< >> jcc.log 2>&1

clean:
	@./assemble-link.sh clean $(CUSTOMCLASS) > clean.jcl
	@$(RDRPREP) clean.jcl
	@$(NC) -w1 $(TK4HOST) $(TK4PORT) < reader.jcl
#	@sleep 5
#	@grep "BRCLEAN1   BRDELETE" $(TK4PRINTER) |tail -1 
#	@grep "BRCLEAN1   BRDLINKL" $(TK4PRINTER) |tail -1 
	@$(RM) -f $(OBJS) brexx.objp jcc.log list.out prelink.log *.jcl


