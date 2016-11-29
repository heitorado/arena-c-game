#include "lib/client.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define NAME_SIZE 40

struct player{
    int x, y;
    // int informações do game, hp, ataque, defesa, pontos...;
    int vida, ataque, defesa, turn;
    //ADICIONADO
    int DEX,WIS,MaxHP;
    int sockid, fight, whofight;
    char nome[NAME_SIZE];
    char ant;
};

typedef struct player player;


//VARIAVEIS GLOBAIS DE MAPA
	int largura_mapa,altura_mapa,mobs_mapa; //largura, altura e quantidade de mobs do mapa.

//VARIAVEL GLOBAL DE IP
	char IP[20];
    char ch;


char** openMap(char nomemapa[]);//função que abre o mapa da sessão e aloca memoria para comportá-lo.
void printMap(char **mapa); //função que printa o mapa na tela do jogador.
void printStatus(upd_msg *updt);
void printBattleStatus(upd_msg *player , upd_msg *opp);

int selectClass(void);//Muda a classe do usuario
int menuOptions(clientInfo *info); //função que abre o menu de opções
int menuPrincipal(clientInfo info); //função que abre o menu principal
void readStoredInfos(clientInfo *info);
void menuInstructions(void);//abre o menu

int CheckFileExistence(char *filename);
void MapGenerator();
void MonsterFactory(int mapNumber , int num_mobs);

