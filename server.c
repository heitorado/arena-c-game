#include "lib/server.h"

typedef struct{
    int skill;
}cd;

void MyClientConnected(int id, clientInfo startInfo); // função que é chamada quando um cliente é connectado
void MyClientMoved(int id, mov_msg mov);// função que é chamada quando cliente manda mensagem de movimento
void startGame(void);// função que é chamada quando cliente 0 confirma o inicio do jogo
void MyBroadcast(void);// um exemplo de como mandar uma mensagem para todos os usuários
char movMonster(char PosMovAnt);
int isMonster(int x, int y);
void moveMonster(int id);

void clearClientPosition(int id); //limpa a posição atual do cliente (informado na ID)
void clearMonsterPosition(int id);
void moveClient(int Newx, int Newy, int id); //move o cliente com id em + ou - x ou y
int isTree(int x, int y);
int isPlayer(int x, int y);
int isMonster(int x, int y);
void isDead(int op_id, int id);
void isDeadPvE(int player_id, int mob_id);
void KILL( int op_id );
void KILLMONSTER (int mon_id);
void victory( int id );
int nullMovPlayer(int x, int y, char mov);
void lifeBonus(int id , upd_msg *updt); //função para dar bonus de vida
void startBattle(int idPlayer1, int idPlayer2);
void startBattleMonster(int idPlayer1, int idMonster2);
void baseAttack(int id);
void baseAttackM(int id);
void monsterIsDead(int mon_id, int id);
void monsterAttack(int jog_id, int mon_id);

void checkIfWinner(int id);
void subirDeNivel(int id);



void fireBallM(int id);
void fireBall(int id);
void shieldSlamM(int id);
void shieldSlam(int id);
void DivineHealM(int id);
void DivineHeal(int id);
void veneno(int id);
void arapucaM(int id);
void arapuca(int id);

int contadorDeVeneno[5]={};

char **cacheMAP;
int mapaAlt,mapaLarg,qtdMobs,mapa,mobsKilled=0;
player *mobs;
void printCacheMap(void);

cd skillcd[5]={};



int main ()
{
    srand(time(NULL)); // seed pra usar o rand durante o jogo
    clientMoved = MyClientMoved;
    clientConnected = MyClientConnected;
    clientConfirmed = startGame;

    init();

    while (1)
    {
        // espera alguma mensagem
        sleepServer();

        // verifando se foi uma conexão..
        checkConn();

        // se foi alguém se comunicando...
        wasClient();

        //broadcasting das modificações do mapa
        if(game_status == 2)
            broadcast();
    }
}

