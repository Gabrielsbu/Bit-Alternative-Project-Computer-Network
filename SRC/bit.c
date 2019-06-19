
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* ************************************************ ******************
 ALTERNANDO BIT E EMULADOR DE REDE GO-BACK-N:
     VERSÃO 1.1 J.F.Kurose
     Revisado 1.2 D.R.Figueiredo
   Este código deve ser usado para PA2, transferência de dados unidirecional
   protocolos (de A para B). Propriedades de rede:
   - atrasos de rede unidirecionais calculam a média de alguns números de unidades de tempo
     são outras mensagens no canal para GBN), mas podem ser maiores
   - pacotes podem ser corrompidos (o cabeçalho ou a porção de dados)
     ou perdido, de acordo com as probabilidades definidas pelo usuário
   - os pacotes serão entregues na ordem em que foram enviados
     (embora alguns possam ser perdidos).
************************************************** ********************/


/* a "msg" é a unidade de dados passada da camada 5 (código dos professores) para a camada * /
/ * 4 (código dos alunos). Ele contém os dados (caracteres) a serem entregues * /
/ * para a camada 5 (usando a estrutura da mensagem) através do nível de transporte de alunos * /
/ * entidades de protocolo. */
struct msg {
  char data[20];
  };


/* um pacote é a unidade de dados passada da camada 4 (código dos alunos) para a camada * /
/ * 3 (código dos professores). Observe a estrutura de pacotes pré-definida, que todos * /
/ * os alunos devem seguir. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
    };

/* Protótipos de rotinas para estudantes */
void starttimer(int, float);
void stoptimer(int);
void tolayer3(int,struct pkt);
void tolayer5(int, struct msg);
int check_checksum(struct pkt);
int generate_checksum(struct pkt);
int flip_number(int);
/********* ALUNOS ESCREVER AS PRÓXIMAS SEIS ROTINAS *********/

#define A 0
#define B 1
#define TIMER 20
#define MESSAGE_SIZE 20

int A_STATE = 0;
int B_STATE = 0;

int count = 0;

int ACK = 0;
int SEQ = 0;
int prev_sequence_number;
struct msg prev_message;
struct pkt prev_packet;
struct pkt B_prev_packet;

/* Usado para verificar o checksum */
int check_checksum(struct pkt package){
    int i;
    int sum = (package.seqnum + package.acknum);
    for(i = 0; i < MESSAGE_SIZE; i++){
        sum += (int)package.payload[i];
    }
    return (sum == package.checksum);
}
/* usado para gerar o checksum */
int generate_checksum(struct pkt package){
    int i;
    int sum = (package.seqnum + package.acknum);
    for(i = 0; i < MESSAGE_SIZE; i++){
        sum += (int)package.payload[i];
    }
    return sum;            
}
/* usado para alternar o bit, entre 0 e 1 de acordo com o pacote*/
int flip_number(int number){
    if(number == 0) return 1;
    else return 0;
}

/* chamado da camada 5, passou os dados para serem enviados para o outro lado. Devolva um 1 se
os dados são aceitos para transmissão, número negativo de outra forma */
int A_output(message)
  struct msg message;
{

    if(A_STATE){
        printf("A está atualmente aguardando um ACK: %d\n",ACK);

        return -1;
    }else{
        //Construindo o pacote
        struct pkt packet;
        strcpy(packet.payload,message.data);
        packet.seqnum = SEQ;
        packet.acknum = flip_number(ACK);
        packet.checksum = generate_checksum(packet);
    
        prev_packet = packet;
        
        A_STATE = flip_number(A_STATE);
        SEQ = flip_number(SEQ);
        printf("|---------------------------------|\n");
        printf("|Informações do pacote            |\n");
        printf("|A irá transmitir um pacote para B|\n");
        printf("|Num sequencia: %d                 |\n", packet.seqnum);
        printf("|Checksum: %d                   |\n", packet.checksum);
        printf("|Dado: %s       |\n", packet.payload);
        printf("|O Ack esperado é %d               | \n",ACK);
        printf("|---------------------------------|\n\n");

        tolayer3(A, packet); 
        starttimer(A,TIMER);
        
        return 1;
    }
}