int main()
{
    FILE *PVPIMG = fopen("data/monster_battle.txt","rt");
    FILE *GAMEOVERSCREEN;
    FILE *WINSCREEN;

    clientInfo info; //cria uma variavel "info" do tipo clientInfo, que armazena o numero do mapa e o nome do client.
    readStoredInfos(&info); //le o que ja tem no clientinfo.bin
    info.classe=0;

    char msg[BUFFER_SIZE];//512
	char **MAPA; //mapa do jogo atual
    int falha = 0, comecar = 0, jogando = 1;
    int statusMenu=0, statusOptions=0; //variaveis de menu
    //int classe=0;
    int BATTLE=0;
    int read=0;
    char quit=0; //variavel para finalizar o jooj

    upd_msg updt, PLAYER, OPPONENT; //cria var do tipo upd_msg, que faz atualizações dependendo do tipo (mapa, vida, etc)
    mov_msg mov; //cria var do tipo mov_msg (manda w,a,s,d)

    do{
       
       do
       {
            statusMenu=menuPrincipal(info);
            statusOptions=0;
            jogando=1;
            quit=0;
            switch(statusMenu)
            {
                case 1: if(info.classe>0)
                        {
                            connectToServer(IP);//Start Game
                            statusOptions++;
                        }
                        break;
                case 2: info.classe=selectClass(); //Change Class, retorna o valor da classe;
                        break;

                case 3: menuOptions(&info); //Options
                        break;

                case 4: menuInstructions();//Instructions
                        break;
                case 5: exit(1);//Quit
                        break;
            }
        }while(statusOptions!=1);//O usuario apertou home no options

        sendInfoToServer(info); // primeira mensagem é sempre clientInfo

        while(!comecar && !falha) // depois só mensagem de texto até o jogo começar
        {

           	mov.msg=getch();

            if(mov.msg>-1)
            {
                sendMovToServer(mov);
            }

            if(readTxtFromServer(msg) > 0)
            {
                if(msg[0] == '+')
                    comecar = 1;

                else if(msg[0] == '-')
                    falha = 1;

                    printf("%s\n", msg);
            }
        }

        if(falha)
            exit(1);

        //receber informações iniciais do jogo(mapa, status inicial, etc...)
        //depois desse ponto, todas as mensagens recebidas serão de update, e as enviadas são de movimento.

    	if(readTxtFromServer(msg) > 0)     //verifica se tem uma mensagem chegando
    		MAPA=openMap(msg);             //é o mapa chegando

    	printMap(MAPA);

        //SÓ ENVIA: WASD
        //SÓ RECEBE: ATUALIZAÇÕES DE MAPA E STATUS


        while(jogando)
        {
            while(readUpdFromServer(&updt) > 0)
            {// recebe todas mensagens
                // qual o tipo do update, ex:

                //printf("pelo menos aqui ne\n");
                switch(updt.tipo)
                {
                    case 0: //Case 0 - Apenas andando pelo mapa
                        MAPA[updt.x][updt.y]=updt.new;

                        break;
                    case 1: //Case 1 - Em batalha
                        system("clear");
                        fseek(PVPIMG,0,SEEK_SET);
                        while((ch =fgetc(PVPIMG)) != EOF) 
                        {
                            putchar(ch);
                        }
                        BATTLE=1; //variavel para dizer q ta em batalha
                        //printStatus(&PLAYER); //printa o proprio status
                        OPPONENT.vida = updt.vida;
                        OPPONENT.x = updt.x;
                        OPPONENT.y = updt.y;
                        //printStatus(&OPPONENT); //printa status do oponente
                        printBattleStatus(&PLAYER,&OPPONENT); //printa ambos os status
                        
                        if(readTxtFromServer(msg) > 0)
                            printf("%s\n",msg);

                        break;

                    case 2://Case 2 - Recebendo atualizações de status
                        //system("clear");
                        //printMap(MAPA);
                        PLAYER.vida = updt.vida;
                        PLAYER.x = updt.x;
                        PLAYER.y = updt.y;
                        PLAYER.newDex = updt.newDex;
                        PLAYER.newWis = updt.newWis;
                        //printStatus(&PLAYER);

                        break;

                    case 3: //Case 3 - Saindo Vitorioso da Batalha
                        //ESPERA?
                        BATTLE=0;

                        break;

                    case 4:
                    	system("clear");
                    	GAMEOVERSCREEN=fopen("data/game_over.txt","rt");
                    	fseek(GAMEOVERSCREEN,0,SEEK_SET);

                        while((ch=fgetc(GAMEOVERSCREEN)) != EOF) 
                        {
                            if(ch=='*')
                            {
                                printf(BLU);
                                putchar(ch);
                                
                            }
                            else
                                if(ch=='>')
                                {                        
                                    printf(YEL);
                                    putchar(ch);                        
                                }
                                else
                                    if (ch==':'||ch=='\\'||ch=='/'||ch=='.'||ch=='('||ch==')'||ch=='_'||ch=='-'||ch=='`')
                                    {
                                        printf(RED);   
                                        putchar(ch);
                                    }
                                    else
                                    {                        
                                        printf(RED);   
                                        putchar(ch);                     
                                    }                            
                        }
                        printf(RESET"\n");                    	
                        

                        while(quit <= 0)
                        {
                        	quit = getch();

                            jogando=0;
                            info.classe=0;
                            falha = 0;
                            comecar = 0;
                            statusMenu=0;
                            statusOptions=0;
                            BATTLE=0;
                        }
                        fclose(GAMEOVERSCREEN);
                        break;

                    case 5:
                        comecar = 0;
                        system("clear");
                        WINSCREEN=fopen("data/win_frame.txt","rt");
                        fseek(WINSCREEN,0,SEEK_SET);
                        while((ch=fgetc(WINSCREEN)) != EOF) 
                        {
                            if(ch=='*')
                            {
                                printf(BLU);
                                putchar(ch);
                                
                            }
                            else
                                if(ch=='>')
                                {                        
                                    printf(YEL);
                                    putchar(ch);                        
                                }
                                else
                                    if (ch==':'||ch=='\\'||ch=='/'||ch=='.'||ch=='('||ch==')'||ch=='_'||ch=='-'||ch=='`')
                                    {
                                        printf(RED);   
                                        putchar(ch);
                                    }
                                    else
                                    {                        
                                        printf(RED);   
                                        putchar(ch);                     
                                    }                            
                        }
                        printf(RESET"\n"); 

                        while(quit <= 0)
                        {
                            quit = getch();

                            jogando=0;
                            info.classe=0;
                            falha = 0;
                            comecar = 0;
                            statusMenu=0;
                            statusOptions=0;
                            BATTLE=0;
                        }
                        fclose(WINSCREEN);

                    	break;

                    /*
                    tipo
                    = 0 atualiza o mapa, (x,y) é a  posição, e new é o novo char
                    = 1 mesagem para avisar que vai entrar em batalha/atualiza status do oponente vida é a vida do oponente, x é o ataque, y é a defesa
                    = 2 atualiza a meus status, vida é a minha nova vida, x é meu novo ataque, y é minha nova defesa
                    = 3 mensagem para avisar que saiu de batalha
                    = 4 mensagem para avisar que morreu
                    = 5 mensagem para avisar que venceu o jogo
                    */


                }
            }

            if(!BATTLE)
            {
                system("clear");
                printMap(MAPA);
                printStatus(&PLAYER);
            }

            mov.msg = getch();

            if(mov.msg != -1) // retorna -1 se demorou muito e nada foi digitado.
                sendMovToServer(mov);
        }
    }while(quit!='q' && quit!='Q');

    return 0;
}