void MyClientConnected(int id, clientInfo startInfo)
{
    FILE *MAPFILE,*MOBSDB;
    char mapName[BUFFER_SIZE];
    char mapInfoToPlayers[BUFFER_SIZE];
    char clientInfoToPlayers[BUFFER_SIZE];
    char mobsInfo[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    //char **cacheMAP;
    int i,j;
    int posx=0,posy=0;
    upd_msg initConfigs; //variavel para enviar as infos iniciais de HP ATK DEF e Posicao inicial no mapa

    printf("Client %s connected, id = %d, map = %d\n", startInfo.nome, id, startInfo.mapa); //printa na janela do server que o cliente tal se conectou
    sprintf(clientInfoToPlayers,"Jogador %s entrou na sala com id %d", startInfo.nome, id); // cria uma string com a informacao de quem entrou na sala

    if(id==0)
    {
        //LENDO DADOS DO CLIENT 0...
        sendTxtToClient(clients[id].sockid,"Voce e o jogador 0. Aperte qualquer tecla para iniciar o jogo.\n");
        mapa=startInfo.mapa;   // que mapa o cliente 0 escolheu

        sprintf(mapName,"data/mapa%d.txt",mapa); //printa na string mapName o caminho para o arquivo do mapa
        sprintf(mobsInfo,"data/monstros%d.bin",mapa);


        sprintf(mapInfoToPlayers,"A luta acontecerá na arena %d\n",mapa); //grava uma mensagem de aviso aos players
        
        MAPFILE=fopen(mapName,"r");
        fscanf(MAPFILE,"%d %d %d", &mapaAlt,&mapaLarg,&qtdMobs);

        MOBSDB=fopen(mobsInfo,"r");

        //LENDO DADOS DO CLIENT 0...
 

        // //CARREGANDO MAPA...
        // cacheMAP=(char**)calloc(mapaAlt,sizeof(char*));
        // for(i=0;i<mapaAlt;i++)
        //     cacheMAP[i]=(char*)calloc(mapaLarg+5,sizeof(char));
        
        // i=0;

        // fgets( cacheMAP[i],mapaLarg+5 , MAPFILE);

        // while( ( fgets(cacheMAP[i] , mapaLarg+5 , MAPFILE) ) != NULL )
        // {
        //     printf("%s",cacheMAP[i]);
        //     i++;
        // }
        
        // //CARREGANDO MAPA...


        //CARREGANDO MAPA...
        cacheMAP=(char**)calloc(mapaAlt,sizeof(char*));
        for(i=0;i<mapaAlt;i++)
            cacheMAP[i]=(char*)calloc(mapaLarg+1,sizeof(char));

        for(i=0;i<mapaAlt;i++)
            for(j=0;j<mapaLarg+1;j++)   
                cacheMAP[i][j]=fgetc(MAPFILE);
        //CARREGANDO MAPA...


        //CARREGANDO MONSTROS...
        mobs=(player*)calloc(qtdMobs,sizeof(player));
        for (j = 0; j < qtdMobs; ++j) {
            fread(&mobs[j],sizeof(player),1,MOBSDB);
        }

        printf("Monsters in this map: %d\n",qtdMobs);

        for(i=0;i<qtdMobs;i++)
        {
            while(cacheMAP[posx][posy]!=' ')
            {
                posx = (rand() % (mapaAlt-2) ) + 1;
                posy = (rand() % (mapaLarg-2) ) + 1;
            }

            mobs[i].x = posx;
            mobs[i].y = posy;
            cacheMAP[posx][posy] = i+97;

            printf("Monster ID: [%d]\n",i);
            printf("Monster Name: %s\n",mobs[i].nome);
            printf("Starting Position: (%d,%d)\n",mobs[i].x,mobs[i].y);
            printf("<HP>: %d\n",mobs[i].vida);
            printf("<ATK>: %d\n",mobs[i].ataque);
            printf("<DEF>: %d\n",mobs[i].defesa);

            posx = 0;
            posy = 0;
        }

        for ( i = 0; i < qtdMobs; i++)
        {
            mobs[i].sockid=97+i;
            j=i%4;
            switch(j){
                case 0 : j='a';  
                        break;
                case 1 : j='s';  
                        break;
                case 2 : j='d';
                        break;
                case 3 : j='w';  
                        break;
            }
            mobs[i].ant=j;
        }        

        //CARREGANDO MONSTROS...


        fclose(MAPFILE);
        fclose(MOBSDB);
    }

    else
    {
        sendTxtToClient(clients[id].sockid,"Aguarde o criador do jogo iniciar a partida!\n");
        sendTxtToClient(clients[id].sockid,mapInfoToPlayers);
        broadcastTxt(clientInfoToPlayers,-1);
    }

    //POSICIONANDO JOOJADORES
    while(cacheMAP[posx][posy]!=' ')
    {
        posx = (rand() % (mapaAlt-3) ) + 2;
        posy = (rand() % (mapaLarg-3) ) + 2;
    }


    clients[id].x = posx; //posicao inicial do jogador em x
    clients[id].y = posy; //posicao inicial do jogador em y
    printf("Player %d spawned at (%d,%d)\n",id,clients[id].x,clients[id].y );
    
    cacheMAP[clients[id].x][clients[id].y] = id+48; //printa no mapa em cache a posicao do jogador
    //POSICIONANDO JOOJADORES
    
    
    //CARREGANDO CLASSES
    switch(startInfo.classe)
    {
        case 1:         //carrega status do PALADINO
            clients[id].vida = 30;
            clients[id].ataque = 3;
            clients[id].defesa = 3;
            clients[id].DEX = 3;
            clients[id].WIS = 3;
            clients[id].MaxHP = 30;
            clients[id].ant = 1;
            break;
        case 2:         //carrega status do LADRAO
            clients[id].vida = 20;
            clients[id].ataque = 3;
            clients[id].defesa = 2;
            clients[id].DEX = 6;
            clients[id].WIS = 2;
            clients[id].MaxHP = 30;
            clients[id].ant = 2;
            break;
        case 3:         //carrega status do CACADOR
            clients[id].vida = 20;
            clients[id].ataque = 6;
            clients[id].defesa = 2;
            clients[id].DEX = 4;
            clients[id].WIS = 1;
            clients[id].MaxHP = 20;
            clients[id].ant = 3;
            break;
        case 4:         //carrega status do GUERREIRO
            clients[id].vida = 40;
            clients[id].ataque = 2;
            clients[id].defesa = 6;
            clients[id].DEX = 2;
            clients[id].WIS = 1;
            clients[id].MaxHP = 40;
            clients[id].ant = 4;
            break;
        case 5:         //carrega status do MAGO
            clients[id].vida = 20;
            clients[id].ataque = 2;
            clients[id].defesa = 2;
            clients[id].DEX = 3;
            clients[id].WIS = 6;
            clients[id].MaxHP = 20;
            clients[id].ant = 5;
            break;
    }
    //CARREGANDO CLASSES

    //clients[id].vida = 50 ; //+ (rand() % 501);    //geração de status iniciais, vida, atk, def
    //clients[id].ataque = 15; //+ (rand() % 501);
    //clients[id].defesa = 5; //+ (rand() % 501);
    //CARREGANDO CLASSES

    game_status = 1;

    printCacheMap();
}

void MyClientMoved(int id, mov_msg mov)
{
    usleep(100); //verificado experimentalmente que melhora a dinâmica do jogo
    printf("Client %d moved: %c\n", id, mov.msg);
    upd_msg statUpdt;
    int x = clients[id].x, y = clients[id].y, i; //para poupar trabalho
    

    if(clients[id].fight==0) //se o client nao esta em batalha
    {

        if(mov.msg == 'd') //se a tecla apertada foi d
        {
            if(!nullMovPlayer(x,y,mov.msg)) //if(y < mapaLarg-1) //if(!nullMovPlayer(x,y))
            {
                if( isTree(x,y+1) ) //se é uma arvore na coordenada que o player vai se mover
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    lifeBonus(id , &statUpdt);
                    moveClient(0 , 1 , id); //move o player em +0x +1y
                }
                else if( isPlayer(x,y+1) )
                {
                    startBattle( id , cacheMAP[x][y+1]-48);
                    printf("(Batalha Iniciou)\n");
                }
                else if (isMonster(x, y+1) )
                {
                    startBattleMonster( id , cacheMAP[x][y+1]-97);
                    printf("(Batalha Iniciou)\n");
                }
                else
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    moveClient(0 , 1 , id); //move o player em +0x +1y
                }
            }
        }
    
        if(mov.msg == 'w')
        {
            if(!nullMovPlayer(x,y,mov.msg))
            {              
                if( isTree(x-1,y) ) //se é uma arvore na coordenada que o player vai se mover
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    lifeBonus(id , &statUpdt);
                    moveClient(-1 , 0 , id); //move o player em -1x +0y
                }
                else if( isPlayer(x-1,y) )
                {
                    startBattle( id , cacheMAP[x-1][y]-48);
                    printf("(Batalha Iniciou)\n");
                }
                else if(isMonster(x-1,y))
                {
                    startBattleMonster( id , cacheMAP[x-1][y]-97);
                    printf("(Batalha Iniciou)\n");
                }
                else
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    moveClient(-1 , 0 , id); //move o player em -1x +0y
                }
    
            }
        }
    
        if(mov.msg == 'a')
        {
            if(!nullMovPlayer(x,y,mov.msg))
            {
    
                if( isTree(x,y-1) ) //se é uma arvore na coordenada que o player vai se mover
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    lifeBonus(id , &statUpdt);
                    moveClient(0 , -1 , id); //move o player em +0x -1y
                }
                else if( isPlayer(x,y-1) )
                {
                    startBattle( id , cacheMAP[x][y-1]-48);
                    printf("(Batalha Iniciou)\n");
                }
                else if (isMonster(x, y-1))
                {
                    startBattleMonster( id , cacheMAP[x][y-1]-97);
                    printf("(Batalha Iniciou)\n");
                }    
                else
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    moveClient(0 , -1 , id); //move o player em +0x -1y
                }
            }
        }
    
        if(mov.msg == 's')
        {
            if(!nullMovPlayer(x,y,mov.msg))
            {
                if( isTree(x+1,y) ) //se é uma arvore na coordenada que o player vai se mover
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    lifeBonus(id , &statUpdt);
                    moveClient(1 , 0 , id); //move o player em +1x +0y
                }
                else if( isPlayer(x+1,y) )
                {
                    startBattle( id , cacheMAP[x+1][y]-48);
                    printf("(Batalha Iniciou)\n");
                }
                else if ( isMonster(x+1,y) )
                {
                    startBattleMonster( id , cacheMAP[x+1][y]-97);
                    printf("(Batalha Iniciou)\n");
                }
                else
                {
                    clearClientPosition(id); //limpa a posição que o player "estava"
                    moveClient(1 , 0 , id); //move o player em +0x -1y
                }
            }
        }
        printCacheMap();
    }

    else if(clients[id].fight==1 && clients[id].turn) 
    {

        if(mov.msg == 'd') //se a tecla apertada foi d, ataque basico
        {
            if(clients[id].turn)
            {
                baseAttackM(id);
                if(skillcd[id].skill>0)
                    skillcd[id].skill--;
            }
        }
    
        if(mov.msg == 'w') //se w, habilidade 1
        {
            switch(clients[id].ant) {
                case 1:
                    if(skillcd[id].skill==0)
                    {
                        DivineHealM(id);
                        skillcd[id].skill=4;
                    }
                    break;
                case 2:
                    if(skillcd[id].skill==0)
                    {
                        veneno(id);
                        skillcd[id].skill=4;
                    }
                    break;
                case 3:
                    if(skillcd[id].skill==0)
                    {
                        arapucaM(id);
                        skillcd[id].skill=3;
                    }
                    break;
                case 4:
                    if(skillcd[id].skill==0)
                    {
                        shieldSlamM(id);
                        skillcd[id].skill=2;
                    }
                    break;
                case 5:
                    if(skillcd[id].skill==0)
                    {
                        fireBallM(id);
                        skillcd[id].skill=2;
                    }
                    break;
            }
        }
    }

    else if(clients[id].fight==2 && clients[id].turn)
    {
        if(mov.msg == 'd') //se a tecla apertada foi d, ataque basico
        {
            if(clients[id].turn)
            {
                baseAttack(id);
                if(skillcd[id].skill>0)
                    skillcd[id].skill--;
            }
        }
    
        if(mov.msg == 'w') //se w, habilidade 1
        {
            switch(clients[id].ant) 
            {
                case 1:
                    if(skillcd[id].skill==0)
                    {
                        DivineHeal(id);
                        skillcd[id].skill=4;
                    }
                    break;
                case 2:
                    if(skillcd[id].skill==0)
                    {
                        veneno(id);
                        skillcd[id].skill=4;
                    }
                    break;
                case 3:
                    if(skillcd[id].skill==0)
                    {
                        arapuca(id);
                        skillcd[id].skill=3;
                    }
                    break;
                case 4:
                    if(skillcd[id].skill==0)
                    {
                        shieldSlam(id);
                        skillcd[id].skill=2;
                    }
                    break;
                case 5:
                    if(skillcd[id].skill==0)
                    {
                        fireBall(id);
                        skillcd[id].skill=2;
                    }
                    break;      
            }
        }
        
    }
    for (i=0;i<qtdMobs;i++)
    {
        if(mobs[i].fight == 0 && mobs[i].vida > 0)
            moveMonster(i);
    }

    checkIfWinner(id);
}