/*chamado da camada 3, quando um pacote chega para a camada 4 */
void A_input(packet)
  struct pkt packet;
{
    printf("|-----------------------------|\n");
    printf("|A recebeu o pacote (ACK = %d) |\n",packet.acknum);
    printf("|O Ack atual é: %d             |\n",ACK);
    printf("|-----------------------------|\n\n");
    if(packet.acknum == ACK && check_checksum(packet)){
        stoptimer(A);
        
        struct msg message;
        strcpy(message.data,packet.payload);
        
        ACK = flip_number(ACK);
        A_STATE = flip_number(A_STATE);
        tolayer5(A,message);
    }
}

/* chamado quando o temporizador de A se apaga */
void A_timerinterrupt()
{
    printf("\nUm temporizador foi interrompido, A irá reenviar o pacote. \n");
    
    tolayer3(A,prev_packet);
    starttimer(A,TIMER);
}  

/* a seguinte rotina será chamada uma vez (somente) antes de qualquer outro * /
/ * entidade rotinas são chamadas. Você pode usá-lo para fazer qualquer inicialização */
void A_init()
{
    A_STATE = 0;
    ACK = 0;
    SEQ = 0;
}



/* chamado da camada 3, quando um pacote chega para a camada 4 em B*/
void B_input(packet)
  struct pkt packet;
{
    printf("|--------------------------------------------|\n");
    printf("|B recebendo o pacote de A                   |\n");
    printf("|Informções do pacote                        |\n");
    printf("|Num sequência: %d                            |\n",packet.seqnum);
    printf("|Checksum: %d                              |\n",packet.checksum);
    printf("|Dado: %s                  |\n",packet.payload);
    printf("|O ACK que B está esperando para enviar é %d  |\n",ACK);
    printf("|--------------------------------------------|\n\n");


        if(check_checksum(packet) && packet.seqnum == B_STATE){
            struct msg message;
            strcpy(message.data,packet.payload);
            
            B_STATE = flip_number(B_STATE);

            struct pkt ack_packet;
            ack_packet.acknum = packet.seqnum;
            ack_packet.checksum = generate_checksum(ack_packet);

            B_prev_packet = ack_packet;

            printf("|-----------------------------------|\n");
            printf("|B está enviando o (Ack = %d) para A |\n", ACK);
            printf("|-----------------------------------|\n\n");

            count++;
            tolayer5(B,message);
            tolayer3(B, ack_packet);
            stoptimer(B);
            starttimer(B,TIMER);
        }else{printf("\nB NÃO está aceitando este pacote!\n B aceitou o %d letra do alfabeto e está esperando o próximo\n",count);
        }

}

/* chamado quando o temporizador de B se apaga */
void B_timerinterrupt()
{
    printf("\nB interrupção do temporizador, B agora está reenviando o pacote.\n");
    tolayer3(B,B_prev_packet);
    starttimer(B,TIMER);
}


/* o seguinte rotytine será chamado uma vez (somente) antes de qualquer outro * /
/ * as rotinas da entidade B são chamadas. Você pode usá-lo para fazer qualquer inicialização */
void B_init()
{
    B_STATE = 0;
}


/************************************************* ***************
***************** O CÓDIGO DE EMULAÇÃO DE REDE SE INICIA ABAIXO ***********
O código abaixo emula a camada 3 e abaixo do ambiente de rede:
  - emula a transmissão e a entrega (possivelmente com corrupção em nível de bit
    e perda de pacotes) de pacotes através da camada de interface 3/4
  - lida com o início / parada de um temporizador e gera temporizador
    interrupções (resultando na chamada de manipulador de temporizador de alunos).
  - gera mensagem para ser enviada (passada de 5 para 4)
NÃO HÁ RAZÃO QUE QUALQUER ALUNO DEVERIA LER OU ENTENDER
O CÓDIGO ABAIXO. VOCÊ NÃO TOCAR OU TOCAR (em seu código) QUALQUER
DAS ESTRUTURAS DE DADOS ABAIXO. Se você está interessado em como eu projetei
o emulador, você está convidado a olhar para o código - mas, novamente, você deve ter
para, e você definitivamente não deveria ter que modificar nada.
************************************************** ****************/
struct event {
   float evtime;           /* Tempo evento */
   int evtype;             /* Tipo evento */
   int eventity;           /* entidade onde o evento ocorre */
   struct pkt *pktptr;     /* ptr para pacote (se houver) assoc com este evento */
   struct event *prev;
   struct event *next;
 };