char** openMap(char nomemapa[])
{
	int i,j;
	char **MAPA;
	FILE *MAPFILE;

	MAPFILE=fopen(nomemapa,"r");

	fscanf(MAPFILE,"%d %d %d", &altura_mapa,&largura_mapa,&mobs_mapa);

	MAPA=(char**)calloc(altura_mapa,sizeof(char*));

	for(i=0;i<altura_mapa;i++)
		MAPA[i]=(char*)calloc(largura_mapa+1,sizeof(char));

	for(i=0;i<altura_mapa;i++)
		for(j=0;j<largura_mapa+1;j++)
			MAPA[i][j]=fgetc(MAPFILE);

	fclose(MAPFILE);

	return MAPA;
}

void printMap(char **MAPA)
{
    int i,j;

    system("clear");

    for(i=0;i<altura_mapa;i++)
        for(j=0;j<largura_mapa+1;j++)
            if(MAPA[i][j]=='*')
            {
                printf("%c",MAPA[i][j] );
                //printf(RESET);
            }
            else
                if(MAPA[i][j]=='m')
                {
                    printf(YEL "%c",MAPA[i][j] );
                    printf(RESET);
                }
                else if (MAPA[i][j]=='M')
                {
                    printf(RED "%c",MAPA[i][j] );
                    printf(RESET);
                }
                else
                {
                    printf(RESET"%c",MAPA[i][j] );
                    printf(RESET);
                }

    printf("\n");
}

void printStatus(upd_msg *player)
{
    printf("\n");
    printf("<HERO STATS>\n");
    printf(RED"<HP>: %d\n",player->vida);
    printf(WHT"<ATK>: %d\n",player->x);
    printf(YEL "<DEF>: %d\n",player->y);
    printf(MAG"<DEX>: %d\n",player->newDex);
    printf(BLU "<WIS>: %d\n",player->newWis);
    printf(RESET);
    printf("\n");
}

void printBattleStatus(upd_msg *player , upd_msg *opp)
{
    printf("\n");
    printf("<YOU>\t\t\t\t\t\t <ENEMY>\n");
    printf("<HP>: %d\t\t\t\t\t <HP>: %d\n",player->vida, opp->vida);
    printf("<ATK>: %d\t\t\t\t\t <ATK>: %d\n",player->x, opp->x);
    printf("<DEF>: %d\t\t\t\t\t <DEF>: %d\n",player->y,opp->y);
    printf("\n");
}

void readStoredInfos(clientInfo *info)
{
    FILE *CLIENTPROFILE;
    FILE *LASTIP;

    CLIENTPROFILE=fopen("data/clientInfo.bin","r+b");
    LASTIP=fopen("data/lastConn.bin","r+b");

    fread(info,sizeof(clientInfo),1,CLIENTPROFILE);
    fread(IP,sizeof(char)*20,1,LASTIP);

    fclose(CLIENTPROFILE);
    fclose(LASTIP);
}

