#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream.h>
#include "myclass.h"

template <class all> class ALL:
  public SHARE, public GIFTS, public BONUS, public MONSTER
{
  all *array;
  long size;
  int error;
public:
  ALL(int value1=-1, int value2=-1, int value3=-1);
  int SET(void);
  void OBJECTS(void);
  void COLISION(BOY *boy,MUSIC *musicas);
  all &operator[] (unsigned index) { return array[index]; }
  ~ALL();
};

/* ALL */

template <class all> ALL <all>::ALL(int value1, int value2, int value3):
  GIFTS(), BONUS(), SHARE(), MONSTER()
{
  long length;

  error=0;
  if(value1==-1) length=LENGTH(value1,"MOVEL.001");
  else
  {
    length=LENGTH(value1);
    if(value2!=-1) length+=LENGTH(value2);
    if(value3!=-1) length+=LENGTH(value3);
  }
  if((array=new all [length])==NULL) error=100;
  size=length;
}

template <class all> ALL <all>::~ALL(void)
{
  delete array;
}

template <class all> int ALL <all>::SET(void)
{
  long cont;

  if(error!=0) return(error);
  for(cont=0;cont<size;cont++)
  {
    array[cont].SET(cont+1);
    if((error=array[cont].ERROR())!=0) break;
  }
  return(error);
}

template <class all> void ALL <all>::OBJECTS(void)
{
  long cont;
  for(cont=0;cont<size;cont++)
    array[cont].OBJECTS();
}

template <class all> void ALL <all>::COLISION(BOY *boy,MUSIC *musicas)
{
  long cont;
  for(cont=0;cont<size;cont++)
    array[cont].COLISION(boy,musicas);
}


#define XMIN 0                               /* coordenada horizontal inicial do ecran de jogo */
#define XMAX 320                             /* coordenada horizontal final do ecran de jogo */
#define YMIN 32                               /* coordenada vertical inicial do ecran de jogo */
#define YMAX 160                             /* coordenada vertical final do ecran de jogo */
#define XMAXMAPA 99                          /* tamanho horizontal do mapa de jogo em gr ficops de 32*32 */
#define YMAXMAPA 12                          /* tamanho vertical do mapa de jogo em gr ficops de 32*32 */


/*---------------------------------------------------------------------------*/
/*  Programa principal                                                       */
/*                                                                           */
/*  Parametros:                                                              */
/*    n„o tem                                                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
int main(int argc,char *argv[])
{
  int valpal,bonec,vid,cont,coderro=0,keyvalue=1;
  short tocainic=0;
  short timedelay=10;
  long length;

  if(argc>=2)
    timedelay=atoi(argv[1]);
  if(argc>=3)
    tocainic=atoi(argv[2]);
  if(argc==4)
    keyvalue=atoi(argv[3]);
  if(timedelay<0) timedelay=0;
  if(tocainic>1) tocainic=0;
  if(keyvalue>1) keyvalue=0;
  do
  {
    MUSIC musicas("musica.som","choque.som","apanha.som","mata.som",tocainic);
    if((coderro=musicas.INITMUSIC())!=0) break;

    GRAFIC modo_grafico;

    PALETTE pal;
    if((coderro=pal.SETPALETTE())!=0) break;
    pal.RESETPALETTE();

    BORDER fundo1("FUNDO1.BIT",0,0);
    if((coderro=fundo1.ERROR())!=0) break;

    BORDER fundo2("FUNDO2.BIT",0,159);
    if((coderro=fundo2.ERROR())!=0) break;

    SPRITE::XCOORD=0;
    SPRITE::YCOORD=4*32;
    MAP maps;
    if((coderro=maps.ERROR())!=0) break;

    BACKGROUND background;
    if((coderro=background.ERROR())!=0) break;

    BOY boy;
    if((coderro=boy.ERROR())!=0) break;

    MONSTER &mons=0;  /* so para inicializar *mostro e desloca */
    ALL <MONSTER> monstros;
    if((coderro=monstros.SET())!=0) break;
    length=LENGTH(-1,"MOVEL.001")-1;

    GIFTS &gift=0;
    ALL <GIFTS> gifts(5);
    if((coderro=gifts.SET())!=0) break;

    WEAPONS &weap=0;
    ALL <WEAPONS> weapons(0);
    if((coderro=weapons.SET())!=0) break;


    BONUS &bon=0;
    ALL <BONUS> bonus(1,2,3);
    if((coderro=bonus.SET())!=0) break;

    SHARE &shar=0;
    ALL <SHARE> share(4);
    if((coderro=share.SET())!=0) break;
