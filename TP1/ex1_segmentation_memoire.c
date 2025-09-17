#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>


//Global Variables
int dataGlobalInit=10;
int dataGlobalNonInit;
char* str="Hello world";

//Methodes
/**
* Name: ShowAddr
* Usel: Show the different address of the segments memories
**/
void showAddr();

int main(){	
	showAddr();
exit(EXIT_SUCCESS);
}

void showAddr(){
        pid_t pid = fork();
        if(pid>0){

                //Local variables
                int dataLocal=5;
                int *dynamic = malloc(sizeof(int));
                char valuePid[20];
		
		void *mmap_ptr = mmap(NULL, sysconf(_SC_PAGESIZE), 
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1,0);
                
		printf("Address of the Data : %p \n",&dataGlobalInit);
                printf("Address of the BSS : %p \n",&dataGlobalNonInit);
                printf("Address of the Str : %p \n",str);  
                printf("Address of the Heap : %p \n",dynamic);
                printf("Address of the Stack : %p \n",&dataLocal); 
                printf("Address of the Main Function : %p \n",main);
                printf("Address of the LibC Function : %p \n",printf);
		printf("Address of the memory mmap (Mmap) : %p\n",mmap_ptr);

                snprintf(valuePid, sizeof(valuePid), "%d",getpid());
        /*
        * Note perso: Eviter d'utiliser system préféré l'utilisation
        * d'excv, la raison est que system exécute qu'importe ce qui
        * est en son sein ce qui peut être source d'erreur.
        * code : system(cmd);
        */
                char *argv[] = {"pmap","-X",valuePid, NULL};
                execv("/usr/bin/pmap",argv);

        }
}