int menuOptions(clientInfo *info)               //VERIFICAR OS INDICES DO ARQUIVO, EM RELAÇAO AO LOCAL DOS APONTADORES DE OPÇÕES
{
    FILE *arq=fopen("data/options.txt", "r+b");
    FILE *CLIENTPROFILE=fopen("data/clientInfo.bin","r+b"); //Abre o arquivo para atualizar os dados do client (se necessario)
    FILE *LASTIP=fopen("data/lastConn.bin","r+b");			//Abre o arquivo para atualizar o IP (se necessario)

    char tipo[2600], ch='w';
    int i;//printa o arquivo
    long long int num=40001;
    fread(tipo, sizeof(char), 2600, arq);
    while(1)
    {
        if(ch=='w'||ch=='s'||ch=='W'||ch=='S'||ch=='d'||ch=='D')
        {
            if(ch=='s'||ch=='S')
                num++;
            if(ch=='w'||ch=='W')
                num--;
            if(ch=='d'||ch=='D')
            {
                if(num%5==0)//Change Name
                {
                    system("clear");
                    printf("Digite seu nome\n");
                    scanf(" %[^\n]", info->nome);
                }
                if(num%5==1)//Change Map
                {
                    system("clear");
                    printf("Digite o numero do mapa\n");
                    scanf("%d", &info->mapa);
                }
                if(num%5==2)//Change Server IP
                {
                    system("clear");
                    printf("Digite o IP que deseja se conectar: ");
                    scanf("%s",IP);
                }
                if(num%5==3)//Creat Random Map
                    MapGenerator();
                if(num%5==4)//home
                {
                	fwrite(info,sizeof(clientInfo),1,CLIENTPROFILE);
                	fwrite(IP,sizeof(char)*20,1,LASTIP);
                    fclose(CLIENTPROFILE);
                    fclose(LASTIP);
                    return 1;
                }
            }
            system("clear");
            if(num%5==0)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]='>';
            }
            if (num%5==1)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]='>';             tipo[1642]=' ';
            }
            if (num%5==2)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]='>';            tipo[1804]=' ';             tipo[1642]=' ';
            }
            if (num%5==3)
            {
                tipo[2290]=' ';            tipo[2128]='>';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]=' ';
            }
            if (num%5==4)
            {
                tipo[2290]='>';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]=' ';
            }            
            for ( i = 0; i < 2592; i++)
            {
                if(tipo[i]=='*')
                {
                    printf(BLU"%c", tipo[i]);
                    
                }
                else
                    if(tipo[i]=='>')
                    {                        
                        printf(YEL"%c", tipo[i]);                        
                    }
                    else
                        if (tipo[i]==':'||tipo[i]=='\\'||tipo[i]=='/'||tipo[i]=='.'||tipo[i]=='('||tipo[i]==')'||tipo[i]=='_'||tipo[i]=='-'||tipo[i]=='`')
                        {
                            printf(RED"%c", tipo[i]);   
                        }
                        else
                        {                        
                            printf(RED"%c", tipo[i]);                        
                        }

            }
            printf(RESET"\n");
            ch='\0';
        }
        ch=getch();
        usleep(200);
    }
}

