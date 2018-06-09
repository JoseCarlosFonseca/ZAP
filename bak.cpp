#include <dos.h>
#include <stdlib.h>
#include <iostream.h>
#include "myclass.h"

#define XMIN 0                               /* coordenada horizontal inicial do ecran de jogo */
#define XMAX 320                             /* coordenada horizontal final do ecran de jogo */
#define YMIN 32                               /* coordenada vertical inicial do ecran de jogo */
#define YMAX 160                             /* coordenada vertical final do ecran de jogo */
#define XMAXMAPA 99                          /* tamanho horizontal do mapa de jogo em gr ficops de 32*32 */
#define YMAXMAPA 12                          /* tamanho vertical do mapa de jogo em gr ficops de 32*32 */

const NUM_MOVING=49;
const NUM_WEAPON=25;
const NUM_BONUS=70;
const NUM_SHARE=23;
const NUM_GIFTS=4;



/*---------------------------------------------------------------------------*/
/*  Programa principal                                                       */
/*                                                                           */
/*  Parametros:                                                              */
/*    n„o tem                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int main(int argc,char *argv[])
{
  int bonec,vid,cont,coderro=0;
  short tocainic=0;
  short timedelay=40;

  if(argc>=2)
    timedelay=atoi(argv[1]);
  if(argc==3)
    tocainic=atoi(argv[2]);
  if(timedelay<0) timedelay=0;
  if(tocainic>1) tocainic=0;
  do
  {
    MUSIC musicas("musica.som","choque.som","apanha.som","mata.som",tocainic);
    if((coderro=musicas.INITMUSIC())!=0) break;

    GRAFIC modo_grafico;

    PALETTE pal;
    if((coderro=pal.SETPALETTE())!=0) break;
    pal.RESETPALETTE();

    BACKGROUND fundo1("FUNDO1.BIT",0,0);
    if((coderro=fundo1.ERROR())!=0) break;

    BACKGROUND fundo2("FUNDO2.BIT",0,159);
    if((coderro=fundo2.ERROR())!=0) break;

    MAP maps;
    if((coderro=maps.ERROR())!=0) break;

    BOY boy;
    if((coderro=boy.ERROR())!=0) break;

    MONSTER &mons=0;  /* so para inicializar *mostro e desloca */
    MONSTER monstros[NUM_MOVING];
    for(cont=0;cont<NUM_MOVING-1;cont++)
    {
      monstros[cont].SET(cont+1);
      if((coderro=monstros[cont].ERROR())!=0) break;
    }
    if(coderro!=0) break;

/*
    GIFTS &gift=0;
    GIFTS gifts[NUM_GIFTS];
    for(cont=0;cont<NUM_GIFTS-1;cont++)
    {
      gifts[cont].SET(cont+1);
      if((coderro=gifts[cont].ERROR())!=0) break;
    }
    if(coderro!=0) break;
*/
//    ALL <GIFTS> gifts(NUM_GIFTS);
//    if((coderro=gifts.SET())!=0) break;

    WEAPONS &weap=0;
    WEAPONS weapons[NUM_WEAPON];
    for(cont=0;cont<NUM_WEAPON-1;cont++)
    {
      weapons[cont].SET(cont+1);
      if((coderro=weapons[cont].ERROR())!=0) break;
    }
    if(coderro!=0) break;

    BONUS &bon=0;
    BONUS bonus[NUM_BONUS];
    for(cont=0;cont<NUM_BONUS-1;cont++)
    {
      bonus[cont].SET(cont+1);
      if((coderro=bonus[cont].ERROR())!=0) break;
    }
    if(coderro!=0) break;

    SHARE &shar=0;
    SHARE share[NUM_SHARE];
    for(cont=0;cont<NUM_SHARE-1;cont++)
    {
      share[cont].SET(cont+1);
      if((coderro=share[cont].ERROR())!=0) break;
    }
    if(coderro!=0) break;

    maps.ECRAN();
    boy.STORE();
    for(cont=0;cont<NUM_MOVING-1;cont++)
      monstros[cont].OBJECTS();
/*
    for(cont=0;cont<NUM_GIFTS-1;cont++)
      gifts[cont].OBJECTS();
*/
//    gifts.OBJECTS();

    for(cont=0;cont<NUM_WEAPON-1;cont++)
      weapons[cont].OBJECTS();
    for(cont=0;cont<NUM_BONUS-1;cont++)
      bonus[cont].OBJECTS();
    for(cont=0;cont<NUM_SHARE-1;cont++)
      share[cont].OBJECTS();

    maps.REFRESH();
    pal++;

    KEYPRESS teclado;

			      /* Ciclo principal do jogo */
    do
    {
      vid=boy.LIVES();
			      /* Calcula as colis”es */
      for(cont=0;cont<NUM_MOVING-1;cont++)
	monstros[cont].COLISION(&boy,&musicas);
      if(vid!=boy.LIVES()) pal--;

/*
      for(cont=0;cont<NUM_GIFTS-1;cont++)
	gifts[cont].COLISION(&boy,&musicas);
*/
//      gifts.COLISION(&boy,&musicas);

      for(cont=0;cont<NUM_BONUS-1;cont++)
	bonus[cont].COLISION(&boy,&musicas);
      for(cont=0;cont<NUM_SHARE-1;cont++)
	share[cont].COLISION(&boy,&musicas);
      for(cont=0;cont<NUM_WEAPON-1;cont++)
	weapons[cont].COLISION(&boy,&musicas);

			  /* Descodifica‡„o do teclado */
      boy.KEYWORD(&teclado);

			     /* Movimento do boneco */
      boy.MOVEMENT(&teclado);

			  /* Posiciona o ecran de fundo */
      maps.ECRAN();

			      /* Posiciona o boneco */
      boy.STORE();

	    /* Calcula as novas posi‡”es dos OBJECTOS e posiciona-os */
      for(cont=0;cont<NUM_MOVING-1;cont++)
	monstros[cont].OBJECTS();
/*
      for(cont=0;cont<NUM_GIFTS-1;cont++)
	gifts[cont].OBJECTS();
*/
//      gifts.OBJECTS();

      for(cont=0;cont<NUM_BONUS-1;cont++)
	bonus[cont].OBJECTS();
      for(cont=0;cont<NUM_SHARE-1;cont++)
	share[cont].OBJECTS();
      for(cont=0;cont<NUM_WEAPON-1;cont++)
	weapons[cont].OBJECTS();

			      /* Calcula o disparo */
      weap.SHOOT(&boy);
      for(cont=0;cont<NUM_MOVING-1;cont++)
	weap.CRASH(&monstros[cont],&boy,&musicas);

		/* Copia o ecran virtual para a mem¢ria de v¡deo */
      maps.REFRESH();

		 /* Determina se deve haver ou n„o musica e sons */
      TECLAMUSIC(&musicas,&teclado);

      if (vid!=boy.LIVES()) pal++;

      boy.INFO();
      delay(timedelay);
    }while(!teclado.KEY(1) && !(XCOORD==XMAXMAPA*32-(XMAX-XMIN) && YCOORD==YMAXMAPA*32-(YMAX-YMIN) && boy.FINISH()) && vid);
    musicas.SOM2MUSIC();
    pal--;
  }while(0);
  if(coderro!=0)
    ERROR(coderro);
  return(coderro);
}
