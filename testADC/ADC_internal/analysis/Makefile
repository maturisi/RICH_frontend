CC=g++
LDFLAGS= #-Wall #-lconfig++ -L/usr/local/lib -L. 
EXE = adctest


#ROOT_CFLAGS = -I/Users/matteo/MAT/root/root/include/
#ROOT_LDFLAGS= -L/Users/matteo/MAT/root/root/lib -lCore -lRIO -lTree -lGpad -lHist -lSpectrum -lMathCore -lGraf


ROOT_CFLAGS=$(shell root-config --cflags)
ROOT_LDFLAGS=$(shell root-config --libs)  
ROOT_GLIBS=$(shell root-config --glibs)

 
MAIN=main.cpp 

$(EXE): $(MAIN) 
	$(CC) -o $(EXE)  $(MAIN) $(LDFLAGS) $(ROOT_CFLAGS) $(ROOT_LDFLAGS) $(ROOT_GLIBS)
 
clean: 
	rm -f $(EXE) *~ *.log *.o
#eof
