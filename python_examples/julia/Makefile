#export OPENGL=0
include ../../src/Makefile.defs

OPT+= -fPIC -DLIBREBOUND 

all: librebound librebounduser.so

SOURCES=migration_forces.c
OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(SOURCES:.c=.h)

%.o: %.c $(HEADERS)
	@echo "Compiling source file $< ..."
	$(CC) -c $(OPT) $(PREDEF) -o $@ $<

librebounduser.so: $(OBJECTS)
	@echo ""        
	@echo "Linking share library $@ ..."
	$(CC) $(OPT) -shared $(OBJECTS) $(LIB) -o $@ 

#rebound: librebound librebounduser.so problem.c
#	@echo ""
#	@echo "Compiling problem file ..."
#	$(CC) -I../../src/ -Wl,-rpath,./ $(OPT) $(PREDEF) problem.c -L. -lrebound -lrebounduser $(LIB) -o rebound
#	@echo ""
#	@echo "REBOUND compiled successfully."

librebound: 
	@echo "Compiling shared library librebound.so ..."
	#$(MAKE) -C ../../src/
	@-rm -f librebound.so
	@ln -s ../../src/librebound.so .

clean:
	@echo "Cleaning up shared library librebound.so ..."
	@-rm -f librebound.so
	#$(MAKE) -C ../../src/ clean
	@echo "Cleaning up local directory ..."
	@-rm -vf rebound $(OBJECTS)


