#ifndef MYCLASS_H
#define MYCLASS_H

extern void ERROR(int coderror);
extern long LENGTH(int value=0, char *name="PARADO.001");

extern int far *ecran_mem;                     /* ponteiro para a zona de ecran virtual */


	    /* estructura que contem os dados dos sprites parados */
struct STOPED
{
                     /* 1 se existem 0 se j  n„o existem */
  unsigned char exist;

                              /* tipo de sprite */
                              /* 0 pedra        */
                              /* 1 ma‡„         */
                              /* 2 pˆra         */
                              /* 3 ovo          */
                              /* 4 flores       */
			      /* 5 boneca       */
  unsigned char kind;

			 /* coordenadas dos objectos */
  unsigned int x;
  unsigned int y;
};

	 /* estructura que contem os dados dos sprites que se movem */
struct MOVING : public STOPED
{
			      /* tipo de sprite */
			      /* 10 monstro1    */
			      /* 11 monstro2    */
			      /* 12 monstro3    */
			      /* 13 monstro4    */
			      /* 21 chao1       */
			      /* 22 chao2       */
			      /* 23 chao3       */

				/* velocidade */
  unsigned char vel;

	     /* aponta para a tabela de deslocamentos espec¡fica */
  unsigned char pointer;
       /* ponteiro para a posi‡„o na tabela de deslocamentos e sprites */
  unsigned char shift;
};

struct DESLOC
{
			 /* tabela de deslocamentos */
//  unsigned char desloc[MAX_DESLOC];
			    /* tabela dos sprites */
			      /* 10 monstro1    */
			      /* 11 monstro2    */
			      /* 12 monstro3    */
			      /* 13 monstro4    */
			      /* 21 chao1       */
			      /* 22 chao2       */
			      /* 22 chao3       */
//  unsigned char image[MAX_DESLOC];
  unsigned char far *desloc;
  unsigned char far *image;
};


class GRAFIC
{
  unsigned char old_mode;
  unsigned char new_mode;
public:
  GRAFIC(short mode=19);
  unsigned char CURRENT_MODE(void) { return new_mode; }
  unsigned char OLD_MODE(void) { return old_mode; }
  ~GRAFIC(void);
};


class MUSIC;


class KEYPRESS
{
  #define nk 128
  static int keymap[nk];
  unsigned char key_intr;
  static void interrupt (far * near old_intr)(...);
  static void interrupt KEYCLICK(...);
public:
  KEYPRESS(void);
  int KEY(int num) { return keymap[num]; }
  friend void TECLAMUSIC(MUSIC *musica,KEYPRESS *tecla);
  friend int TECLABACK(MUSIC *musica,KEYPRESS *tecla,int keyvalue);
  ~KEYPRESS(void);
};


class MUSIC
{
  #define MAXNOTA 512
  #define MAXACOMP 16
  #define NMUSICA 3
  int play_start;
  static unsigned int *music_pointer[NMUSICA];
  static unsigned int musics[3][MAXNOTA+1];
  unsigned int acomp[3][MAXACOMP+1];
  static int sincmusic;       /* para sincronizar as mudan‡as de m£sicas */
  static int sound_on;           /* indica se est  a tocar os sons especiais */
  static int timer;           /* controla um timer */
  static int timerdelay;           /* controla um timer */
  static int play;            /* se h  ou n„o musica de fundo */
  unsigned char clktick_intr;
  static void interrupt (far * near old_intr)(...);
  static void interrupt ClkTick(...);
  unsigned char error;
public:
  MUSIC(char *name1,char *name2,char *name3,char *name4, int start=0);
  int INITMUSIC(void);
  void SOM2MUSIC(void);
  void MUSIC2SOM(int sound);
  friend void TECLAMUSIC(MUSIC *musica,KEYPRESS *tecla);
  friend int TECLABACK(MUSIC *musica,KEYPRESS *tecla,int keyvalue);
  friend void WAIT(MUSIC *musica);
  ~MUSIC(void);
};


class PALETTE
{
  int error;
  int fade;
  unsigned char *r,*g,*b;              /* palette das cores */
  unsigned char *rbak,*gbak,*bbak;
  void FADEOUT(void);
  void FADEIN(void);
public:
  PALETTE(char *pal="FUNDO.PAL");
  int ERROR(void) { return(error); }
  int SETPALETTE(void);
  void RESETPALETTE(void);
  int operator ++(int);
  int operator --(int);
  ~PALETTE(void);
};


class SPRITE
{
  int error;
public:
  static unsigned int XCOORD;
  static unsigned int YCOORD;
//  static unsigned char far *ecran_mem;                     /* ponteiro para a zona de ecran virtual */

  SPRITE(void) { error=0; }
  virtual void SETERROR(int err) { error=err; }
  virtual int ERROR(void) { return(error); }
  virtual void STORE(unsigned char far *buf,int x0,int y0);
  virtual int READ_BIT(unsigned char far *buf,char *bit,unsigned pointer=0);
};


class MAP : public SPRITE
{
  unsigned char far *enfeite;
  unsigned char far *buf1;
  unsigned char far *buf2;
  int XMAXMAPA;		       	  /* tamanho horizontal do mapa de jogo em gr ficops de 32*32 */
  int YMAXMAPA;                   /* tamanho vertical do mapa de jogo em gr ficops de 32*32 */
  void LENGTH(char *name);
public:
  MAP(char *mapa="MAPAS.001");
  int XMAP(void) { return(XMAXMAPA); }
  int YMAP(void) { return(YMAXMAPA); }
  unsigned char BUFF(int par1, int par2) { return(buf2[par1*(XMAXMAPA+1)+par2]); }
  void REFRESH(void);
  void ECRAN(void);
  void ECRAN2(void);
  void ECRANBACK(void);
  ~MAP(void);
};


