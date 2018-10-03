#include <stdio.h>
#include <stdlib.h>

/****************************************************************************/
/*                                                                          */
/* 			     Module CPU                                     */
/* 			External Declarations 				    */
/*                                                                          */
/****************************************************************************/


/* OSP constant      */

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

// cabeça e cauda da fila
typedef struct Fila{
	PCB * inicio;
	PCB * fim;
}Fila;

Fila *PtFila;


void cpu_init()
{
	PTFila = (Fila *)(malloc(sizeof(Fila)));
	if(PTFila == NULL){
		printf("Error na alocacao!\n");
		exit(5);
	}
	PTFila->fim = NULL;
	PTFila->inicio = NULL;
	PTFila->fim->next = PTFila->inicio;
	PTFila->inicio->prev = PTFila->fim;
}



void dispatch()
{

    if(PTBR != NULL && PTBR->pcb->status == running){
	insert_ready(PTBR->pcb);
	return;
    }
    if(PtFila == NULL){
        PTBR = NULL;
        return;
    }
       
    
	
}

void verificaNaFila(PCB * pcb){
    PCB *aux = PtFila->inicio;

    while(aux->next != PtFila->inicio){
        if(aux->pcb_id == pcb->pcb_id)
	   return 1;
    }

  return 0;
}

void insert_ready(pcb)
PCB *pcb;
{
    if(verificaNaFila(pcb))
      return;
    //atualiza o ponteiro anterior do novo nó p/ apontar p/ o antigo ultimo nó da fila
    pcb->prev = PtFila->fim;
    //atualiza o antigo ponteiro do fim da fila p/ apontar p/ o novo nó que está entrando
    PtFila->fim->next = pcb;
    //o próx ponteiro do novo nó apontará para o inicio da fila (garantindo a circularidade da fila)
    pcb->next = PtFila->inicio;
    //o pŕox ponteiro do inicio da fila passará a apontar para o novo nó que está entrando (garantindo a circularidade da fila)
    PtFila->inicio->next = pcb;

}
//PTBR É O CONTROLADOR QUE VERIFICA SE TEM ALGUM PROCESSO RODANDO OU NÃO
/* end of module */