/*
    valpal=pal++;
    for(cont=0;cont<32*8;cont++)
    {
      SPRITE::YCOORD=cont;
      background.PAINT();
      maps.ECRANBACK();
      maps.REFRESH();
      delay(10);
    }
    SPRITE::YCOORD=32*3+2;
    for(cont=0;cont<32*89;cont++)
    {
      SPRITE::XCOORD=cont;
      background.PAINT();
      maps.ECRANBACK();
      maps.REFRESH();
      delay(1);
    }
    */
    if(keyvalue==1)
    {
      background.PAINT();
      boy.STORE();
      gifts.OBJECTS();
      weapons.OBJECTS();
      bonus.OBJECTS();
      share.OBJECTS();
      monstros.OBJECTS();
      maps.ECRANBACK();
    }
    else
    {
      maps.ECRAN();
      boy.STORE();
      gifts.OBJECTS();
      weapons.OBJECTS();
      bonus.OBJECTS();
      share.OBJECTS();
      monstros.OBJECTS();
    }

    boy.INFO();

    maps.REFRESH();
    valpal=pal++;

    KEYPRESS teclado;

			      /* Ciclo principal do jogo */
    do
    {
      vid=boy.LIVES();
			      /* Calcula as colis”es */
      monstros.COLISION(&boy,&musicas);
      if(vid!=boy.LIVES()) valpal=pal--;

      gifts.COLISION(&boy,&musicas);
      bonus.COLISION(&boy,&musicas);
      share.COLISION(&boy,&musicas);
      weapons.COLISION(&boy,&musicas);

			  /* Descodifica‡„o do teclado */
      boy.KEYWORD(&teclado,&maps);

			     /* Movimento do boneco */
      boy.MOVEMENT(&teclado,&maps);

			  /* Posiciona o ecran de fundo */

      keyvalue=TECLABACK(&musicas,&teclado,keyvalue);
      if(keyvalue==1)
      {
	background.PAINT();

				/* Posiciona o boneco */
	boy.STORE();

	      /* Calcula as novas posi‡”es dos OBJECTOS e posiciona-os */
	gifts.OBJECTS();
	bonus.OBJECTS();
	share.OBJECTS();
	weapons.OBJECTS();
	monstros.OBJECTS();
				/* Calcula o disparo */
	weap.SHOOT(&boy);
	maps.ECRANBACK();
      }
      else
      {
	maps.ECRAN();

				/* Posiciona o boneco */
	boy.STORE();

	      /* Calcula as novas posi‡”es dos OBJECTOS e posiciona-os */
	gifts.OBJECTS();
	bonus.OBJECTS();
	share.OBJECTS();
	weapons.OBJECTS();
	monstros.OBJECTS();
				/* Calcula o disparo */
	weap.SHOOT(&boy);
      }

			      /* Calcula o disparo */
      for(cont=0;cont<length;cont++)
	weap.CRASH(&monstros[cont],&boy,&musicas);

		/* Copia o ecran virtual para a mem¢ria de v¡deo */
      maps.REFRESH();

		 /* Determina se deve haver ou n„o musica e sons */
      TECLAMUSIC(&musicas,&teclado);

      if (vid!=boy.LIVES() && boy.LIVES()>0) valpal=pal++;

      boy.INFO();

//      WAIT(&musicas);
      delay(timedelay);

    }while(!teclado.KEY(1) && !(SPRITE::XCOORD==XMAXMAPA*32-(XMAX-XMIN) && SPRITE::YCOORD==YMAXMAPA*32-(YMAX-YMIN) && boy.FINISH()) && vid);
    musicas.SOM2MUSIC();
    if(valpal>0) valpal=pal--;
  }while(0);
  if(coderro!=0)
    ERROR(coderro);
  return(coderro);
}
