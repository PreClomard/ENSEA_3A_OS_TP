#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>


//Variables
int dataGlobalInit=10;
int dataGlobalNonInit;
char* str="Hello world";

//Methodes
/**
* Name: ShowAddr
* Usel: Show the different address of the segments memories
**/
void showAddr(){
	printf("Address of the Data : %p \n",&dataGlobalInit);  
        printf("Address of the BSS : %p \n",&dataGlobalNonInit);
        printf("Address of the Str : %p \n",str);  

 
 
}
int main(){
	showAddr();
exit(EXIT_SUCCESS);
}