int menuPrincipal(clientInfo info/*clientInfo info->nome, clientInfo info->mapa TALVEZ PRECISE DE ARGUMENTOS*/)       // NAO SEI SE ISSO FUNCIONA
{
    FILE *arq=fopen("data/tela_inicial.txt", "r+b");
    char tipo[2592], ch='w';
    int i;
    long long int num=40001;
    fread(tipo, sizeof(char), 2592, arq);
    while(1)
    {
        if(ch=='w'||ch=='s'||ch=='W'||ch=='S'||ch=='d'||ch=='D')
        {
            if(ch=='s'||ch=='S')
                num++;
            if(ch=='w'||ch=='W')
                num--;
            if(ch=='d'||ch=='D')
            {
                if(num%5==0)//start game
                    return 1;
                if(num%5==1)//selecte class
                    return 2;
                if(num%5==2)//options
                    return 3;
                if(num%5==3)//instruction
                    return 4;
                if(num%5==4)//quit
                    return 5;
            }
            system("clear");
            if(num%5==0)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]='>';
            }
            if (num%5==1)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]='>';             tipo[1642]=' ';
            }
            if (num%5==2)
            {
                tipo[2290]=' ';            tipo[2128]=' ';            tipo[1966]='>';            tipo[1804]=' ';             tipo[1642]=' ';
            }
            if (num%5==3)
            {
                tipo[2290]=' ';            tipo[2128]='>';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]=' ';
            }
            if (num%5==4)
            {
                tipo[2290]='>';            tipo[2128]=' ';            tipo[1966]=' ';            tipo[1804]=' ';             tipo[1642]=' ';
            }
            
            for ( i = 0; i < 2592; i++)
            {
                if(tipo[i]=='*')
                {
                    printf(BLU"%c", tipo[i]);
                    
                }
                else
                    if(tipo[i]=='>')
                    {                        
                        printf(YEL"%c", tipo[i]);                        
                    }
                    else
                        if (tipo[i]==':'||tipo[i]=='\\'||tipo[i]=='/'||tipo[i]=='.'||tipo[i]=='('||tipo[i]==')'||tipo[i]=='_'||tipo[i]=='-'||tipo[i]=='`')
                        {
                            printf(RED"%c", tipo[i]);   
                        }
                        else
                        {                        
                            printf(RED"%c", tipo[i]);                        
                        }

            }
            printf(RESET"\n");


            ch='\0';

            printf("Username: %s\n",info.nome);
            printf("Map Selected: %d\n",info.mapa);
            printf("Connect to: %s\n",IP);

            if(info.classe > 0)
                switch(info.classe)
                {
                    case 1: printf("Class Chosen: ");
                            printf(CYN"PALADINO");
                            printf(RESET"\n");
                            break; 
                    case 2: printf("Class Chosen: ");
                            printf(CYN"LADRÃO");
                            printf(RESET"\n");
                            break;
                    case 3: printf("Class Chosen: ");
                            printf(CYN"CAÇADOR");
                            printf(RESET"\n");
                            break;
                    case 4: printf("Class Chosen: ");
                            printf(CYN"MAGO");
                            printf(RESET"\n");
                            break;
                    case 5: printf("Class Chosen: ");
                            printf(CYN"GUERREIRO");
                            printf(RESET"\n");
                            break;            
                }
            else
            {
                printf(RED"Please Select a Class Before Starting!");
                printf(RESET"\n");
            }


        }
        ch=getch();
        usleep(200);
    }
    fclose(arq);
}
int selectClass()
{

    FILE *arq;
    int num=2000, i;
    long int TamArq;
    char tipo[4031];
    char teste='u';
    while(1)
    {
        switch(teste)
        {
            case 'a':
            case 'A':
                    num--;
                    break;

            case 'd':
            case 'D':num++;
                    break;

            default: break;
        }
        if(num%5==0)
        {
            system("clear");
            arq=fopen("data/paladino.txt", "r+b");
            fread(tipo, sizeof(char), 1196, arq);
            TamArq=1196;
            for ( i = 0; i<TamArq; i++)
                printf(GRN"%c", tipo[i]);
            printf("\n\n");
            printf(RED"\n\t\t\t*********************************\n");            
            printf("\t\t\t*");     
            printf(BLU);       
            printf(" guerreiro");
            printf(YEL" <-PALADINO-> ");
            printf(BLU"ladrao ");
            printf(RED"*\n");
            printf(RED"\t\t\t*********************************");
            printf(RESET"\n");   
            
            fclose(arq);
        }
        if(num%5==1)
        {
            system("clear");
            arq=fopen("data/ladrao.txt", "r+b");
            fread(tipo, sizeof(char), 1047, arq);
            TamArq=1047;
            for ( i = 0; i<TamArq; i++)
                printf(GRN"%c", tipo[i]);
            printf(RED"\n\t\t\t*********************************\n");;            
            printf("\t\t\t*");     
            printf(BLU);           
            printf(" paladino ");
            printf(YEL" <-LADRAO->");
            printf(BLU"  cacador ");
            printf(RED"*\n");
            printf(RED"\t\t\t*********************************");
            printf(RESET"\n");  
           
            fclose(arq);
        }
        if(num%5==2)
        {
            system("clear");
            arq=fopen("data/cacador.txt", "r+b");
            fread(tipo, sizeof(char), 1608, arq);
            TamArq=1608;
            for ( i = 0; i<TamArq; i++)
                printf(GRN"%c", tipo[i]);
            printf(RED"\n\t\t\t*********************************\n");            
            printf("\t\t\t*");     
            printf(BLU); 
            printf("   ladrao");
            printf(YEL"  <-CAÇADOR->  ");
            printf(BLU"mago   ");
            printf(RED"*\n");
            printf(RED"\t\t\t*********************************");
            printf(RESET"\n");  
            
            fclose(arq);
        }
        if(num%5==3)
        {
            system("clear");
            arq=fopen("data/guerreiro.txt", "r+b");
            fread(tipo, sizeof(char), 1525, arq);
            TamArq=1531;
            printf("\n");
            for ( i = 0; i<TamArq; i++)
                printf(GRN"%c", tipo[i]);
            printf(RED"\n\t\t\t*********************************\n");            
            printf("\t\t\t*");     
            printf(BLU);   
            printf(" cacador");
            printf(YEL"  <-MAGO->  ");
            printf(BLU"guerreiro  ");
            printf(RED"*\n");
            printf(RED"\t\t\t*********************************");
            printf(RESET"\n"); 
            fclose(arq);
        }
        if(num%5==4)
        {
            system("clear");
            arq=fopen("data/mago.txt", "r+b");
            fread(tipo, sizeof(char), 1209, arq);
            TamArq=1203;
            for ( i = 0; i<TamArq; i++)
                printf(GRN"%c", tipo[i]);
            printf("\n\n");
            
            printf(RED"\n\t\t\t*********************************\n");            
            printf(RED"\t\t\t*");
            printf(BLU" mago");
            printf(YEL"  <-GUERREIRO->  ");
            printf(BLU"paladino ");
             printf(RED"*\n");
            printf("\t\t\t*********************************");
            printf(RESET"\n"); 
            fclose(arq);
        }
        usleep(500);
        printf(RESET); 
        teste=getch();
        if (teste=='\n')
            if (num%5==0)
                
                return 1; //Retorna 1 se escolher PALADINO
            else
                if (num%5==1)
                    return 2; //Retorna 2 se escolher LADRAO
                else
                    if (num%5==2)
                        return 3; //Retorna 3 se escolher CACADOR
                    else
                        if (num%5==3) //Retorna 4 se escolher GUERREIRO
                            return 4;
                        else
                            return 5; //Retorna 5 se escolher MAGO
                        printf(RESET); 
    }
    printf(RESET); 
}

