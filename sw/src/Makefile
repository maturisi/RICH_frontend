CC=g++
CFLAGS= -Wall 
LDFLAGS= -lconfig++  -L. -lmrc #-lrt 

SOURCES= 	TRich_ConnectTCP.cpp \
		TRich_Frontend.cpp \
		TRich_Config.cpp\
		TRich_DAQ.cpp\
		TRich_Analysis.cpp \
		TRich_Scaler.cpp \
		TRich_TDC.cpp\
		TRich_Adapter.cpp

OBJ=$(SOURCES:.cpp=.o)
LIB=libmrc.so

DAQ = daq
ANA = ana

PATH_ROOTLIB = $(shell root-config --libdir)
PATH_ROOTINC = $(shell root-config --incdir)

ROOTINC= -I $(PATH_ROOTINC) 
ROOTLIB= -L$(PATH_ROOTLIB) -lCore -lHist -lTree -lRIO -lGpad

ARCHIVIO 	= rich_sw.0.150709.tar

MAIN_DAQ = daq.cpp
MAIN_ANA = analysis.cpp

all : $(DAQ) $(ANA) $(SOURCES) 

$(DAQ):$(MAIN_DAQ) $(LIB)
	$(CC) -o $(DAQ) $(MAIN_DAQ) $(LDFLAGS) $(ROOTINC) $(ROOTLIB)
	cp $(DAQ) ../bin
$(ANA):	$(MAIN_ANA) $(LIB)
	$(CC) -o $(ANA) $(MAIN_ANA) $(LDFLAGS) $(ROOTINC) $(ROOTLIB)
	cp $(ANA) ../bin
$(LIB):	$(OBJ)
	ar cr $(LIB) $(OBJ)
.cpp.o: $(SOURCES)
	$(CC) -o $@ $< -c $(CFLAGS) $(ROOTINC)

clean:
	rm -f $(DAQ) $(ANA) $(OBJ) $(LIB) *~ *.log *.o *.so ../bin/ana ../bin/daq
	

#
# Archivio di backup
#
bk	: gzip
	mv $(ARCHIVIO) ..
 
gzip	: 
	tar cvfz $(ARCHIVIO) $(MAIN) \
	TRich_Frontend.cpp TRich_Frontend.h \
	TRich_DAQ.cpp TRich_DAQ.h \
	TRich_ConnectTCP.cpp TRich_ConnectTCP.h \
	TRich_Config.cpp TRich_Config.h \
	TRich_Analysis.cpp TRich_Analysis.h \
	TRich_Scaler.cpp TRich_Scaler.h \
	TRich_TDC.cpp TRich_TDC.h \
	TRich_Adapter.cpp TRich_Adapter.h \
	runme.sh \
	rich.cfg \
	datatype.h \
	Makefile\

# to do:


#--------

#ROOT_CFLAGS = -I/Users/matteo/MAT/root/root/include/
#ROOTLIB= $(shell root-config --libs)
#ROOT_LDFLAGS=  -L/Users/matteo/MAT/root/root/lib -lCore -lHist -lTree -lRIO -lGpad
# PC Lab Ferrara 
#ROOT_CFLAGS = -I/home/matteo/ROOT/root/include
#ROOT_LDFLAGS=  -L/home/matteo/ROOT/root/lib -lCore -lCint -lRIO -lNet -lGpad -lTree -lMathCore -lThread -lHist -lGraf -lMatrix