class BOY : public SPRITE
{
  unsigned int DDX;
  unsigned int DDY;
  unsigned int SALT;
  unsigned int MAXVIDA;
  unsigned int MAXESCUDO;
  unsigned int MAXPODER;
  unsigned int MAXENERGIA;
  unsigned int DISDISPARO;
  unsigned int DISPINIC;
  unsigned int poder;
  int shield;
  int step;
  int jump;
  int x1;
  int y1;
  unsigned char direc;
  unsigned char deita;
  int shoot;
  unsigned char far *boneco[8];
  int lives;
  int energy;
  unsigned int points;
  unsigned int amunition;
public:
  BOY(void);
  void INCPOINTS(int point) { points+=point; }
  void INCAMNO(void) { amunition++; }
  void DECSHOOT(void) { shoot--; }
  void RESETSHOOT(void) { shoot=0; }
  int DECENERGY(void);
  int SHOOT(void) { return(shoot); }
  int ENDSHOOT(void) {return(shoot==DISDISPARO); }
  unsigned char DIREC(void) { return(direc); }
  unsigned char DEITA(void) { return(deita); }
  int X1() { return(x1); }
  int Y1() { return(y1); }
  void KEYWORD(KEYPRESS *tecla,MAP *maps);
  void MOVEMENT(KEYPRESS *tecla,MAP *maps);
  int LIVES(void) { return(lives); }
  int FINISH(void) { return(x1==224 && y1==88); }
  void INFO(void);
  void STORE(unsigned char far *buf=NULL,int x0=0,int y0=0);
  ~BOY(void);
};


class FIGURE : public SPRITE
{
protected:
  virtual void OBJECTS(void)=0;
  virtual void COLISION(BOY *boy,MUSIC *musica)=0;
};


class MONSTERIMAGES : public FIGURE
{
  static int flag;
  static long NUM_DESLOC;
protected:
  static unsigned char far *monstro[10];
  static struct DESLOC far *desloca;
  void MLENGTH(char *name="DESLOC.001");
  static long MAX_DESLOC;
public:
  MONSTERIMAGES(int num=1,char *name="DESLOC.001",char *mons1="monstro1.bit",char *mons2="monstro2.bit",char *mons3="monstro3.bit",char *mons4="monstro4.bit");
  ~MONSTERIMAGES(void);
};


class MONSTER : public MONSTERIMAGES
{
  int counter;
  struct MOVING movel;
public:
  MONSTER(int num) : MONSTERIMAGES(num) {}
  MONSTER(void) {}
  void SET(int num=1,char *name="MOVEL.001");
  unsigned char POINTER(void) { return(movel.pointer); }
  unsigned char SHIFT(void) { return(movel.shift); }
  unsigned char EXIST(void) { return(movel.exist); }
  unsigned char MONST(int desl,int img, int monst) { return(monstro[desloca[desl].image[img]][monst]); }
  unsigned int X(void) { return(movel.x); }
  unsigned int Y(void) { return(movel.y); }
  void RESETEXIST(void) { movel.exist=0; }
  void COLISION(BOY *boy,MUSIC *musica);
  void OBJECTS(void);
};


class WEAPONS : public FIGURE
{
  static int flag;
  static unsigned char far *atira;
  int xdis;
  int ydis;
  unsigned int VELDISPARO;
  struct STOPED stop;
public:
  WEAPONS(int num, char *name="PEDRA1.BIT");
  WEAPONS(void) {}
  void SET(int num,int value=0, char *name="PARADO.001");
  void SHOOT(BOY *boy);
  void CRASH(MONSTER *monstros,BOY *boy,MUSIC *musica);
  void OBJECTS(void);
  void COLISION(BOY *boy,MUSIC *musica);
  ~WEAPONS(void);
};


class GIFTS : public FIGURE
{
  static int flag;
  static unsigned char far *prenda;
  struct STOPED stop;
public:
  GIFTS(int num, char *name="BONECA1.BIT");
  GIFTS(void) {}
  void SET(int num,int value=5, char *name="PARADO.001");
  void OBJECTS(void);
  void COLISION(BOY *boy,MUSIC *musica);
  ~GIFTS(void);
};


class BONUS : public FIGURE
{
  static int flag;
  static unsigned char far *bonus[3];
  struct STOPED stop;
public:
  BONUS(int num, char *name1="MACA1.BIT", char *name2="PERA1.BIT", char *name3="OVO1.BIT");
  BONUS(void) {}
  void SET(int num,int lowvalue=1, int uppervalue=3, char *name="PARADO.001");
  void OBJECTS(void);
  void COLISION(BOY *boy,MUSIC *musica);
  ~BONUS(void);
};


class SHARE : public FIGURE
{
  static int flag;
  static unsigned char far *share;
  struct STOPED stop;
public:
  SHARE(int num, char *name="FLOR1.BIT");
  SHARE(void) {}
  void SET(int num, int value=4, char *name="PARADO.001");
  void OBJECTS(void);
  void COLISION(BOY *boy,MUSIC *musica);
  ~SHARE(void);
};


class BORDER :public SPRITE
{
  void STORE(unsigned char far *buf,int x0,int y0);
public:
  BORDER(char *fundo="FUNDO1.BIT",int x0=0,int y0=0);
};


class BACKGROUND :public SPRITE
{
  unsigned char far *fund;
public:
  BACKGROUND(char *fundo="BACKGND1.BIT");
  void PAINT(void);
  ~BACKGROUND(void);
};

#endif