void menuInstructions()
{
    char controleDeSaida='a';
    system("clear");
    printf(RED"\n\t\t\t\t\t> INSTRUCOES DE JOGO <\n\n"RESET);
    printf("\t-> Digite seu nome em Options\n");
    printf("\t-> Digite o mapa desejado em Options\n");
    printf("\t-> Digite o IP do server em Options\n");
    printf("\t-> Escolha uma classe\n");
    printf("\t-> Teclas de movimentacao: 'w', 'a, 's', 'd'\n");
    printf("\t-> Teclas na batalha: 'd' -> Ataque Basico, 'w','s,' -> Habilidades\n");

    printf(RED"\n\t\t\t\t\t> Seja bem vindo a arena <\n"RESET);
    printf("\tVocê foi escolhido para participar da arena entre os grandes, prepare-se.\nOs monstros serão liberados, preparando armadilhas, clones e preparados para matar \nquaisquer um que entrem em seus caminhos.\n Cuidado com o monstro chefe, tenha certeza de que já adquiriu experiência\n suficiente para derrotá-lo.\n Os conpetidores entrarão na arena, \npreparados para lutar pelas recompensas. Mas apenas um pode sobreviver.\nSeja o vencedor de recompensas inimaginaveis, gloria e ouro que nenhum homem imaginou.\nPerca e você será impiedosamente executado, então boa sorte!\n");

    printf(RED"\n\t\t\t\t\t> CLASSES <\n"RESET);

    printf("\nCACADOR -> Desde cedo, o chamado das selvas incita alguns aventureiros a deixarem a comodidade\nde seus lares para adentrarem o impiedoso mundo selvagem. Aqueles que sobrevivem acabam se\ntornando CACADORES. Senhores de seu dominio, os cacadores são capazes de deslizar como fantasmas\n por entre as arvores e colocar armadilhas.\n");
    printf("\n - HABILIDADE -\nArapuca->O cacador prepara uma armadilha e causa dano e pode atordoar o oponente\n");
    printf("\nMAGO-> Estudantes privilegiados com intelecto agucado e disciplina inabalavel, este e o MAGO\nA magia e igualmente incrivel como perigosa, Magos sao capazes de evocar bolas de fogo para\nexplodir seus inimigos, bem como rajadas de gelo, para congela-los.\n");
    printf("\n - HABILIDADE -\nBola de Fogo-> Uma enorme bola de fogo para explodir seus oponentes\n");
    printf("\nGUERREIRO-> GUERREIROS combinam força, liderança e vasto conhecimenhto em armas e armaduras para\ncriar o caos no campo de batalha. As vezes protegendo a linha de frente com seus escudos, as\nvezes liberando sua furia na ameaca mais proxima, com armas letais.\n");
    printf("\n - HABILIDADE -\nEscudada-> O guerreiro bate como escudo no oponente\n");
    printf("\nPALADINO-> Este e o dever do PALADINO: proteger os fracos, trazer justica aos injustos e exterminar\n o mal nos confins mais sombrios do mundo. Este guerreiro sagrado equipado com armas divinas e adepto\nda bencao da luz que lhe permite curar feridas.\n");
    printf("\n - HABILIDADE -\nCura Divina-> O paladino pede ajuda aos divinos, curando suas feridas e ficando mais forte\n");
    printf("\nLADINO-> Para os LADINOS, o unico codigo que existe e o contrato e sua honra pode ser comprada. \nLivre de escrupulos, estes mercenarios empregam o uso de taticas brutais e eficientes. \nAssassinos letais e mestres da furtividade, usam as taticas mais sujas ao seu alcance.\n");
    printf("\n - HABILIDADE -\nVeneno-> O ladino aplica veneno em suas laminas, causando mais dano\n");

    printf(RED"\n*********");
    printf(GRN"\n> ");
    printf(YEL"QUIT");
    printf(RED"  *\n");
    printf("*********");
    printf(RESET"\n");
    while(controleDeSaida!='d')
        controleDeSaida=getch();
    return;
}