struct event *evlist = NULL;   /* lista de evento */

/* usar para transferência bidirecional de dados */
#define BIDIRECTIONAL 0 

/* eventos possiveis: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define   A    0
#define   B    1

int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from layer 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float simul_time = 0.000;        /* global simulation simul_time */
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */   
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int   ncorrupt;            /* number corrupted by media*/
int   randseed;            /* random number seed */

/****************** ROTINA DA LISTA DE EVENTOS ************* /
/ * Rotinas de manipulação de lista de eventos * /
/ ************************************************* ***/

void insertevent(p)
   struct event *p;
{
   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: Tempo: %lf\n",simul_time);
      printf("            INSERTEVENT: Tempo futuro é: %lf\n",p->evtime); 
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q; 
        if (q==NULL) {   /* final da lista */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q==evlist) { /* cabeça da lista */
             p->next=evlist;
             p->prev=NULL;
             p->next->prev=p;
             evlist = p;
             }
           else {     /* meio da lista */
             p->next=q;
             p->prev=q->prev;
             q->prev->next=p;
             q->prev=p;
             }
         }
}

void printevlist()
{
  struct event *q;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(AorB)
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q;

 if (TRACE>2)
    printf("          STOP TIMER: Parando o tempo em: %f\n",simul_time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
       if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
             q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
             q->next->prev=NULL;
             evlist = q->next;
             }
           else {     /* middle of list */
             q->next->prev = q->prev;
             q->prev->next =  q->next;
             }
       free(q);
       return;
     }
  printf("Aviso: não é possível cancelar seu temporizador. Não estava correndo.\n");
}


void starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;

 if (TRACE>2)
    printf("          START TIMER: Começando o cronômetro em %f\n",simul_time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)  
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
      printf("Aviso: tentativa de iniciar um cronômetro que já foi iniciado\n");
      return;
      }
 
/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  simul_time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
double random_number() {

  // generate a uniform random number in the interval [0,1)
  return (double)1.0*rand()/(RAND_MAX+1.0);
}

void init_random(unsigned int randseed) {
  
  // initialize the random number generator
  if (!randseed) {
    srand((unsigned int)time(NULL));
  } else
    srand(randseed);
}

void tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
 struct pkt *mypktptr;
 struct event *evptr,*q;
 float lastime, x;
 int i;

 ntolayer3++;

 /* simulate losses: */
 if (random_number() < lossprob)  {
      nlost++;
      if (TRACE>0)    
	printf("          TOLAYER3: Pacote sendo perdido\n");
      return;
 }  

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */ 
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: Sequencia: %d, Ack %d, checksum: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = simul_time;
 for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
 evptr->evtime =  lastime + 1.0 + 9.0*random_number();
 

 /* simulate corruption: */
 if (random_number() < corruptprob)  {
    ncorrupt++;
    if ( (x = random_number()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)    
	printf("          TOLAYER3: Pacote sendo corrompido\n");
    }  

  if (TRACE>2)  
     printf("          TOLAYER3: Agendamento de chegada no outro lado\n");
  insertevent(evptr);
} 

