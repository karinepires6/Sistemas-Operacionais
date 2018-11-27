// Karine Pires de Araújo OSP - 3

/****************************************************************************/
/*                                                                          */
/* 			     Module CPU                                     */
/* 			External Declarations 				    */
/*                                                                          */
/****************************************************************************/


/* OSP constant      */
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define   MAX_PAGE                   16 /* max size of page tables          */

/* OSP enumeration constants */

typedef enum {
    false, true                         /* the boolean data type            */
} BOOL;

typedef enum {
    running, ready, waiting, done       /* types of status                  */
} STATUS;



/* external type definitions */

typedef struct page_entry_node PAGE_ENTRY;
typedef struct page_tbl_node PAGE_TBL;
typedef struct event_node EVENT;
typedef struct pcb_node PCB;



/* external data structures */

extern struct page_entry_node {
    int    frame_id;    /* frame id holding this page                       */
    BOOL   valid;       /* page in main memory : valid = true; not : false  */
    int    *hook;       /* can hook up anything here                        */
};

extern struct page_tbl_node {
    PCB    *pcb;        /* PCB of the process in question                   */
    PAGE_ENTRY page_entry[MAX_PAGE];
    int    *hook;       /* can hook up anything here                        */
};

extern struct pcb_node {
    int    pcb_id;         /* PCB id                                        */
    int    size;           /* process size in bytes; assigned by SIMCORE    */
    int    creation_time;  /* assigned by SIMCORE                           */
    int    last_dispatch;  /* last time the process was dispatched          */
    int    last_cpuburst;  /* length of the previous cpu burst              */
    int    accumulated_cpu;/* accumulated CPU time                          */
    PAGE_TBL *page_tbl;    /* page table associated with the PCB            */
    STATUS status;         /* status of process                             */
    EVENT  *event;         /* event upon which process may be suspended     */
    int    priority;       /* user-defined priority; used for scheduling    */
    PCB    *next;          /* next pcb in whatever queue                    */
    PCB    *prev;          /* previous pcb in whatever queue                */
    int    *hook;          /* can hook up anything here                     */
};


/* external variables */

extern PAGE_TBL *PTBR;		/* page table base register */

extern int    Quantum;		/* global time quantum; contains the value
				   entered at the beginning or changed 
				   at snapshot. Has no effect on timer
				   interrupts, unless passed to set_timer() */



/* external routines */

extern prepage(/* pcb */);
extern int start_cost(/* pcb */);
/*  PCB    *pcb; */    

extern set_timer(/* time_quantum */);
/*  int    time_quantum; */

extern int get_clock();





/****************************************************************************/
/*                                                                          */
/*				Module CPU				    */
/*			     Internal Routines				    */
/*                                                                          */
/****************************************************************************/

#define MAX 10 //define tamanho da fila

typedef struct Fila{
	PCB * inicio;
	PCB * fim;
	int tam;
	int quantum;
}Fila;

Fila fila[MAX];

//executa apenas uma unica vez
void cpu_init()
{
	int i, cont = 5;

	for(i=0;i<MAX;i++){
		fila[i].fim = NULL;
		fila[i].inicio = NULL;
		fila[i].tam = 0;
		fila[i].quantum = cont;
		cont += 5;
	}
	
}

int verificaNaFila(PCB * pcb, int id){
	PCB *aux = fila[id].inicio;
	
	int j = 0;
	while(j < fila[id].tam){
		if(aux->pcb_id == pcb->pcb_id)
			return 1;
		j++;
		aux = aux->next;		
	}
	return 0;
}