void MapGenerator()
{
    
    srand( (unsigned)time(NULL) );

    FILE *arquivodemapa;
    FILE *monstrosmapa;
    int mapNumber; //numero do mapa
    int NL; //numero de linhas do mapa
    int NC; //numero de colunas do mapa
    int Mobs_Qtd; //numero de monstros do mapa
    char mapfilename[512];//trocar pra buffer_size

    printf(CYN"Digite o numero do mapa que deseja criar (ID) : ");
    printf(RESET"");
    scanf("%d",&mapNumber);

    sprintf(mapfilename,"data/mapa%d.txt",mapNumber);


    while(CheckFileExistence(mapfilename))
    {
        printf(RED"Esse mapa já existe. Por favor digite outro numero para o mapa.\n");
        printf(RESET"");
        scanf("%d",&mapNumber);

        sprintf(mapfilename,"data/mapa%d.txt",mapNumber);
    }

    arquivodemapa=fopen(mapfilename,"wt");
        if(arquivodemapa==NULL){
            printf(RED"Erro Gerando Mapa\n");
            printf(RESET"");
            exit(1);
        }

    printf("Digite a Altura do Mapa:\n");
    scanf("%d",&NL);
    while(NL<10 || NL>100)
    {
        printf(RED"Valor Invalido! Digite outro valor.\n");
        printf(RESET"");
        scanf("%d",&NL);   
    }

    printf("Digite a Largura do Mapa:\n");
    scanf("%d",&NC);
    while(NC<10 || NL>100)
    {
        printf(RED"Valor invalido! Digite outro valor.\n");
        printf(RESET"");
        scanf("%d",&NC);   
    }

    printf("Digite a quantidade de Monstros desejada no Mapa:\n");
    scanf("%d",&Mobs_Qtd);
    while(Mobs_Qtd<0 || Mobs_Qtd>50)
    {
        printf(RED"Quantidade Invalida! Digite novamente.\n");
        printf(RESET"");
        scanf("%d",&Mobs_Qtd);
    }

    fprintf(arquivodemapa, "%d %d %d\n", NL,NC,Mobs_Qtd);

    int matriz[512][512],i,j,x;

    for (i=0;i<NL;i++)
        matriz[i][0]='*';

    for (i=0;i<NC;i++)
        matriz[0][i]='*';

    for (i=0;i<NC;i++)
        matriz[NL-1][i]='*';

    for (i=0;i<NL;i++)
        matriz[i][NC-1]='*';

    for (i=1;i<(NL-1);i++)
    {
        for (j=1;j<(NC-1);j++)
        {
            x=rand()%8;
            
            if (x>2) 
                matriz[i][j]=' ';
            else 
                matriz[i][j]='*';
        }
    }
    for (i=0;i<NL;i++){
        for (j=0;j<NC;j++)
        {
            fprintf(arquivodemapa,"%c",matriz[i][j]);
        }
        fprintf(arquivodemapa,"\n");
    }

    fclose(arquivodemapa);

    printf(GRN"Mapa Criado. Iniciando criador de monstros.\n");
    printf(RESET"");

    MonsterFactory(mapNumber , Mobs_Qtd);
}

