install: 
	mkdir -p /usr/local/include
	mkdir -p /usr/local/lib
	cp include/stutils.h /usr/local/include/
	cp build/libmylib.a /usr/local/lib/
	ldconfig //this wil update the loader config to see the new libraries

remove:
	rm /usr/local/include/stutils.h
	rm /usr/local/lib/libmylib.a 
	ldconfig


buildMac:
	gcc -c src/stlength.c src/streverse.c
	ar rcs libstutilsMac.a stlength.o streverse.o
	mv libstutilsMac.a build/
	rm stlength.o streverse.o 


installMac:
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include
	cp include/stutils.h /usr/local/include/
	cp build/libstutilsMac.a /usr/local/lib/

uninstallMac:
	rm /usr/local/include/stutils.h 
	rm /usr/local/lib/libstutilsMac.a
	