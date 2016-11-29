#include <stdio.h>
#include <stdlib.h>

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
/*
	fight
	= 0 se não está lutando
	= 1 se lutando contra monstro, e whofight = id do monstro
	= 2 se lutando contra outro player, e whofight = id do player, turn diz se é a vez de tal cleinte
*/

typedef struct player player;

int main(int argc, char const *argv[])
{
	FILE *MONSTER,*MAPFILE;
	int mapa,i;
	char filename[50],mapName[50];
	int mapaAlt,mapaLarg,qtdMobs;
	player *mobs;

	printf("Digite o Numero do Mapa: \n");
	scanf("%d",&mapa);

	sprintf(mapName,"mapa%d.txt",mapa); //printa na string mapName o caminho para o arquivo do mapa
	
	MAPFILE=fopen(mapName,"r");
    fscanf(MAPFILE,"%d %d %d", &mapaAlt,&mapaLarg,&qtdMobs);

    mobs=(player*)calloc(qtdMobs,sizeof(player));

	sprintf(filename,"monstros%d.bin",mapa);

	MONSTER=fopen(filename,"wb");

	printf("O mapa escolhido tem %d monstros\n",qtdMobs );
	printf("Iniciando Criacao de Monstros\n");
	for(i=0;i<qtdMobs;i++)
	{	
		printf("<Monstro Numero [%d]>\n", i);
		printf("Nome: ");
		scanf(" %[^\n]",mobs->nome);
		printf("HP: ");
		scanf("%d",&mobs->vida);
		printf("Ataque: ");
		scanf("%d",&mobs->ataque);
		printf("Defesa: ");
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

	return 0;
}
