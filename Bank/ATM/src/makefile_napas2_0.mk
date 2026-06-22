#
# atmif source make file
#

AP_HOME	 = /vbsdev/atm
RUN_HOME = /vbsdev/atm
LDIR	 = $(AP_HOME)/lib
INCDIR	 = $(AP_HOME)/inc
OBJDIR	 = $(AP_HOME)/lib

REP_HOME = /vbsrep/atm
REP_OBJDIR = $(AP_HOME)/lib
REP_INCDIR = $(AP_HOME)/inc

#----------------------------------------------------------------------------------
# oracle environment
#----------------------------------------------------------------------------------
ORACLE_BASE = /sw/oracle
ORACLE_HOME = {ORACLE_BASE}/client/12.2.0.1
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


# include $(ORACLE_HOME)/precomp/lib/env_precomp.mk


CFLAGS	 = -w -I$(REP_INCDIR) -I$(ORAINCDIR)
LDFLAGS	 = $(LDIR)/libsysutil.a $(LDIR)/libdesutil.a $(LDIR)/IS_LIB.a -lnsl -lm -L$(ORALIBDIR) -lsyscomm -lclntsh

PCFLAGS = $(PROCFLAGS) 	include=$(INCDIR) 	include=$(REP_INCDIR) 	include=$(ORAINCDIR) 	include=$(TMAXINCDIR)        

OBJS	 = 

EXE	 = nlogging.o isotrdef.o hsmManagement.o OasistoIso8583.o Iso8583toOasis.o  anyifsxx vstifsxx smartifoxx smartifi 
CC	 = cc -g -rdynamic -O0
ECHO	 = echo   
AR	 = X64           

all : $(EXE)         
	@$(ECHO)     
	@$(ECHO) "Construct Success !!!"
	@$(ECHO)    
       
nlogging.o : nlogging.c 
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/nlogging.o nlogging.c   

isotrdef.o : isotrdef.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/isotrdef.o isotrdef.c

hsmManagement.o : hsmManagement.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/hsmManagement.o hsmManagement.c
          
OasistoIso8583.c : OasistoIso8583.pc
	$(PROC) $(PCFLAGS) iname=OasistoIso8583.pc PARSE=NONE
OasistoIso8583.o : OasistoIso8583.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/OasistoIso8583.o OasistoIso8583.c

Iso8583toOasis.c : Iso8583toOasis.pc    
	$(PROC) $(PCFLAGS) iname=Iso8583toOasis.pc PARSE=NONE
Iso8583toOasis.o : Iso8583toOasis.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/Iso8583toOasis.o Iso8583toOasis.c

anyifsxx.c : anyifsxx.pc
	$(PROC) $(PCFLAGS) iname=anyifsxx.pc PARSE=NONE   
anyifsxx.o : anyifsxx.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/anyifsxx.o anyifsxx.c
anyifsxx : anyifsxx.o
	$(CC) -o anyifsxx anyifsxx.c $(OBJDIR)/nlogging.o $(CFLAGS) $(LDFLAGS)
	mv $@ $(RUN_HOME)/bin/

hstifsxx.c : hstifsxx.pc
	$(PROC) $(PCFLAGS) iname=hstifsxx.pc  PARSE=NONE          
hstifsxx.o : hstifsxx.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/hstifsxx.o hstifsxx.c
vstifsxx : hstifsxx.o
	$(CC) -o vstifsxx hstifsxx.c $(OBJDIR)/nlogging.o $(CFLAGS) $(LDFLAGS)
	mv $@ $(RUN_HOME)/bin/

smartifoxx.c : smartifoxx.pc
	$(PROC) $(PCFLAGS) iname=smartifoxx.pc PARSE=NONE
smartifoxx.o : smartifoxx.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/smartifoxx.o smartifoxx.c
smartifoxx : smartifoxx.o
	$(CC) -o smartifoxx $(OBJDIR)/smartifoxx.o $(REP_OBJDIR)/fifo.o $(OBJDIR)/OasistoIso8583.o $(OBJDIR)/Iso8583toOasis.o $(OBJDIR)/isotrdef.o $(OBJDIR)/hsmManagement.o $(OBJDIR)/nlogging.o  $(CFLAGS) $(LDFLAGS)
	mv $@ $(RUN_HOME)/bin/

smartifi.c : smartifi.pc
	$(PROC) $(PCFLAGS) iname=smartifi.pc PARSE=NONE
smartifi.o : smartifi.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/smartifi.o smartifi.c
smartifi : smartifi.o
	$(CC) -o smartifi $(OBJDIR)/smartifi.o $(REP_OBJDIR)/fifo.o $(OBJDIR)/OasistoIso8583.o $(OBJDIR)/Iso8583toOasis.o $(OBJDIR)/isotrdef.o $(OBJDIR)/hsmManagement.o $(OBJDIR)/nlogging.o  $(CFLAGS) $(LDFLAGS)
	mv $@ $(RUN_HOME)/bin/

# INSTALL
#   
install: $(EXE)
	cp -p $(EXE) $(RUN_HOME)/bin/
#
# CLEAN
#
clean:
	rm -f $(EXE) core atmifd.c atmifsxx.c hstifint.c hstifsxx.c anyifint.c anyifsxx.c smartifd.c smartifd_INT.c OasistoIso8583.c OasistoIso8583_INT.c checkPrc.c dpcheck.c Iso8583toOasis.c
