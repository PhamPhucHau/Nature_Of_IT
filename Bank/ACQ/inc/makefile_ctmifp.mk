#
# atmifp source make file
#

AP_HOME	 = /vbsdev/atm
RUN_HOME = /vbsdev/atm
LDIR	 = $(AP_HOME)/lib
INCDIR	 = $(AP_HOME)/inc
OBJDIR	 = $(AP_HOME)/lib

REP_HOME = /vbsrep/atm
REP_OBJDIR = $(REP_HOME)/lib
REP_INCDIR = $(REP_HOME)/inc

#----------------------------------------------------------------------------------
# oracle environment
#----------------------------------------------------------------------------------
ORACLE_BASE = /sw/oracle
"ORACLE_HOME = {ORACLE_BASE}/client/12.2.0.1
ORAINCDIR = {ORACLE_HOME}/precomp/public
ORALIBDIR = {ORACLE_HOME}/lib

PROC = {ORACLE_HOME}/bin/proc
#PROC = proc

PROCFLAGS = ireclen=1024 oreclen=1024 select_error=yes release_cursor=no 	    mode=ORACLE dbms=v8 unsafe_null=yes hold_cursor=yes lines=yes             ltype=no DYNAMIC=ANSI sqlcheck=full CODE=ANSI_C common_parser=yes             userid=atmdba/gtatmdba@DGBVNDB
#----------------------------------------------------------------------------------
# tmax environment
#----------------------------------------------------------------------------------
TMAXDIR = /tmax/tmaxeb
TMAXINCDIR = {TMAXDIR}/usrinc
TMAXLIBDIR = {TMAXDIR}/lib64


#include $(ORACLE_HOME)/precomp/lib/env_precomp.mk


CFLAGS	 = -w -I$(REP_INCDIR) -I$(ORAINCDIR) -I$(INCDIR)
LDFLAGS	 = $(LDIR)/libsysutil.a $(LDIR)/libdesutil.a $(LDIR)/IS_LIB.a -lnsl -lm -L$(ORALIBDIR) -lsyscomm -lclntsh

PCFLAGS = $(PROCFLAGS) 	include=$(INCDIR) 	include=$(REP_INCDIR) 	include=$(ORAINCDIR) 	include=$(TMAXINCDIR)

OBJS	 = 

EXE	 = acqifint
CC	 = cc
ECHO	 = echo
AR	 = X64


all : $(EXE)
	@$(ECHO)  
	@$(ECHO) "Construct Success !!!"
	@$(ECHO)

atmifp.c : atmifp.pc
	$(PROC) $(PCFLAGS) iname=atmifp.pc PARSE=NONE
atmifp.o : atmifp.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/atmifp.o atmifp.c
acqifint : atmifp.o
	$(CC) -o acqifint $(OBJDIR)/atmifp.o $(OBJDIR)/hsmManagement.o $(CFLAGS) $(LDFLAGS)
	mv $@ $(RUN_HOME)/bin/

#
# INSTALL
#
install: $(EXE)
	cp -p $(EXE) $(RUN_HOME)/bin/
#
# CLEAN
#
clean:
	rm -f $(EXE) core atmifp.c
