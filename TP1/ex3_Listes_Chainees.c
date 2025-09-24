#include <stdio.h>
#include <stdlib.h>

typedef struct pile{
	int value;
	struct pile *prec;
}pile;


/*
* Name: Push
* use : push the value in the pile
*/
void push(pile **p,int Val){
	pile *element = malloc(sizeof(pile));
        if(!element) exit(EXIT_FAILURE);
        element->value = Val;
        element->prec = *p;
        *p = element;    
}

int length(pile *p){
	int n=0;
	while(p){
		n++;
		p=p->prec;
	}
	return n;
}

void view(pile *p){
	while(p){
		printf("%d\n",p->value);
		p=p->prec;
	}
}


int main(){
	
	pile *MyList=NULL;
	int n=10;
	for(int i=0; i<n;i++){
		push(&MyList,i);
	}
	view(MyList);
}