void startGame()
{
    int i=0;
    char msg_start[BUFFER_SIZE] = "+ Jogo Iniciando\n";
    char msg_mapa[BUFFER_SIZE];
    char msg_openmap[BUFFER_SIZE];
    upd_msg initConfigs;

    sprintf(msg_openmap,"data/mapa%d.txt",mapa);
    sprintf(msg_mapa,"Mapa da Partida: %d\n", mapa);

    printf("Broadcasting Map %d to players. There are currently %d players in this match.\n",mapa,clients_connected);   
    
    broadcastTxt(msg_mapa,-1);
    broadcastTxt(msg_start,-1);
    broadcastTxt(msg_openmap,-1);

    printf("Client 0 confirmed, the game will start now...\n");

    //broadcast das INFOS INICIAIS DE CADA JOGADOR
    int id=0, sd=0;
    for(id = 0; id < MAX_CLIENTS; ++id)
    {
        sd = clients[id].sockid;
        if(sd > 0)
        {
            //envio da posição inicial dos jogadores
            initConfigs.tipo=0;
            initConfigs.x = clients[id].x;
            initConfigs.y = clients[id].y;
            initConfigs.new = id+48;

            for(i=0;i<MAX_CLIENTS;i++) //todas as posicoes para todos os players
                sendUpdToClient(clients[i].sockid, initConfigs);

            cacheMAP[clients[id].x][clients[id].y]=id+48; //atualização da posição em cache no servidor

            //envio dos STATUS iniciais dos jogadores
            initConfigs.tipo=2;
            initConfigs.vida = clients[id].vida;
            initConfigs.x = clients[id].ataque;
            initConfigs.y = clients[id].defesa;
            initConfigs.newDex = clients[id].DEX;
            initConfigs.newWis = clients[id].WIS;
            initConfigs.newMaxHP = clients[id].MaxHP;
            sendUpdToClient(clients[id].sockid, initConfigs); 

        }
    }
    //broadcast DAS INFOS INICIAIS DE CADA JOGADOR

    //envio da posição inicial dos monstros para cada jogador
        for(i=0;i<qtdMobs;i++)
        {
            if(i==qtdMobs-1)
            {
                initConfigs.tipo=0;
                initConfigs.x = mobs[i].x;
                initConfigs.y = mobs[i].y;
                initConfigs.new = 'M';
                cacheMAP[mobs[i].x][mobs[i].y]='M';
            }
            else
            {
                initConfigs.tipo=0;
                initConfigs.x = mobs[i].x;
                initConfigs.y = mobs[i].y;
                initConfigs.new = 'm';
                cacheMAP[mobs[i].x][mobs[i].y]='m';
            }

            for(id=0;id<MAX_CLIENTS;id++) //ŧodas as posicoes de todos os monstros para todos os players
                sendUpdToClient(clients[id].sockid, initConfigs);
        }

    // avisar para os clientes que o jogo vai começar
    // enviar algumas informações, como mapa, status inicial do cliente, etc...

    game_status=2;
}

void MyBroadcast()
{
    int id, sd;
    for(id = 0; id < MAX_CLIENTS; ++id)
    {
        sd = clients[id].sockid;
        if(sd > 0)
        {
            sendTxtToClient(sd,"teste");
        }
    }
}

void printCacheMap(void)
{
    int i=0,j=0;

    for(i=0;i<mapaAlt;i++)
        for(j=0;j<mapaLarg+1;j++)   
            printf("%c",cacheMAP[i][j]);

    //for(i=0;i<mapaAlt;i++) 
        //printf("%s",cacheMAP[i]);

    printf("\n");
}

void clearClientPosition(int id)
{
    map_changes[pos_broad].tipo = 0;
    map_changes[pos_broad].y = clients[id].y;
    map_changes[pos_broad].x = clients[id].x;
    map_changes[pos_broad++].new = ' ';

    usleep(150);

    cacheMAP[clients[id].x][clients[id].y]=' ';//atualiza o mapa em cache
}
void moveClient(int Newx, int Newy, int id)
{
    map_changes[pos_broad].tipo = 0;
    map_changes[pos_broad].x = clients[id].x+Newx;
    map_changes[pos_broad].y = clients[id].y+Newy;
    map_changes[pos_broad++].new = id+48;
    
    clients[id].x+=Newx;
    clients[id].y+=Newy;

    usleep(150);

    cacheMAP[clients[id].x][clients[id].y]=id+48;//atualiza o mapa em cache
}

int isTree(int x, int y)
{
    int tree=0;

    if(cacheMAP[x][y]=='*')
    {
        //cacheMAP[x][y]=' ';
        tree=1;
    }

    return tree;
}

int isPlayer(int x, int y)
{
    int player=0;
    //int id=0;

    if(cacheMAP[x][y]>='0' && cacheMAP[x][y]<='5')
    {   
        //id=cacheMAP[x][y]-48;
        player=1;
    }

    return player;
}

void isDead(int op_id, int id)
{
    KILL(op_id);
    victory(id);
}

void isDeadPvE(int player_id, int mob_id)
{
    KILL(player_id);
    mobs[mob_id].fight = 0;
}

void monsterIsDead(int mon_id, int id)
{
    KILLMONSTER(mon_id);
    victory(id);
}

