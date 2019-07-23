#include <stdio.h>
#include "SystemDefsAndFuncs.h"

//return 1 if Big-Endian
//return 0 if Little-Endian
char SysGetEndian() {
	int a = 1;
	char * aPtr = (char *)&a;
	//Big-Endian: 0 0 0 1 : 0 1 2 3
	//Little-Endian: 1 0 0 0 : 0 1 2 3
	//fprintf(stdout, "\n%d %d %d %d", aPtr[0], aPtr[1], aPtr[2], aPtr[3]);
	return aPtr[sizeof(int) - 1];	
}