int CheckFileExistence(char *filename)
{
    FILE *check;
    if (check = fopen(filename, "r"))
    {
        fclose(check);
        return 1;
    }
    return 0;
}




void MonsterFactory(int mapNumber , int num_mobs)
{
    FILE *MONSTER;
    int i;
    player *mobs;
    char monstersfilename[512];

    mobs=(player*)calloc(num_mobs,sizeof(player));

    sprintf(monstersfilename,"data/monstros%d.bin",mapNumber);

    MONSTER=fopen(monstersfilename,"wb");
    
    for(i=0;i<num_mobs;i++)
    {   
        if(i==num_mobs-1)
            printf(YEL "<Criando BOSS>\n");
        else    
            printf(YEL "<Criando Monstro Numero [%d]>\n", i+1);

        printf(RESET"");
        printf(CYN "Nome: ");
        printf(RESET"");
        scanf(" %[^\n]",mobs->nome);
        printf(RED"HP: ");
        printf(RESET"");
        scanf("%d",&mobs->vida);
        printf(RED"Ataque: ");
        printf(RESET"");
        scanf("%d",&mobs->ataque);
        printf(RED"Defesa: ");
        printf(RESET"");
        scanf("%d",&mobs->defesa);
        
        
        mobs->turn=0;
        mobs->fight=0;
        mobs->whofight=0;
        mobs->MaxHP=0;
        mobs->DEX=0;
        mobs->WIS=0;

        fwrite(mobs,sizeof(player),1,MONSTER);
    }

    fclose(MONSTER);
}