void KILL( int op_id )
{
    upd_msg msg;

    clients[op_id].vida = 0;
    clients[op_id].ataque = 0;
    clients[op_id].defesa = 0;
    clients[op_id].turn = 0;
    clients[op_id].fight = 0;
    clients[op_id].whofight = 0;

    msg.tipo = 4;                               //(TIPO 4 - PERDEU, DED, REKT)

    clearClientPosition(op_id);

    sendUpdToClient(clients[op_id].sockid, msg);

    if(clients_connected == 1)
    {
    	mobsKilled=0;
        pos_broad = 0;
        game_status = 0;
    }

    disconnectClient(op_id);

}

void KILLMONSTER(int mon_id)
{
    mobs[mon_id].vida = 0;
    mobs[mon_id].ataque = 0;
    mobs[mon_id].defesa = 0;
    mobs[mon_id].turn = 0;
    mobs[mon_id].fight = 0;
    mobs[mon_id].whofight = 0;

    clearMonsterPosition(mon_id);

    mobsKilled++;
}

void victory( int id )
{
    upd_msg msg;                                    //variavel pra mandar o update
    
    msg.tipo = 3;                                   //(TIPO 3 - SAIU DE BATALHA)

    sendUpdToClient(clients[id].sockid, msg);       //manda o update tipo 3

    clients[id].turn = 0;
    clients[id].fight = 0;
    clients[id].whofight = 0;

    subirDeNivel(id);

    msg.tipo = 2;                                  //envia atualizacao de status pro vencedor (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    msg.vida = clients[id].vida;                   //com a propria vida
    msg.x = clients[id].ataque;                    //ataque
    msg.y = clients[id].defesa;                    //defesa
    msg.newDex = clients[id].DEX;
    msg.newWis = clients[id].WIS;
    msg.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, msg);      //envia as atualizações de status 

}

int nullMovPlayer(int x, int y, char mov)
{
    int FORBIDDEN=0;

    if(mov=='w' && x <= 1)
        FORBIDDEN=1;

    if(mov=='a' && y <= 2)
        FORBIDDEN=1;

    if(mov=='s' && x >= mapaAlt-2)
        FORBIDDEN=1;

    if(mov=='d' && y >= mapaLarg-1)
        FORBIDDEN=1;

    return FORBIDDEN;
}

void lifeBonus(int id , upd_msg *statUpdt)
{
    int overHeal=0;
    overHeal=clients[id].MaxHP/2;
    
    if(clients[id].vida <= (clients[id].MaxHP + overHeal))
        clients[id].vida+=10; //aumenta a vida do client em 10 pontos
    
    statUpdt->tipo = 2;
    statUpdt->vida = clients[id].vida;
    statUpdt->x = clients[id].ataque;
    statUpdt->y = clients[id].defesa;
    statUpdt->newDex = clients[id].DEX;
    statUpdt->newWis = clients[id].WIS;
    statUpdt->newMaxHP = clients[id].MaxHP;

    sendUpdToClient(clients[id].sockid, *statUpdt);

    usleep(150);
}


void startBattle(int idPlayer1, int idPlayer2)
{
    upd_msg P1,P2;

    if(clients[idPlayer2].fight==0)
    {   
        clients[idPlayer1].fight=2; //para nao enviar/receber mais map_updts e entrar no modo de luta
        clients[idPlayer2].fight=2;
    
        clients[idPlayer1].whofight = idPlayer2; //vincula os oponentes.
        clients[idPlayer2].whofight = idPlayer1;
    
        clients[idPlayer1].turn=1; //jogador que iniciou a batalha começa atacando.
        clients[idPlayer2].turn=0;
    
        P1.tipo = 1; //tipo do updt
        P1.vida = clients[idPlayer1].vida;
        P1.x = clients[idPlayer1].ataque;
        P1.y = clients[idPlayer1].defesa;
    
        P2.tipo = 1;
        P2.vida = clients[idPlayer2].vida;
        P2.x = clients[idPlayer2].ataque;
        P2.y = clients[idPlayer2].defesa;
    
        sendUpdToClient(clients[idPlayer1].sockid, P2);
        sendUpdToClient(clients[idPlayer2].sockid, P1);

        usleep(150);
    }
}

void startBattleMonster(int id_jog, int id_mon)
{
    upd_msg monster;

    if(mobs[id_mon].fight==0)
    {   
        clients[id_jog].fight=1; //para nao enviar/receber mais map_updts e entrar no modo de luta
        mobs[id_mon].fight=1;
    
        clients[id_jog].whofight = id_mon; //vincula os oponentes.
        mobs[id_mon].whofight = id_jog;
    
        clients[id_jog].turn=1; //jogador que iniciou a batalha começa atacando.

        monster.tipo = 1;
        monster.vida = mobs[id_mon].vida;
        monster.x = mobs[id_mon].ataque;
        monster.y = mobs[id_mon].defesa;

        sendUpdToClient(clients[id_jog].sockid, monster);

        usleep(150);
    }
}



void baseAttack(int id)
{
    upd_msg oponente,atacante;                         //mensagens de update pro oponente e pro atacante.
    int OID = clients[id].whofight;                    //id do oponente. OID = Oponnent ID. Logo, a ID do oponente é a ID de quem está lutando contra quem está atacando (whofight)
    int dano=0,modifier=0, crit=1, esquiv=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    if(1 + (rand()%100) <= ((2*clients[id].DEX)/5)*10) 
        crit=2;

    if (1 + (rand()%100) <= ((3*clients[OID].DEX)/5)*10) 
        esquiv = 0;
    
    if(contadorDeVeneno[id] > 0) {
        poison = 2;
        contadorDeVeneno[id] --;
    }


    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = ((clients[id].ataque - clients[OID].defesa) + modifier)*crit*esquiv*poison;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[OID].vida -= dano;                          //reduz a vida do seu oponente do banco de dados de players.

    if(clients[OID].vida < 0)                           //não permite que a vida seja negativa
        clients[OID].vida = 0;                          //zera a vida

    oponente.tipo = 2;                                  //envia atualizacao de status pro oponente (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    oponente.vida = clients[OID].vida;                  //com a nova vida apos o dano
    oponente.x = clients[OID].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    oponente.y = clients[OID].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    oponente.newDex = clients[id].DEX;
    oponente.newWis = clients[id].WIS;
    oponente.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[OID].sockid, oponente);     //envia as atualizações pro oponente.

    atacante.tipo = 2;                                  //envia atualizacao de status pro atacante (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    atacante.vida = clients[id].vida;                   //com a propria vida
    atacante.x = clients[id].ataque;                    //ataque
    atacante.y = clients[id].defesa;                    //defesa
    atacante.newDex = clients[id].DEX;
    atacante.newWis = clients[id].WIS;
    atacante.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, atacante);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    oponente.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[id].sockid, oponente);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do OPONENTE na tela de quem atacou.

    atacante.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[OID].sockid, atacante);     //envia atualizacao de BATALHA a quem foi atacado, para atualizar os status do ATACANTE na tela de quem levou o dano.

    if (dano > 0)
        sprintf(hitmsg, "\t\tOuch! Took %d damage!\n", dano);   //grava mensagem de dano na string hitmsg
    else if (esquiv == 0)
        sprintf(hitmsg, "\t\tYou dodged from the attack\n");
    else
        sprintf(hitmsg, "\t\tYou took no damage\n");
    sendTxtToClient(clients[OID].sockid, hitmsg);           //envia mensagem de dano pro oponente q levou dano


    if (dano > 0 && crit == 1)
        sprintf(atkmsg, "\t\tCRIT! Dealt %d damage!\n", dano); 
    else if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
        sprintf(atkmsg, "\t\tYou missed\n");
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    clients[id].turn = 0;                               //troca o turno entre oponente e atacante.
    clients[OID].turn = 1;                              //troca o turno entre oponente e atacante.

    if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id);                                 //caso positivo, chama a função.
}