//RECEBE UM PROCESSO COMO PARAMETRO PARA COLOCAR NO FIM DA FILA
void insert_ready(pcb)
PCB *pcb;
{

	//verifica se o processo jah foi escalado
	if(pcb->last_cpuburst < 0){
		pcb->priority = 0;
	} else {
		if(pcb->last_cpuburst > fila[pcb->priority].quantum - 3){
			if(pcb->priority + 1 < 10){
				pcb->priority++;			
			} else {
				pcb->priority = 9;			
			}
		} else if((fila[pcb->priority].quantum * 0.6) > pcb->last_cpuburst) {
			if( pcb->priority == 0){
				pcb->priority = 0;			
			}
			else {
				pcb->priority--;			
			}		
		}

		if(verificaNaFila(pcb, pcb->priority)){
			return;
		}
	}
	
	pcb->status = ready;
		
	int id = pcb->priority;
	if(fila[id].tam == 0){ // se a fila estiver vazia faz o nó ser o primeiro, onde o inicio e fim aponta para o mesmo no
		pcb->next = pcb; // o proximo do fim é ele mesmo
		pcb->prev = pcb; // o anterior do fim é ele mesmo
		
		fila[id].inicio = pcb; // faz o inicio apontar para o novo nó
		fila[id].fim = pcb; // faz o fim apontar para o novo nó
		fila[id].tam = 1;
	} else { // se a fila tiver pelo menos um elemento então vou fazer apenas o novo nó apontar para o inicio e o apontar para o penultimo
		pcb->next = fila[id].inicio; // faz o proximo do novo nó apontar para o inicio
		pcb->prev = fila[id].fim; // faz o anterior do novo nó apontar para o fim
		pcb->next->prev = pcb; // faz inicio apontar para o novo fim
		pcb->prev->next = pcb;  // faz o antigo fim apontar para o novo nó
		
		fila[id].fim = pcb; // faz o novo nó se tornar o fim
		fila[id].tam++; // aumenta o tamanho da fila
	}

	int k;
	for(k = 0 ; k < MAX ; k++){
		PCB *aux = fila[k].inicio;
		
		int l = 0;
		while(l < fila[k].tam){
			fprintf(stderr, "\nFila: %i\tId processo: %i\tlast_cpuburst: %i\n", k, aux->pcb_id, aux->last_cpuburst);
			l++;
			aux = aux->next;
		}
	}

}

//pega o primeiro processo da fila principal/auxiliar e coloca para a execução
void dispatch()
{
	/* verifica se há algum processo em execução, se sim, coloca ele na fila de pronto */
	if(PTBR != NULL && PTBR->pcb->status == running)
		insert_ready(PTBR->pcb);
	
	PCB *pcb;

	int i;

	for(i = 0; i < MAX ; i++){
		if(fila[i].tam == 0){
			continue;		
		}

		pcb = fila[i].inicio;
		if(fila[i].tam == 1){
			fila[i].fim = NULL;
			fila[i].inicio = NULL;
			fila[i].tam = 0;		
		} else {
			pcb->prev->next = pcb->next; // faz o ultimo elemento apontar para o proximo do inicio (antigo segundo da fila)
			pcb->next->prev = pcb->prev; // faz proximo do inicio apontar para o fim
			fila[i].inicio = pcb->next; // faz o inicio da fila ser o proximo do inicio
			fila[i].tam--;
		}
		//imprime a qtd de vezes que o processo executou
		fprintf(stderr, "id: %d\tlast_cpuburst: %i\tFila que foi escalado: %i\tQuantum: %i\n", pcb->pcb_id, pcb->last_cpuburst , i, fila[i].quantum);

		// atualiza o PTBR para a tabela de pagina do processo
		PTBR = pcb->page_tbl;
		//ATUALIZA O STATUS DO PROCESSO
		pcb->status = running;
		//CHAMA A FUNCAO PREPAGE()
		prepage(pcb);
		//ATUALIZA A ULTIMA VEZ QUE O PROCESSO FOI DESPACHADO
		pcb->last_dispatch = get_clock();
		//SETA O QUANTUM DO PROCESSO
		set_timer(fila[i].quantum);
		break;
	}
	
	if(i == 10){
		PTBR = NULL;
		return;
	}
	
}

/* end of module */