void tolayer5(AorB, msgReceived)
  int AorB;
  struct msg msgReceived;
{
  int i;  
  if (TRACE>2) {
     printf("          TOLAYER5: Dado recebido: ");
     for (i=0; i<20; i++)  
        printf("%c",msgReceived.data[i]);
     printf("\n");
  }
  
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
 
void generate_next_arrival(int entity)
{
   double x;
   struct event *evptr;

   if (TRACE>2)
       printf("          GERAR PRÓXIMA CHEGADA: criar nova chegada\n");
 
   x = lambda*random_number()*2;   /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  simul_time + x;
   evptr->evtype =  FROM_LAYER5;

   if (entity) 
     evptr->eventity = entity;
   else {
     if (BIDIRECTIONAL && (random_number()>0.5) )
       evptr->eventity = B;
     else
       evptr->eventity = A;
   }
   insertevent(evptr);
} 


/*************** INITIALIZATION ROUTINE  *************/
/* Read input from user and initalize parameters     */
/*****************************************************/
void init()                         
{
   int i;
   float sum, avg;

   printf("-----  Simulador Para e Espera - Version 1.1 -------- \n\n");
   printf("|---------------------------------------------------------------------------|\n");
   printf("|Informe a quantidade de mensagem a simular:                                | = ");
   scanf(" %d",&nsimmax);
   printf("|Informe a probabilidade de perda de pacote [informe 0.0 sem perdas]:       | = ");
   scanf(" %f",&lossprob);
   printf("|Informe a probabilidade de corromper pacote [informe 0.0 sem corromper]:   | = ");
   scanf(" %f",&corruptprob);
   printf("|Informe o tempo entre msgs do emissor na camada 5[ > 0.0]:                 | = ");
   scanf(" %f",&lambda);
   printf("|Informe TRACE ou Nível de detalhamento:                                    | = ");// >2 p/ mensagens de debug
   scanf(" %d",&TRACE);
   printf("|---------------------------------------------------------------------------|\n\n");


   /* init random number generator */
   init_random(randseed);

   sum = 0.0;                /* test random number generator for students */
   for (i=0; i<1000; i++)
      sum=sum+random_number();     /* should be uniform in [0,1) */
   avg = sum/1000.0;
   if ((avg < 0.25) || (avg > 0.75)) {
    printf("É provável que a geração de números aleatórios em sua máquina\n" ); 
    printf("é diferente do que esse emulador espera. Por favor, pegue\n");
    printf("uma olhada na rotina random_number () no código do emulador. Desculpa. \n");
    exit(0);
   }

   ntolayer3 = 0;
   nlost = 0;
   ncorrupt = 0;

   simul_time=0.0;                     /* initialize simul_time to 0.0 */
   generate_next_arrival(0);     /* initialize event list */
}

/********************* MAIN ROUTINE  *****************/
/* Main simulation loop and handling of events       */
/*****************************************************/
int main(void)
{
   struct event *eventptr;
   struct msg  msg2give;
   struct pkt  pkt2give;
   
   int i,j;

   /* initialize our data structures and read parameters */
   init();

   /* call the user's init functions */
   A_init();
   B_init();
   
   /* loop forever... */
   while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL) {
	  printf("PÂNICO INTERNO: A lista de eventos está vazia! Isto não deveria ter acontecido.\n");
	  break;
	}
        evlist = evlist->next;        /* remover este evento da lista de eventos */
        if (evlist!=NULL)
           evlist->prev=NULL;
        if (TRACE>=2) {
           printf("\nTempo do evento: %f,",eventptr->evtime);
           printf("  Tipo: %d",eventptr->evtype);
           if (eventptr->evtype==0)
	       printf(", Tempo de interrupção  ");
             else if (eventptr->evtype==1)
               printf(",Para camada 5 ");
             else
	     printf(", Para camada 3 ");
           printf(" entity: %d\n",eventptr->eventity);
           }
        simul_time = eventptr->evtime;        /* atualize simul_time para a próxima hora do evento */
        if (nsim==nsimmax)
	  break;                        /* tudo feito com simulação */
        if (eventptr->evtype == FROM_LAYER5 ) {

            /* preencha msg para dar com string da mesma letra */    
            j = nsim % 26; 
            for (i=0; i<20; i++)  
               msg2give.data[i] = 97 + j;
            if (TRACE>2) {
               printf("          MAINLOOP: Dados enviados ao estudante: ");
                 for (i=0; i<20; i++) 
                  printf("%c", msg2give.data[i]);
               printf("\n");
	    }

            if (eventptr->eventity == A) 
               j = A_output(msg2give);
           
            
           
	    if (j < 0) {
	      if (TRACE>=1)
		printf("MAINLOOP: dados NÃO aceitos pela camada 4\n");
	      /* configurar futura chegada para a mesma entidade*/
	      generate_next_arrival(eventptr->eventity);   

	    } else {
	      nsim++;
	      if (TRACE>=1)
		printf("          MAINLOOP: dados aceitos pela camada 4\n");
	      /* configurar futura chegada */
	      generate_next_arrival(0);   
	    }
	}
          else if (eventptr->evtype ==  FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i=0; i<20; i++)  
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity == A)      /* entregar pacote chamando */
   	       A_input(pkt2give);             /* Entidade apropriada */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* limpar pacote na memória */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A) 
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
	     break;
	  }
        free(eventptr);
   }

   printf(" Simulador parou no tempo %f\n Após enviar %d pacotes para Camada 5\n",simul_time,nsim);
   return 0;
}