void baseAttackM(int id)
{
    upd_msg monstro,jogador;                         //mensagens de update pro monstro e pro jogador.
    int OID = clients[id].whofight;                    //id do monstro. OID = Oponnent ID. Logo, a ID do monstro é a ID de quem está lutando contra quem está atacando (whofight)
    int dano,modifier, crit=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    if(1 + (rand()%100) <= ((2*clients[id].DEX)/5)*10)
        crit=2;

    if(contadorDeVeneno[id] > 0) {
        poison = 2;
        contadorDeVeneno[id] --;
    }


    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = ((clients[id].ataque - mobs[OID].defesa) + modifier)*2*crit*poison;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    mobs[OID].vida -= dano;                          //reduz a vida do seu monstro do banco de dados de players.

    if(mobs[OID].vida < 0)                           //não permite que a vida seja negativa
        mobs[OID].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[id].vida;                   //com a propria vida
    jogador.x = clients[id].ataque;                    //ataque
    jogador.y = clients[id].defesa;                    //defesa
    jogador.newDex = clients[id].DEX;
    jogador.newWis = clients[id].WIS;
    jogador.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)
 
    monstro.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    monstro.vida = mobs[OID].vida;
    monstro.x = mobs[OID].ataque;
    monstro.y = mobs[OID].defesa;    
    sendUpdToClient(clients[id].sockid, monstro);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do monstro na tela de quem atacou.

    if (dano > 0 && crit == 1)
        sprintf(atkmsg, "\t\tCRIT! Dealt %d damage!\n", dano); 
    else if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
        sprintf(atkmsg, "\t\tYou missed\n");

    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
        monsterIsDead(OID,id);
    else
        monsterAttack(id, OID);                                 
}

void fireBallM(int id)
{
    upd_msg monstro,jogador;                         //mensagens de update pro monstro e pro jogador.
    int OID = clients[id].whofight;                    //id do monstro. OID = Oponnent ID. Logo, a ID do monstro é a ID de quem está lutando contra quem está atacando (whofight)
    int dano,modifier;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (clients[id].WIS) + modifier;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    mobs[OID].vida -= dano;                          //reduz a vida do seu monstro do banco de dados de players.

    if(mobs[OID].vida < 0)                           //não permite que a vida seja negativa
        mobs[OID].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[id].vida;                   //com a propria vida
    jogador.x = clients[id].ataque;                    //ataque
    jogador.y = clients[id].defesa;                    //defesa
    jogador.newDex = clients[id].DEX;
    jogador.newWis = clients[id].WIS;
    jogador.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)
 
    monstro.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    monstro.vida = mobs[OID].vida;
    monstro.x = mobs[OID].ataque;
    monstro.y = mobs[OID].defesa;    
    sendUpdToClient(clients[id].sockid, monstro);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do monstro na tela de quem atacou.

    if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Fireball dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
      sprintf(atkmsg, "\t\tYour fireball missed");  
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
        monsterIsDead(OID,id);
    else
        monsterAttack(id, OID);                                 
}

void fireBall(int id) {
    upd_msg oponente,atacante;                         //mensagens de update pro oponente e pro atacante.
    int OID = clients[id].whofight;                    //id do oponente. OID = Oponnent ID. Logo, a ID do oponente é a ID de quem está lutando contra quem está atacando (whofight)
    int dano=0,modifier=0, crit=1, esquiv=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (clients[id].WIS) + modifier;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[OID].vida -= dano;                          //reduz a vida do seu oponente do banco de dados de players.

    if(clients[OID].vida < 0)                           //não permite que a vida seja negativa
        clients[OID].vida = 0;                          //zera a vida

    oponente.tipo = 2;                                  //envia atualizacao de status pro oponente (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    oponente.vida = clients[OID].vida;                  //com a nova vida apos o dano
    oponente.x = clients[OID].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    oponente.y = clients[OID].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    oponente.newDex = clients[id].DEX;
    oponente.newWis = clients[id].WIS;
    oponente.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[OID].sockid, oponente);     //envia as atualizações pro oponente.

    atacante.tipo = 2;                                  //envia atualizacao de status pro atacante (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    atacante.vida = clients[id].vida;                   //com a propria vida
    atacante.x = clients[id].ataque;                    //ataque
    atacante.y = clients[id].defesa;                    //defesa
    atacante.newDex = clients[id].DEX;
    atacante.newWis = clients[id].WIS;
    atacante.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, atacante);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    oponente.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[id].sockid, oponente);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do OPONENTE na tela de quem atacou.

    atacante.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[OID].sockid, atacante);     //envia atualizacao de BATALHA a quem foi atacado, para atualizar os status do ATACANTE na tela de quem levou o dano.

    if (dano > 0)
        sprintf(hitmsg, "\t\tOuch! Took %d damage!\n", dano);   //grava mensagem de dano na string hitmsg
    else if (esquiv == 0)
        sprintf(hitmsg, "\t\tYou dodged from the attack\n");
    else
        sprintf(hitmsg, "\t\tYou took no damage\n");
    sendTxtToClient(clients[OID].sockid, hitmsg);           //envia mensagem de dano pro oponente q levou dano


    if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Fireball dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
      sprintf(atkmsg, "\t\tYour fireball missed");  
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    clients[id].turn = 0;                               //troca o turno entre oponente e atacante.
    clients[OID].turn = 1;                              //troca o turno entre oponente e atacante.

    if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id);                                 //caso positivo, chama a função.
}

void shieldSlamM(int id)
{
    upd_msg monstro,jogador;                         //mensagens de update pro monstro e pro jogador.
    int OID = clients[id].whofight;                    //id do monstro. OID = Oponnent ID. Logo, a ID do monstro é a ID de quem está lutando contra quem está atacando (whofight)
    int dano,modifier;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (clients[id].defesa + clients[id].DEX) + modifier;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    mobs[OID].vida -= dano;                          //reduz a vida do seu monstro do banco de dados de players.

    if(mobs[OID].vida < 0)                           //não permite que a vida seja negativa
        mobs[OID].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[id].vida;                   //com a propria vida
    jogador.x = clients[id].ataque;                    //ataque
    jogador.y = clients[id].defesa;                    //defesa
    jogador.newDex = clients[id].DEX;
    jogador.newWis = clients[id].WIS;
    jogador.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)
 
    monstro.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    monstro.vida = mobs[OID].vida;
    monstro.x = mobs[OID].ataque;
    monstro.y = mobs[OID].defesa;    
    sendUpdToClient(clients[id].sockid, monstro);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do monstro na tela de quem atacou.

    if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Shield Slam dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
      sprintf(atkmsg, "\t\tYour shield slam missed");  
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
        monsterIsDead(OID,id);
    else
        monsterAttack(id, OID);                                 
}

void shieldSlam(int id) {
    upd_msg oponente,atacante;                         //mensagens de update pro oponente e pro atacante.
    int OID = clients[id].whofight;                    //id do oponente. OID = Oponnent ID. Logo, a ID do oponente é a ID de quem está lutando contra quem está atacando (whofight)
    int dano=0,modifier=0, crit=1, esquiv=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (clients[id].defesa + clients[id].DEX) + modifier;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[OID].vida -= dano;                          //reduz a vida do seu oponente do banco de dados de players.

    if(clients[OID].vida < 0)                           //não permite que a vida seja negativa
        clients[OID].vida = 0;                          //zera a vida

    oponente.tipo = 2;                                  //envia atualizacao de status pro oponente (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    oponente.vida = clients[OID].vida;                  //com a nova vida apos o dano
    oponente.x = clients[OID].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    oponente.y = clients[OID].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    oponente.newDex = clients[id].DEX;
    oponente.newWis = clients[id].WIS;
    oponente.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[OID].sockid, oponente);     //envia as atualizações pro oponente.

    atacante.tipo = 2;                                  //envia atualizacao de status pro atacante (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    atacante.vida = clients[id].vida;                   //com a propria vida
    atacante.x = clients[id].ataque;                    //ataque
    atacante.y = clients[id].defesa;                    //defesa
    atacante.newDex = clients[id].DEX;
    atacante.newWis = clients[id].WIS;
    atacante.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, atacante);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    oponente.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[id].sockid, oponente);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do OPONENTE na tela de quem atacou.

    atacante.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[OID].sockid, atacante);     //envia atualizacao de BATALHA a quem foi atacado, para atualizar os status do ATACANTE na tela de quem levou o dano.

    if (dano > 0)
        sprintf(hitmsg, "\t\tOuch! Took %d damage!\n", dano);   //grava mensagem de dano na string hitmsg
    else if (esquiv == 0)
        sprintf(hitmsg, "\t\tYou dodged from the attack\n");
    else
        sprintf(hitmsg, "\t\tYou took no damage\n");
    sendTxtToClient(clients[OID].sockid, hitmsg);           //envia mensagem de dano pro oponente q levou dano


    if (dano > 0)
        sprintf(atkmsg, "\t\tHit! Shield Slam dealt %d damage!\n", dano);   //grava mensagem de ataque na string atkmsg
    else
      sprintf(atkmsg, "\t\tYour shield slam missed");  
    sendTxtToClient(clients[id].sockid, atkmsg);       //envia mensagem de ataque pra quem atacou.

    clients[id].turn = 0;                               //troca o turno entre oponente e atacante.
    clients[OID].turn = 1;                              //troca o turno entre oponente e atacante.

    if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id); 
}

void DivineHealM(int id) {
    upd_msg monstro,jogador;                         //mensagens de update pro monstro e pro jogador.
    int OID = clients[id].whofight;                    //id do monstro. OID = Oponnent ID. Logo, a ID do monstro é a ID de quem está lutando contra quem está atacando (whofight)
    int dano;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      

    dano = clients[id].WIS*5;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[id].vida += dano;                          //reduz a vida do seu monstro do banco de dados de players.

    if(mobs[OID].vida < 0)                           //não permite que a vida seja negativa
        mobs[OID].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[id].vida;                   //com a propria vida
    jogador.x = clients[id].ataque;                    //ataque
    jogador.y = clients[id].defesa;                    //defesa
    jogador.newDex = clients[id].DEX;
    jogador.newWis = clients[id].WIS;
    jogador.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)
 
    monstro.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    monstro.vida = mobs[OID].vida;
    monstro.x = mobs[OID].ataque;
    monstro.y = mobs[OID].defesa;    
    sendUpdToClient(clients[id].sockid, monstro);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do monstro na tela de quem atacou.

    sprintf(atkmsg, "\t\tThe gods healed you in %d points of life!\n", dano); 
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
        monsterIsDead(OID,id);
    else
        monsterAttack(id, OID); 
}

void DivineHeal(int id) {
    upd_msg oponente,atacante;                         //mensagens de update pro oponente e pro atacante.
    int OID = clients[id].whofight;                    //id do oponente. OID = Oponnent ID. Logo, a ID do oponente é a ID de quem está lutando contra quem está atacando (whofight)
    int dano=0,modifier=0, crit=1, esquiv=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

    dano = clients[id].WIS*5;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[id].vida += dano;                          //reduz a vida do seu oponente do banco de dados de players.

    if(clients[OID].vida < 0)                           //não permite que a vida seja negativa
        clients[OID].vida = 0;                          //zera a vida

    oponente.tipo = 2;                                  //envia atualizacao de status pro oponente (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    oponente.vida = clients[OID].vida;                  //com a nova vida apos o dano
    oponente.x = clients[OID].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    oponente.y = clients[OID].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    oponente.newDex = clients[id].DEX;
    oponente.newWis = clients[id].WIS;
    oponente.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[OID].sockid, oponente);     //envia as atualizações pro oponente.

    atacante.tipo = 2;                                  //envia atualizacao de status pro atacante (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    atacante.vida = clients[id].vida;                   //com a propria vida
    atacante.x = clients[id].ataque;                    //ataque
    atacante.y = clients[id].defesa;                    //defesa
    atacante.newDex = clients[id].DEX;
    atacante.newWis = clients[id].WIS;
    atacante.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, atacante);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    oponente.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[id].sockid, oponente);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do OPONENTE na tela de quem atacou.

    atacante.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[OID].sockid, atacante);     //envia atualizacao de BATALHA a quem foi atacado, para atualizar os status do ATACANTE na tela de quem levou o dano.


    sprintf(hitmsg, "\t\tThe gods healed the paladin in %d points!\n", dano);   //grava mensagem de dano na string hitmsg
    sendTxtToClient(clients[OID].sockid, hitmsg);           //envia mensagem de dano pro oponente q levou dano


    sprintf(atkmsg, "\t\tThe gods healed you in %d points of life!\n", dano); 
    sendTxtToClient(clients[id].sockid, atkmsg);            //envia mensagem de ataque pra quem atacou.

    clients[id].turn = 0;                               //troca o turno entre oponente e atacante.
    clients[OID].turn = 1;                              //troca o turno entre oponente e atacante.

    if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id);         
}

void veneno(int id) 
{
    contadorDeVeneno[id]=2;
    sendTxtToClient(clients[id].sockid, "\t\tYou poison your weapon!");
}

void arapucaM (int id) {
    upd_msg monstro,jogador;                         //mensagens de update pro monstro e pro jogador.
    int OID = clients[id].whofight;                    //id do monstro. OID = Oponnent ID. Logo, a ID do monstro é a ID de quem está lutando contra quem está atacando (whofight)
    int dano,modifier;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO


    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (((clients[id].ataque/2) - mobs[OID].defesa) + modifier)*2;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    mobs[OID].vida -= dano;                          //reduz a vida do seu monstro do banco de dados de players.

    if(mobs[OID].vida < 0)                           //não permite que a vida seja negativa
        mobs[OID].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[id].vida;                   //com a propria vida
    jogador.x = clients[id].ataque;                    //ataque
    jogador.y = clients[id].defesa;                    //defesa
    jogador.newDex = clients[id].DEX;
    jogador.newWis = clients[id].WIS;
    jogador.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)
 
    monstro.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    monstro.vida = mobs[OID].vida;
    monstro.x = mobs[OID].ataque;
    monstro.y = mobs[OID].defesa;    
    sendUpdToClient(clients[id].sockid, monstro);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do monstro na tela de quem atacou.


         //envia mensagem de ataque pra quem atacou.

    if  (1 + (rand()%100) <= (clients[id].ataque/2)*10)  {
        sprintf(atkmsg, "\t\tThe opponent fall into the trap and has taken %d damage!\n", dano);
        sendTxtToClient(clients[id].sockid, atkmsg);   
        if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
            monsterIsDead(OID,id);
    } else {
        sprintf(atkmsg, "\t\tThe opponent doesn't fall into the trap but has taken %d damage!\n", dano);
        sendTxtToClient(clients[id].sockid, atkmsg);   
        if(mobs[OID].vida == 0)                          //verifica se a vida do monstro chegou a zero
            monsterIsDead(OID,id);
        else
            monsterAttack(id, OID);  
    }

    
}

void arapuca(int id) {
    upd_msg oponente,atacante;                         //mensagens de update pro oponente e pro atacante.
    int OID = clients[id].whofight;                    //id do oponente. OID = Oponnent ID. Logo, a ID do oponente é a ID de quem está lutando contra quem está atacando (whofight)
    int dano=0,modifier=0, crit=1, esquiv=1, poison=1;                                 //para calculo do dano causado com o ataque base 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];      //mensagens de HIT e DANO

   modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = (((clients[id].ataque/2) - clients[OID].defesa) + modifier)*2;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[OID].vida -= dano;                          //reduz a vida do seu oponente do banco de dados de players.

    if(clients[OID].vida < 0)                           //não permite que a vida seja negativa
        clients[OID].vida = 0;                          //zera a vida

    oponente.tipo = 2;                                  //envia atualizacao de status pro oponente (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    oponente.vida = clients[OID].vida;                  //com a nova vida apos o dano
    oponente.x = clients[OID].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    oponente.y = clients[OID].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    oponente.newDex = clients[id].DEX;
    oponente.newWis = clients[id].WIS;
    oponente.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[OID].sockid, oponente);     //envia as atualizações pro oponente.

    atacante.tipo = 2;                                  //envia atualizacao de status pro atacante (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    atacante.vida = clients[id].vida;                   //com a propria vida
    atacante.x = clients[id].ataque;                    //ataque
    atacante.y = clients[id].defesa;                    //defesa
    atacante.newDex = clients[id].DEX;
    atacante.newWis = clients[id].WIS;
    atacante.newMaxHP = clients[id].MaxHP;
    sendUpdToClient(clients[id].sockid, atacante);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    oponente.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[id].sockid, oponente);      //envia atualizacao de BATALHA a quem realizou o ataque, para atualizar os status do OPONENTE na tela de quem atacou.

    atacante.tipo = 1;                                  //(TIPO 1 - ATUALIZAÇÃO DE BATALHA)
    sendUpdToClient(clients[OID].sockid, atacante);     //envia atualizacao de BATALHA a quem foi atacado, para atualizar os status do ATACANTE na tela de quem levou o dano.

    if  (1 + (rand()%100) <= (clients[id].ataque/2)*10)  {
        sprintf(atkmsg, "\t\tThe opponent fall into the trap and has taken %d damage!\n", dano);
        sendTxtToClient(clients[id].sockid, atkmsg);
        sprintf(hitmsg, "\t\tOuch! Took %d damage and has bem trapped!!\n", dano);
        sendTxtToClient(clients[OID].sockid, hitmsg); 
        if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id); 
        
    } else {
        sprintf(atkmsg, "\t\tThe opponent doesn't fall into the trap but has taken %d damage!\n", dano);
        sendTxtToClient(clients[id].sockid, atkmsg);
        sprintf(hitmsg, "\t\tOuch! Took %d damage!\n", dano);
        sendTxtToClient(clients[OID].sockid, hitmsg); 
        clients[id].turn = 0;                               //troca o turno entre oponente e atacante.
        clients[OID].turn = 1;   
        if(clients[OID].vida == 0)                          //verifica se a vida do oponente chegou a zero
        isDead(OID,id);
        } 
}





void monsterAttack(int jog_id, int mon_id)
{
    int modifier,dano, esquiv=1;
    upd_msg monstro,jogador; 
    char atkmsg[BUFFER_SIZE],hitmsg[BUFFER_SIZE];

    if (1 + (rand()%100) <= ((3*clients[jog_id].DEX)/5)*10) {
        esquiv = 0;
    }

    modifier = -10 + (rand() % 21);                      //calcula o modificador de dano (que pode ser de -10 a +10)
    dano = ((mobs[mon_id].ataque - clients[jog_id].defesa) + modifier)*esquiv;   //calcula o dano -> DANO = (ATK_P1 - DEF_P2) + modifier

    if(dano < 0)                                       //se o dano for menor que zero;
        dano = 0;                                      //iguala o dano a zero.

    clients[jog_id].vida -= dano;                          //reduz a vida do seu jogador do banco de dados de players.

    if(clients[jog_id].vida < 0)                           //não permite que a vida seja negativa
        clients[jog_id].vida = 0;                          //zera a vida

    jogador.tipo = 2;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    jogador.vida = clients[jog_id].vida;                   //com a propria vida
    jogador.x = clients[jog_id].ataque;                    //ataque
    jogador.y = clients[jog_id].defesa;                    //defesa
    jogador.newDex = clients[jog_id].DEX;
    jogador.newWis = clients[jog_id].WIS;
    jogador.newMaxHP = clients[jog_id].MaxHP;
    sendUpdToClient(clients[jog_id].sockid, jogador);      //envia as atualizações de status (isso abre oportunidade para implementar ataques que possam eventualmente custar vida ou refletir dano.)

    monstro.tipo = 1;                                  //envia atualizacao de status pro jogador (TIPO 2 - ATUALIZACAO DE STATUS DO JOGADOR)
    monstro.vida = mobs[mon_id].vida;                  //com a nova vida apos o dano
    monstro.x = mobs[mon_id].ataque;                   //envia novamente o ataque (mesmo inalterado, senão vai lixo de memória.)
    monstro.y = mobs[mon_id].defesa;                   //envia novamente a defesa (mesmo inalterada, senão vai lixo de memória.)
    sendUpdToClient(clients[jog_id].sockid, monstro);     //envia as atualizações pro jogador.


    if (dano > 0)
        sprintf(hitmsg, "\t\tOuch! Took %d damage!\n", dano);   //grava mensagem de dano na string hitmsg
    else if (esquiv == 0)
        sprintf(hitmsg, "\t\tYou dodged from the attack\n");
    else
        sprintf(hitmsg, "\t\tYou took no damage\n");
    sendTxtToClient(clients[jog_id].sockid, hitmsg);           //envia mensagem de dano pro jogador q levou dano

    if(clients[jog_id].vida == 0)                          //verifica se a vida do jogador chegou a zero
        isDeadPvE(jog_id,mon_id);       



    //caso positivo, chama a função.
}



char movMonster(char PosMovAnt)
{
    char v1, v2, PosCont, newPos, n;    
    switch(PosMovAnt)
    {
        case 'w' :        
                    PosCont='s';
                    v1='a';
                    v2='d';
                    break;
        case 's' :        
                    PosCont='w';
                    v1='d';
                    v2='a';
                    break;
        case 'a' :        
                    PosCont='d';
                    v1='s';
                    v2='w';
                    break;
        case 'd' :       
                    PosCont='a';
                    v1='w';
                    v2='s';
                    break;
    }
    
    n = (rand() % 30);  
    newPos=PosMovAnt; 
    if(n>=0 && n<=3)
        newPos=PosCont;
    if(n>3 && n<=10)
        newPos=PosMovAnt;
    if(n>10 && n<=15)
        newPos=v1;
    if(n>15 && n<=20)
        newPos=v2;

    return newPos;
}
int isMonster(int x, int y)
{
    int monster=0;
    //int id;
    if((cacheMAP[x][y]>= 97 && cacheMAP[x][y]<= 122) || cacheMAP[x][y]==77 )
    {        
        //id=cacheMAP[x][y]-48;
        monster=1;
    }
    return monster;
}
void clearMonsterPosition(int id)
{
    map_changes[pos_broad].tipo = 0;
    map_changes[pos_broad].y = mobs[id].y;
    map_changes[pos_broad].x = mobs[id].x;
    map_changes[pos_broad++].new = ' ';

    usleep(150);

    cacheMAP[mobs[id].x][mobs[id].y]=' ';//atualiza o mapa em cache
}
void moveMonster(int id)
{
    char mov = movMonster(mobs[id].ant);
    int Newx, Newy, x=mobs[id].x, y=mobs[id].y;    
    mobs[id].ant=mov;
    //printf("----%c----%d\n", mov, id);
    switch(mov)
    {
        case 'a':
                if(cacheMAP[x-1][y]==' ')
                    Newx=-1;
                else
                    Newx=0;
                Newy= 0;                               
                break;
        case 's':
                if(cacheMAP[x][y+1]==' ')
                    Newy= 1;
                else
                    Newy=0;
                Newx=0;                 
                 break;
        case 'w':
                if(cacheMAP[x][y-1]==' ')
                    Newy= -1;
                else
                    Newy=0;
                Newx= 0;                    
                break;
        case 'd':
                if(cacheMAP[x+1][y]==' ')
                    Newx= 1;
                else
                    Newx=0;
                Newy= 0;                   
                break;
    }
    clearMonsterPosition(id);

    if(id==qtdMobs-1)
    {
        map_changes[pos_broad].tipo = 0;
        map_changes[pos_broad].x = mobs[id].x + Newx;
        map_changes[pos_broad].y = mobs[id].y + Newy;
        map_changes[pos_broad++].new = 'M';
    }
    else
    {
        map_changes[pos_broad].tipo = 0;
        map_changes[pos_broad].x = mobs[id].x + Newx;
        map_changes[pos_broad].y = mobs[id].y + Newy;
        map_changes[pos_broad++].new = 'm';
    }
       
    mobs[id].x+=Newx;
    mobs[id].y+=Newy;

    cacheMAP[mobs[id].x][mobs[id].y]=id+97;//atualiza o mapa em cache
    //printCacheMap();
    usleep(150);
}

void checkIfWinner(int id)
{

    upd_msg winner;
    int i=0;

    printf("Monstros Mortos: %d\n Clientes Conectados: %d\n",mobsKilled,clients_connected);

    if( (mobsKilled == qtdMobs) && clients_connected==1)
    {
        winner.tipo = 5;

        sendUpdToClient(clients[id].sockid , winner);

        
        for(i=0;i<mapaAlt;i++)
            free(cacheMAP[i]);

        free(cacheMAP);
        free(mobs);
    
        disconnectClient(id);

        mobsKilled=0;
        pos_broad = 0;
        game_status = 0;

    }

}


void subirDeNivel(int id) {
    switch (clients[id].ant) {
        case 1:
            if (clients[id].ataque == 3) {
                clients[id].MaxHP = 40;
                clients[id].ataque = 4;
                clients[id].defesa = 4;
            } else if (clients[id].DEX==3) {
                clients[id].DEX = 4;
                clients[id].WIS = 4;
            } else if (clients[id].DEX == 4) {
                clients[id].MaxHP = 50;
                clients[id].ataque = 5;
                clients[id].DEX = 5;
            } else if (clients[id].WIS == 4) {
                clients[id].defesa=5;
                clients[id].WIS=5;
            } else if (clients[id].ataque == 5) {
                clients[id].MaxHP=60;
                clients[id].ataque=6;
            } else {

            }
            break;
        case 2:
            if (clients[id].ataque == 3) {
                clients[id].ataque = 4;
                clients[id].DEX = 7;
            } else if (clients[id].WIS==2) {
                clients[id].MaxHP = 30;
                clients[id].defesa = 3;
                clients[id].WIS = 3;
            } else if (clients[id].DEX == 7) {
                clients[id].ataque = 5;
                clients[id].DEX = 8;
            } else if (clients[id].WIS == 3) {
                clients[id].MaxHP=40;
                clients[id].WIS=4;
            } else if (clients[id].DEX == 8) {
                clients[id].DEX=9;
                clients[id].ataque=6;
            } else {

            }
            break;
        case 3:
            if (clients[id].ataque == 6) {
                clients[id].ataque = 7;
                clients[id].DEX = 5;
            } else if (clients[id].defesa==2) {
                clients[id].MaxHP = 30;
                clients[id].defesa = 3;
            } else if (clients[id].DEX == 5) {
                clients[id].ataque = 8;
                clients[id].DEX = 6;
            } else if (clients[id].WIS == 1) {
                clients[id].MaxHP=40;
                clients[id].WIS=2;
                clients[id].defesa=4;
            } else if (clients[id].DEX == 5) {
                clients[id].DEX=6;
                clients[id].ataque=9;
            } else {

            }
            break;
        case 4:
            if (clients[id].defesa == 6) {
                clients[id].defesa = 7;
                clients[id].MaxHP = 50;
            } else if (clients[id].ataque==2) {
                clients[id].ataque = 3;
                clients[id].DEX = 3;
                clients[id].WIS = 2;
            } else if (clients[id].defesa ==7) {
                clients[id].defesa = 8;
                clients[id].MaxHP = 60;
            } else if (clients[id].ataque == 3) {
                clients[id].ataque=4;
                clients[id].DEX=4;
            } else if (clients[id].defesa == 8) {
                clients[id].MaxHP=70;
                clients[id].defesa=9;
            } else {

            }
            break;
        case 5:
            if (clients[id].WIS == 6) {
                clients[id].WIS = 7;
                clients[id].MaxHP = 30;
            } else if (clients[id].DEX==3) {
                clients[id].DEX = 4;
                clients[id].defesa = 3;
            } else if (clients[id].WIS == 7) {
                clients[id].WIS = 8;
                clients[id].MaxHP = 40;
            } else if (clients[id].DEX == 4) {
                clients[id].ataque=3;
                clients[id].defesa=4;
                clients[id].DEX=5;
            } else if (clients[id].WIS == 8) {
                clients[id].MaxHP=50;
                clients[id].WIS=9;
            } else {

            }
            break;
    }
}