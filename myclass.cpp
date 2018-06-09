#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <iostream.h>
#include <iomanip.h>
#include "myclass.h"

#define XMIN 0                               /* coordenada horizontal inicial do ecran de jogo */
#define XMAX 320                             /* coordenada horizontal final do ecran de jogo */
#define XMAXmXMIN XMAX-XMIN
#define YMIN 32                               /* coordenada vertical inicial do ecran de jogo */
#define YMAX 160                             /* coordenada vertical final do ecran de jogo */
#define YMAX1 YMAX+1
#define OFFSETECRAN YMIN*320+XMIN
#define LINHxCOL (YMAX-YMIN)*(XMAX-XMIN)/2
#define LINHxCOL2 (YMAX-YMIN)*(XMAX-XMIN)/2
#define XMAXECRAN ((XMAX-XMIN)/32+1)
#define YMAXECRAN ((YMAX-YMIN)/32+1)


int KEYPRESS::keymap[nk];
void interrupt (far * near KEYPRESS::old_intr)(...);

unsigned int *MUSIC::music_pointer[NMUSICA];
unsigned int MUSIC::musics[3][MAXNOTA+1];
int MUSIC::sincmusic;       /* para sincronizar as mudanáas de m£sicas */
int MUSIC::sound_on;           /* indica se est† a tocar a os sons especiais */
int MUSIC::timer;           /* controla um timer */
int MUSIC::timerdelay;           /* controla um timer */
int MUSIC::play;            /* se h† ou nÑo musica de fundo */
void interrupt (far * near MUSIC::old_intr)(...);

int MONSTERIMAGES::flag;
unsigned char far* MONSTERIMAGES::monstro[10];
long MONSTERIMAGES::NUM_DESLOC;
long MONSTERIMAGES::MAX_DESLOC;
struct DESLOC far *MONSTERIMAGES::desloca;

int WEAPONS::flag;
unsigned char far *WEAPONS::atira;

int GIFTS::flag;
unsigned char far *GIFTS::prenda;

int BONUS::flag;
unsigned char far *BONUS::bonus[3];

int SHARE::flag;
unsigned char far *SHARE::share;

unsigned int SPRITE::XCOORD;
unsigned int SPRITE::YCOORD;

int far *ecran_mem;


/* GRAFIC */


/*---------------------------------------------------------------------------*/
/*  Inicia novo modo de video                                                */
/*---------------------------------------------------------------------------*/
GRAFIC::GRAFIC(short mode)
{
  unsigned char aux=0;
  asm {
    mov ah,15
    int(16)
    mov aux,al
    mov ax,mode
    int(16)
  }
  old_mode=aux;
  new_mode=mode;
}

/*---------------------------------------------------------------------------*/
/*  Retorna ao modo de video anterior                                        */
/*---------------------------------------------------------------------------*/
GRAFIC::~GRAFIC(void)
{
  short aux=old_mode;
  asm {
    mov ax,aux;
    int(16)
  }
}


/* KEYPRESS */


/*---------------------------------------------------------------------------*/
/*  Inicia o mapa de TECLADO                                                 */
/*---------------------------------------------------------------------------*/
KEYPRESS::KEYPRESS(void)
{
  int i;
  for(i=0;i<nk;i++)
    keymap[i]=0;
  key_intr=0x09;                      /* interrupt do teclado */
  old_intr=getvect(key_intr);
  setvect(key_intr,&KEYPRESS::KEYCLICK);
}

/*---------------------------------------------------------------------------*/
/*  Retorna ao antigo modo de video                                          */
/*---------------------------------------------------------------------------*/
KEYPRESS::~KEYPRESS(void)
{
  setvect(key_intr, old_intr);
}

/*---------------------------------------------------------------------------*/
/*  Rotina de gestÑo do TECLADO por interrupáîes                             */
/*---------------------------------------------------------------------------*/
void interrupt KEYPRESS::KEYCLICK(...)
{
  unsigned char Code=0;
  #define KeyPort0 0x60
  #define KeyPort1 0x61

  asm         mov     al,20h
  asm         out     20h,al

  asm         in      al,KeyPort0
  asm         mov     Code,al
  asm         in      al,KeyPort1
  asm         or      al,80h
  asm         out     KeyPort1,al
  asm         in      al,KeyPort1
  asm         and     al,7Fh
  asm         out     KeyPort1,al
  if(!(Code & 0x80))
    keymap[Code]=-1;                    /* se a tecla est† premida o seu valor Ç -1 */
  else
    keymap[Code & 0x7f]=0;              /* se a tecla nÑo est† premida o seu valor Ç 0 */
}


/* MUSICA */


/*---------------------------------------------------------------------------*/
/*  le os ficheiros de soms e musica                                         */
/*---------------------------------------------------------------------------*/
MUSIC::MUSIC(char *name1,char *name2,char *name3,char *name4,int start)
{
  FILE *in1,*in2,*in3,*in4;
  int cont;

  timerdelay=-1;
  play_start=start;
  error=0;

  if ((in1 = fopen(name1, "rt"))== NULL) error=59;
  else
  {
    cont=0;
    while (!feof(in1))
    {
      fscanf(in1,"%8u%8u%8u",&musics[0][cont],&musics[1][cont],&musics[2][cont]);
      fgetc(in1);
      cont++;
    }
    musics[0][cont]=musics[1][cont]=musics[2][cont]=-1;
    fclose(in1);
  }
  if ((in2 = fopen(name2, "rt"))== NULL) error=60;
  else
  {
    cont=0;
    while (!feof(in2))
    {
      fscanf(in2,"%8u%8u%8u",&acomp[0][cont],&acomp[0][cont],&acomp[0][cont]);
      fgetc(in2);
      cont++;
    }
    acomp[0][cont]=-1;
    fclose(in2);
  }
  if ((in3 = fopen(name3, "rt"))== NULL) error=70;
  else
  {
    cont=0;
    while (!feof(in3))
    {
      fscanf(in3,"%8u%8u%8u",&acomp[1][cont],&acomp[1][cont],&acomp[1][cont]);
      fgetc(in3);
      cont++;
    }
    acomp[1][cont]=-1;
    fclose(in3);
  }
  if ((in4 = fopen(name4, "rt"))== NULL) error=71;
  {
    cont=0;
    while (!feof(in4))
    {
      fscanf(in4,"%8u%8u%8u",&acomp[2][cont],&acomp[2][cont],&acomp[2][cont]);
      fgetc(in4);
      cont++;
    }
    acomp[2][cont]=-1;
    fclose(in4);
  }
  /* A menor frequencia que se pode utilizar Ç de 21 hz e a maxima 65535hz */

  music_pointer[0]=&musics[0][0];
  music_pointer[1]=&musics[1][0];
  music_pointer[2]=&musics[2][0];

  sound_on=-1;
  play=0;
  timer=-1;
  sincmusic=0;

  /* O clock tick interrupt */
  clktick_intr=0x1C;
  /* get the address of the current clock tick interrupt */
  old_intr = getvect(clktick_intr);
  /* install the new interrupt handler */
  setvect(clktick_intr, &MUSIC::ClkTick);
}

/*---------------------------------------------------------------------------*/
/*  Inicia a musica                                                          */
/*---------------------------------------------------------------------------*/
int MUSIC::INITMUSIC(void)
{
  if(error==0)
  {
  /* A menor frequencia que se pode utilizar Ç de 21 hz e a maxima 65535hz */

  music_pointer[0]=&musics[0][0];
  music_pointer[1]=&musics[1][0];
  music_pointer[2]=&musics[2][0];
  play=play_start;  /* =1 toca; =0 nÑo toca */
  /* comeáa a tocar */
  sincmusic=1;
  return(0);
  }
  else return(error);
}

/*---------------------------------------------------------------------------*/
/*  deixa de tocar os sons                                                   */
/*---------------------------------------------------------------------------*/
void MUSIC::SOM2MUSIC(void)
{
  music_pointer[2]=&musics[2][0];
  sound_on=-1;
  sincmusic=1;
}

/*---------------------------------------------------------------------------*/
/*  comeáa a tocar sons                                                      */
/*---------------------------------------------------------------------------*/
void MUSIC::MUSIC2SOM(int sound)
{
  music_pointer[2]=&acomp[sound][0];
  sound_on=0;
  sincmusic=1;
}

/*---------------------------------------------------------------------------*/
/*  recupera a rotina de interrupcîes do click tic anterior                  */
/*---------------------------------------------------------------------------*/
MUSIC::~MUSIC(void)
{
/* install the old interrupt handler */
setvect(clktick_intr, old_intr);

//nosound()
asm in al,61h
asm and al,0xfc
asm out 61h,al
}

/*---------------------------------------------------------------------------*/
/*  Rotina de gestÑo do clock tic por interrupáîes                           */
/*---------------------------------------------------------------------------*/
void interrupt MUSIC::ClkTick(...)
{
  static int cont=0;
  static unsigned int pont[NMUSICA]={0,0,0};
  int musicas;

  if(timer>=0)
  {
    timer++;
  }
  if(timerdelay>=0)
  {
    timerdelay++;
  }
  if(play==1)
  {
    if(sincmusic==1)
    {
      sincmusic=0;
      cont=0;
      if(sound_on!=-1) pont[2]=0;
      else
        pont[2]=pont[1]=pont[0];
    }
    musicas=*(music_pointer[cont]+pont[cont]);
    if(musicas==-1)
    {
      pont[cont]=0;
      if(sound_on!=-1)
      {
        music_pointer[2]=&musics[2][0];
        pont[cont]=pont[0];
        sound_on=-1;
        sincmusic=1;
      }
      musicas=*(music_pointer[cont]+pont[cont]);
    }
    pont[cont]++;
    if(musicas>20) //sound(musicas);
    {
      asm {
        mov bx,musicas
        mov ax,0x34dd
        mov dx,0x0012
        div bx
        mov bx,ax
        in al,61h
        test al,3
        jne salto
        or al,3
        out 61h,al
        mov al,0xb6
        out 43h,al
        }
  salto:
      asm {
        mov al,bl
        out 42h,al
        mov al,bh
        out 42h,al
        }
    }
    else
    {
      //nosound()
      asm in al,61h
      asm and al,0xfc
      asm out 61h,al
    }
    cont++;
    if (cont==NMUSICA) cont=0;
  }
fim:
  old_intr();
}


/* friend TECLAMUSIC(MUSIC *musica,KEYPRESS *tecla) */


/*---------------------------------------------------------------------------*/
/*  carregando na tecla F1 ora toca ora nÑo toca sons e musica               */
/*---------------------------------------------------------------------------*/
void TECLAMUSIC(MUSIC *musica,KEYPRESS *tecla)
{
  if(musica->timer>10 || musica->timer <0)
  {
    if(tecla->KEY(59))
    {
      musica->timer=0;
      if(musica->play==1)
      {
        musica->play=0;
        //nosound()
        asm in al,61h
        asm and al,0xfc
        asm out 61h,al
      }
      else musica->play=1;
      musica->SOM2MUSIC();
    }
  }
}



/*---------------------------------------------------------------------------*/
/*  carregando na tecla F2 ora mostra o background ora nÑo                   */
/*---------------------------------------------------------------------------*/
int TECLABACK(MUSIC *musica,KEYPRESS *tecla,int keyvalue)
{
  if(musica->timer>10 || musica->timer <0)
  {
    if(tecla->KEY(60))
    {
      musica->timer=0;
      if(keyvalue==1)
        keyvalue=0;
      else keyvalue=1;
    }
  }
  return(keyvalue);
}


/*---------------------------------------------------------------------------*/
/*  carregando na tecla F2 ora mostra o background ora nÑo                   */
/*---------------------------------------------------------------------------*/
void WAIT(MUSIC *musica)
{
  if(musica->timerdelay<0) musica->timerdelay=0;
  while(musica->timerdelay<1);
  musica->timerdelay=0;
}


/* PALETTE */


/*---------------------------------------------------------------------------*/
/*  Là um ficheiro com a palette                                             */
/*                                                                           */
/*  Parametros:                                                              */
/*    pal Ç o nome do ficheiro que contÇm a palette                          */
/*---------------------------------------------------------------------------*/
PALETTE::PALETTE(char *pal)
{
  FILE *fp;
  int cont1;

  error=0;
  fade=0;

  if((r=new unsigned char [256])==NULL) error=80;
  if((g=new unsigned char [256])==NULL) error=81;
  if((b=new unsigned char [256])==NULL) error=82;

  if((rbak=new unsigned char [256])==NULL) error=83;
  if((gbak=new unsigned char [256])==NULL) error=84;
  if((bbak=new unsigned char [256])==NULL) error=85;

  if((fp = fopen(pal,"rb"))!=NULL)
  {
    for(cont1=0;cont1<256;cont1++)
    {
      r[cont1]=getc(fp)>>2;                   /* là e divide por 4 pois os valores da palette s¢ vÑo atÇ 64 */
      g[cont1]=getc(fp)>>2;                   /* là e divide por 4 pois os valores da palette s¢ vÑo atÇ 64 */
      b[cont1]=getc(fp)>>2;                   /* là e divide por 4 pois os valores da palette s¢ vÑo atÇ 64 */
    }
    fclose(fp);
  } else error=1;
}

PALETTE::~PALETTE(void)
{
  delete r;
  delete g;
  delete b;
  delete rbak;
  delete gbak;
  delete bbak;
}

/*---------------------------------------------------------------------------*/
/*  Torna activa a palette constituida por r[] g[] e b[]                     */
/*---------------------------------------------------------------------------*/
int PALETTE::SETPALETTE(void)
{
//  unsigned char rbak[256],gbak[256],bbak[256];
  short cont,aux=256;

  if(error!=0) return(error);

  fade=0;
  /*
  for(cont=0;cont<256;cont++)
  {
    rbak[cont]=r[cont];
    gbak[cont]=g[cont];
    bbak[cont]=b[cont];
  }
  */
  for(cont=0;cont<4;cont++)
  {
    asm push di
    asm mov dx,3dah

                 /* testa se est† a decorrer um retrace no ecran */
            /* isto Ç usado para diminuir as interferàncias no ecran */
  teste1:
    asm in al,dx
    asm test al,8
    asm jz teste1

                           /* modifica os 1¶s 64 cores */
      /* isto Ç feito em 4 ciclos para diminuir as interferàncias no ecran */
                                   /* 1ß ciclo */
    asm mov cx,64
  ciclo1:
    asm mov bx,aux
    asm sub bx,cx
    asm mov di,bx
    asm mov dx,0x03c8
    asm mov al,bl
    asm out dx,al
    asm mov dx,0x03c9
//    asm mov al,byte ptr rbak+di
//    asm out dx,al
    outp(_DX,r[_DI]);
//    asm mov al,byte ptr gbak+di
//    asm out dx,al
    outp(_DX,g[_DI]);
//    asm mov al,byte ptr bbak+di
//    asm out dx,al
    outp(_DX,b[_DI]);
    asm loop ciclo1

    aux-=64;
    asm pop di
  }

  return(0);
}

/*---------------------------------------------------------------------------*/
/*  Torna activa uma palette completamente negra                             */
/*---------------------------------------------------------------------------*/
void PALETTE::RESETPALETTE(void)
{
  int cont1;

  fade=0;
  for(cont1=0;cont1<256;cont1++)
  {
    rbak[cont1]=r[cont1];
    gbak[cont1]=g[cont1];
    bbak[cont1]=b[cont1];
  }
  for(cont1=0;cont1<256;cont1++)
  {
    r[cont1]=0;
    g[cont1]=0;
    b[cont1]=0;
  }
  SETPALETTE();

  for(cont1=0;cont1<256;cont1++)
  {
    r[cont1]=rbak[cont1];
    g[cont1]=gbak[cont1];
    b[cont1]=bbak[cont1];
  }
}

/*---------------------------------------------------------------------------*/
/*  Faz aparecer a palette desde o escuro atÇ Ös cores actuais               */
/*---------------------------------------------------------------------------*/
void PALETTE::FADEOUT(void)
{
  int cont1,cont2;

  for(cont1=0;cont1<256;cont1++)
  {
    rbak[cont1]=r[cont1];
    gbak[cont1]=g[cont1];
    bbak[cont1]=b[cont1];
  }

  for(cont1=0;cont1<256;cont1++)
  {
    r[cont1]=0;
    g[cont1]=0;
    b[cont1]=0;
  }
  for(cont2=0;cont2<15;cont2++)
  {
    for(cont1=0;cont1<256;cont1++)
    {
      if(rbak[cont1]>r[cont1] && rbak[cont1]!=0) r[cont1]+=4;
        else r[cont1]=rbak[cont1];
      if(gbak[cont1]>g[cont1] && gbak[cont1]!=0) g[cont1]+=4;
        else g[cont1]=gbak[cont1];
      if(bbak[cont1]>b[cont1] && bbak[cont1]!=0) b[cont1]+=4;
        else b[cont1]=bbak[cont1];
    }
//    delay(10);
    SETPALETTE();
  }
}

/*---------------------------------------------------------------------------*/
/*  Faz desaparecer a palette desde as cores actuais atÇ o escuro            */
/*---------------------------------------------------------------------------*/
void PALETTE::FADEIN(void)
{
  int cont1,cont2;

  for(cont1=0;cont1<256;cont1++)
  {
    rbak[cont1]=r[cont1];
    gbak[cont1]=g[cont1];
    bbak[cont1]=b[cont1];
  }

  for(cont2=0;cont2<15;cont2++)
  {
    for(cont1=0;cont1<256;cont1++)
    {
    if(r[cont1]>4) r[cont1]-=4;
    if(g[cont1]>4) g[cont1]-=4;
    if(b[cont1]>4) b[cont1]-=4;
    }
    SETPALETTE();
//    delay(10);
  }
  for(cont1=0;cont1<256;cont1++)
  {
    r[cont1]=0;
    g[cont1]=0;
    b[cont1]=0;
  }
  SETPALETTE();
  for(cont1=0;cont1<256;cont1++)
  {
    r[cont1]=rbak[cont1];
    g[cont1]=gbak[cont1];
    b[cont1]=bbak[cont1];
  }
}

int PALETTE::operator ++(int)
{
  FADEOUT();
  return(++fade);
}

int PALETTE::operator --(int)
{
  FADEIN();
  return(--fade);
}


/* SPRITE */




/*---------------------------------------------------------------------------*/
/*  Desenha no ecran virual um sprite tendo em conta os limites deste        */
/*  ê usado pelos OBJECTS()                                                  */
/*                                                                           */
/*  Parametros:                                                              */
/*    buf Ç buffer onde se encontram as dimensîes e o sprite                 */
/*    x0 e y0 sÑo as coordenadas em pixels                                   */
/*---------------------------------------------------------------------------*/
void SPRITE::STORE(unsigned char far *buf,int x0,int y0)
{
  int col,linh,aux;

  asm les ax,dword ptr ds:ecran_mem      /* zona do ecran virtual */
  asm mov aux,ax
  asm push ds
  asm lds si,buf                 /* si offset buffer na RAM */

                          /* là o n£mero de linhas */
  asm lodsw
  asm mov linh,ax

                          /* là o n£mero de colunas */
  asm lodsw
  asm mov col,ax

  asm mov cx,YMIN                       /* cx=32 */
  asm mov ax,y0                         /* ax=y0 */
  asm add ax,32                         /* ax=y0+32 */
  asm cmp ax,cx
  asm jge ymaior                        /* se y0+32>32 salta para ymaior: o sprite n∆o vai ser truncado no topo */

                /* ymenor: o sprite vai ser truncado no topo */
  asm sub cx,ax                         /* cx=32-(y0+32): Ç o nß de linhas do topo que v∆o ser truncadas ao sprite */
  asm sub linh,cx                       /* linh=linh-32+(y0+32) */
  asm mov ax,cx                         /* ax=32-(y0+32) */
  asm imul word ptr col                 /* ax=(32-(y0+32))*col: nß de pixels das linhas do topo do sprite que n∆o v∆o aparecer no ecran */
  asm add si,ax                         /* si=*buf+(32-(y0+32))*col: actualizaá∆o da posiá∆o de in°cio do *buf */
  asm mov ax,YMIN                       /* ax=32 */

ymaior:
  asm push ax                           /* guarda ax que vai ser o valor da coordenada y que pode ser 32 ou y0+24 */
  asm mov cx,linh                       /*cx=linh */
  asm add cx,ax                         /* cx=linh+ax */
  asm cmp cx,YMAX1
  asm jle ymmaior                       /* se linh+ax<=161 salta para ymmaior: o sprite n∆o vai ser truncado por baixo */

               /* ymmenor: o sprite vai ser truncado por baixo */
  asm sub cx,YMAX                       /* cx=linh+ax-160 */
  asm sub linh,cx                       /* linh=ax-160: actualizaá∆o do valor de linhas */

ymmaior:
  asm mov dx,0
  asm mov cx,XMIN
  asm mov ax,x0

  asm cmp ax,cx
  asm jge xmaior                        /* se ax>0 salta para xmaior: o sprite n∆o vai ser truncado no lado esquerdo */

            /* xmenor: o sprite vai ser truncado no lado esquerdo */
  asm sub cx,ax                         /* cx=-x0: notar que x0<0 e -x0>0 */
  asm mov dx,cx                         /* dx=-x0 */
  asm sub col,cx                        /* col=col+x0 */
  asm mov ax,XMIN                       /* ax=0 */
  asm add si,dx                         /* si=*buf+(56-y0)*col-x0: actualizaá∆o da posiá∆o do in°cio do *buf */

xmaior:
  asm push ax                           /* guarda ax que vai ser o valor da coordenada x que pode ser x0 ou 0 */
  asm add ax,col                        /* ax=ax+col */
  asm sub ax,XMAX                       /* ax=ax+col-320 */
  asm jle xmmenor                       /* se ax<=320 salta para xmmenor: o sprite n∆o vai ser truncado no lado direito */

            /* xmmaior: o sprite vai ser truncado no lado direito */
  asm mov dx,ax                         /* dx=ax */
  asm sub col,dx                        /* col=col-ax: actualiza o valor das colunas */

xmmenor:
  asm pop bx                            /* bx=ax guardado em £ltimo: coordenada x do sprite */
  asm pop ax                            /* ax=ax guardado em primeiro: coordenada y do sprite */

                                 /* endereáo */
  asm xchg ah,al                        /* multiplica ax por 256 */
  asm add bx,ax                         /* bx=256*y+x */
  asm shr ax,1
  asm shr ax,1                          /* ax=64*y */
  asm add bx,ax                         /* bx=320*y+x */

//  asm mov ax,0a000h
//  asm mov es,ax

  asm add bx,aux                         /* bx=320*y+x+ax */
  asm sub bx,OFFSETECRAN                /* bx=320*y+x+ax-10240 (10240=32*320 e s∆o as 32 primeiras linhas do ecran que n∆o s∆o usadas no jogo */

  asm cmp linh,0                        /* se linh=0 sa°r */
  asm jle exit
  asm cmp col,0                         /* se col=0 sa°r */
  asm jle exit

  asm mov cx,linh                       /* cx=linh */
loop1:
  asm push cx                           /* guarda cx */
  asm mov cx,col                        /* cx=col */
loop2:
  asm lodsb
  asm cmp al,0                          /* 1 Ç a cor que indica a transparància */
  asm je continuar
  asm mov byte ptr es:[bx],al           /* escreve no ecran virtual o que est† em *buf */
continuar:
  asm inc bx                            /* passa Ö coluna seguinte e repete o ciclo */
  asm loop loop2
  asm pop cx                            /* recupera cx */
  asm add bx,XMAXmXMIN                  /* no fim de cada linha soma 320 */
  asm sub bx,col                        /* e subtrai o valor da coluna */
  asm add si,dx                         /* e acrescenta a si o valor de dx (se dx Ç diferente de 0 o sprite Ç truncado pela esquerda) */
  asm loop loop1
exit:
  asm pop ds
;
}

/*---------------------------------------------------------------------------*/
/*  Là um ficheiro com os dados de um desenho                                */
/*                                                                           */
/*  Parametros:                                                              */
/*    buf Ç o nome do buffer onde vai ficar armazenado o desenho             */
/*    bit Ç o nome do ficheiro que contÇm o desenho                          */
/*---------------------------------------------------------------------------*/
int SPRITE::READ_BIT(unsigned char far *buf,char *bit,unsigned pointer)
{
  FILE *fp;
  unsigned int comp,comp1,comp2,cont;
  unsigned char ch[5];

  if((fp = fopen(bit,"rb"))!=NULL)
  {
    if (fread(ch,(size_t)4, 1, fp) == NULL) return(6);
    buf[0+pointer]=ch[0];
    buf[1+pointer]=ch[1];
    buf[2+pointer]=ch[2];
    buf[3+pointer]=ch[3];

    comp1=(int)(ch[2]+256*ch[3]);
    comp2=(int)(ch[0]+256*ch[1]);
    comp=comp1*comp2;
    for(cont=0;cont<comp;cont++)
    {
      if (fread(ch,(size_t)1, 1, fp) == NULL) return(7);
      buf[cont+4+pointer]=ch[0];
    }
    fclose(fp);
  }
  else
    return(8);
  return(0);
}


/* BOY */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
BOY::BOY(void)
{
  FILE *fp;
  int cont1,cont2;

  DDX=8>>1;                                /* velocidade horizontal do boneco */
  DDY=8>>1;                                /* velocidade vertical do boneco */
  SALT=(8<<1)-2;                               /* altura de salto */
  MAXVIDA=3;                           /* n£mero de vidas inicial */
  MAXENERGIA=10;                        /* energia de cada vida */
  MAXESCUDO=10;
  MAXPODER=10;
  DISDISPARO=20;                        /* alcance do disparo */
  DISPINIC=10;                           /* n£mero de muniá‰es quando se recomeáa a jogar depois de ter perdido uma vida */
  lives=MAXVIDA;
  energy=MAXENERGIA;
  shield=MAXESCUDO;
  poder=MAXPODER;
  amunition=DISPINIC;
  points=0;
  shoot=0;
  step=0;
  jump=0;
  direc=0;
  deita=0;
  x1=64;
  y1=56;

                       /* boneco */
  for(cont1=0;cont1<8;cont1++)
    if((boneco[cont1]=new unsigned char [32*32+4])==NULL) SETERROR(72);
  if(ERROR()==0)
  {
    if(READ_BIT(boneco[0],"b1.bit")!=0) SETERROR(50);
    if(READ_BIT(boneco[1],"b2.bit")!=0) SETERROR(51);
    if(READ_BIT(boneco[2],"b3.bit")!=0) SETERROR(52);
    if(READ_BIT(boneco[3],"b4.bit")!=0) SETERROR(53);
    if(READ_BIT(boneco[4],"b5.bit")!=0) SETERROR(54);
    if(READ_BIT(boneco[5],"b6.bit")!=0) SETERROR(55);
    if(READ_BIT(boneco[6],"b7.bit")!=0) SETERROR(56);
    if(READ_BIT(boneco[7],"b8.bit")!=0) SETERROR(57);
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
BOY::~BOY(void)
{
  int cont;

  for(cont=0;cont<8;cont++)
    delete boneco[cont];
}

/*---------------------------------------------------------------------------*/
/*  Decrementa a energy e se esta for 0                                      */
/*  decrementa uma vida e reposiciona o boneco                               */
/*---------------------------------------------------------------------------*/
int BOY::DECENERGY()
{
  energy--;
  if(energy==0)
  {
    lives--;
    energy=MAXENERGIA;
    x1=64;
    y1=56;
    if(amunition<DISPINIC) amunition=DISPINIC;
    XCOORD=0;YCOORD=4*32;
  }
  return(energy);
 }

/*---------------------------------------------------------------------------*/
/*  Teclado                                                                  */
/*                                                                           */
/*  Parametros:                                                              */
/*    x1 coordenada horizontal actual do boneco                             */
/*    y1 coordenada vertical actual do boneco                                */
/*    *perna tem o valor 1 ou 0 e da° o boneco mexe as pernas                */
/*    *jump tem o valor correspondente a situaáÑo em que se encontra o salto*/
/*    *direc tem o valor 0 se anda para a direita e 2 para a esquerda        */
/*    *deita tem o valor 0 se o boneco est† em pÇ e 4 se est† deitado        */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BOY::KEYWORD(KEYPRESS *tecla, MAP *maps)
{
    static int perna=1;
    int m,x2;
    unsigned char flag1;

    x2=x1;
    flag1=0;
                            /* Baixo              */
                            /* Teclas A ou Cursor */
    if((tecla->KEY(30)==-1 || tecla->KEY(80)==-1) && y1+32<140 && jump==0)
    {
     flag1=1;
     if(deita==0)
     {
       deita=4;
       y1=y1+16;
       x1=x1-direc*8;
     }
    }
                            /* Salto              */
                            /* Teclas Q ou Cursor */
    if((tecla->KEY(16)==-1 || tecla->KEY(72)==-1) && y1+32>32 && jump==0)
    {
      if(deita==0) jump=1;
      else
      {
        deita=0;
        y1=y1-16;
        x1=x1+direc*8;
      }
    }
                            /* Direita            */
                            /* Teclas P ou Cursor */
    if((tecla->KEY(25)==-1 || tecla->KEY(77)==-1) && (XCOORD<maps->XMAP()*32-(XMAX-XMIN) || x1<XMAX-XMIN-96))
    {
      direc=0;
      x1+=DDX;
      if(x1>XMAX-XMIN-96)
      {
        x1-=DDX;
        XCOORD+=DDX;
        m++;
        perna=0;
      }
    }
                            /* Esquerda           */
                            /* Teclas O ou Cursor */
    if((tecla->KEY(24)==-1 || tecla->KEY(75)==-1) && (XCOORD>0 || x1>XMIN+64))
    {
      x1-=DDX;
      direc=2;
      if(x1<XMIN+64)
      {
        x1+=DDX;
        XCOORD-=DDX;
        m++;
        perna=0;
      }
    }
                            /* Disparo            */
                            /* Teclas SPACE       */
    if((tecla->KEY(57)==-1 || tecla->KEY(28)==-1) && shoot==0 && amunition>0)
    {
      amunition--;
      shoot=DISDISPARO;
    }
    if(deita==4 && flag1==0)
    {
      deita=0;
      y1=y1-16;
      x1=x1+direc*8;
    }
    if(x2!=x1 || perna==0)
      step++;
    if(step>1)
      step=0;
    if(perna==0)
      perna=1;
}

/*---------------------------------------------------------------------------*/
/*  Movimento                                                                */
/*                                                                           */
/*  Parametros:                                                              */
/*    x1 coordenada horizontal actual do boneco                             */
/*    y1 coordenada vertical actual do boneco                               */
/*    *jump tem o valor correspondente a situaáÑo em que se encontra o salto*/
/*    direc tem o valor 0 se anda para a direita e 2 para a esquerda         */
/*    deita tem o valor 0 se o boneco est† em pÇ e 4 se est† deitado         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BOY::MOVEMENT(KEYPRESS *tecla,MAP *maps)
{
         /* valmapa Ç o tipo de gr†fico que est† na posiáÑo do boneco */
    int valmapa[4],xmin,xmax,ymindeita,ymin,ymax,x,y,incx,incy;

    incx=0;
    incy=0;
    x=x1+XCOORD;
    y=y1+YCOORD;

    xmin=x-(x/32)*32;
    xmax=xmin+16+deita*4;
    ymin=y-(y/32)*32;
    ymax=ymin+32-deita*4;

    ymindeita=ymin+deita*4;

    valmapa[0]=maps->BUFF(y/32,x/32);
    valmapa[1]=maps->BUFF(y/32,(x+32)/32);
    valmapa[2]=maps->BUFF((y+32)/32,x/32);
    valmapa[3]=maps->BUFF((y+32)/32,(x+32)/32);


    if(jump<SALT && jump>0 && (tecla->KEY(16)==-1 || tecla->KEY(72)==-1))
    {
      jump=jump+1;
      y1-=DDY;
    }
    else
      jump=SALT;
    if(jump==SALT) y1+=DDY;

    if(deita==0)
    {
      if(valmapa[2]==11 && xmax>32)
        incx=32-xmin;

      if(valmapa[3]==11 && xmax>32)
        incx=32-xmax;

      if(((valmapa[2]==1) || (valmapa[3]==1 && xmax>32)) && ymindeita==24)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[2]==2) || (valmapa[3]==2 && xmax>32)) && ymindeita==16)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[2]==3) || (valmapa[3]==3 && xmax>32)) && ymindeita==8)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[2]==10) || (valmapa[3]==10 && xmax>32)) && (ymindeita==16 || ymindeita==24))
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[2]==10) || (valmapa[3]==10 && xmax>32)) && (ymindeita==24) && deita==0)
        incy=32-ymax+16;
    }
    else /* if deita!=0 */
    {
      if(valmapa[0]==11 && xmax>32)
        incx=32-xmin;

      if(valmapa[1]==11 && xmax>32)
        incx=32-xmax;

      if(((valmapa[0]==1) || (valmapa[1]==1 && xmax>32)) && ymindeita==24)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[0]==2) || (valmapa[1]==2 && xmax>32)) && ymindeita==16)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[0]==3) || (valmapa[1]==3 && xmax>32)) && ymindeita==8)
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }
      if(((valmapa[0]==10) || (valmapa[1]==10 && xmax>32)) && (ymindeita==16 || ymindeita==24))
      {
        if(jump==SALT)
        {
          y1-=DDY;
          jump=0;
        }
       }

      if(((valmapa[0]==10) || (valmapa[1]==10)) && xmax>32 &&(ymindeita==24))
        incy=-ymax+16;
    }

    x1+=incx;
    y1+=incy;

    if(y1+32>96 && YCOORD<maps->YMAP()*32-(YMAX-YMIN)) /* ou 96-32 para o fundo se mover com o boneco */
    {
      YCOORD+=DDY;
      y1-=DDY;
    }
    else if(y1+32<64 && YCOORD>0)
    {
      YCOORD-=DDY;
      y1+=DDY;
    }
}

/*---------------------------------------------------------------------------*/
/*  Desenha no ecran virual uma figura                                       */
/*                                                                           */
/*  Parametros:                                                              */
/*    buf Ç buffer onde se encontram as dimensîes e o boneco                 */
/*    x0 e y0 sÑo as coordenadas em pixels                                   */
/*                                                                           */
/*    ⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø    ≥                                                 */
/*    ≥               ≥32  ≥                                                 */
/*    √ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¥    ≥                                                 */
/*    ≥    o         /≥    ≥                                                 */
/*    ≥   /\     ___/ ≥   200   ExplicaáÑo do ecran de jogo                  */
/*    ≥  /||\   /     ≥    ≥                                                 */
/*    ≥   /\   /      ≥    ≥                                                 */
/*    √ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¥    ≥                                                 */
/*    ≥               ≥32  ≥                                                 */
/*    ¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒŸ    ≥                                                 */
/*           320                                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BOY::STORE(unsigned char far *buf,int x0,int y0)
{
  unsigned int col,linh;
  x0=x1;
  y0=y1;
  buf=boneco[step+direc+deita];

  asm push ds
  asm mov ax,y0
  asm mov bx,x0

                                 /* endereáo */
  asm xchg ah,al                        /* ax=256*y */
  asm add bx,ax                         /* bx=256*y+x */
  asm shr ax,1                          /* ax=128*y */
  asm shr ax,1                          /* ax=64*y */
  asm add bx,ax                         /* bx=230*y+x */

//  asm mov ax,0a000h                   /*isto seria necess†rio se */
//  asm mov es,ax                       /* se desejasse escrever na mem¢ria de video */

  asm les ax,dword ptr ds:ecran_mem      /* ds:ax Ç o in°cio do ecran virtual */
  asm add bx,ax                         /* bx=320*y+x+ax */

/* como as coordenadas do boneco s∆o em relaá∆o ao ecran de jogo a pr¢xima linha est† desactivada */
//  asm sub bx,OFFSETECRAN              /* bx=320*y+x+ax-10240. 10240=32*320 pois sÑo as 32 primeiras linhas do ecran que nÑo sÑo usadas */

  asm lds si,buf                 /* si offset buffer na RAM */

                          /* là o n£mero de linhas */
  asm lodsw
  asm mov linh,ax

                          /* là o n£mero de colunas */
  asm lodsw
  asm mov col,ax

  asm mov cx,linh
loop1:
  asm push cx                           /* guarda o n£mero de linhas */
  asm mov cx,col
loop2:
  asm lodsb                             /* al=pr¢ximo byte de imagem de buf */
  asm cmp al,0                          /* 1 Ç a cor que indica a transparància */
  asm je continuar
  asm mov byte ptr es:[bx],al           /* escreve no ecran virtual o que se encontra em buf */
continuar:
  asm inc bx
  asm loop loop2                        /* passa Ö coluna seguinte e repete o ciclo */
  asm pop cx
  asm add bx,XMAXmXMIN                  /* no fim de uma linha tem de somar 320 */
  asm sub bx,col                        /* e subtra°r o valor da coluna */
  asm loop loop1                        /* passa Ö linha seguinte e repete o ciclo */
  asm pop ds
}

/*---------------------------------------------------------------------------*/
/*  Mostra as informaáîes relativas ao boneco                                */
/*---------------------------------------------------------------------------*/
void BOY::INFO(void)
{
  cout<<setfill('0');
  gotoxy(14,22);
  cout<<setw(2)<<lives;
  gotoxy(14,23);
  cout<<setw(2)<<energy;
  gotoxy(29,22);
  cout<<setw(6)<<points;
  gotoxy(29,23);
  cout<<setw(3)<<amunition;
}


/* MONSTERIMAGES */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
MONSTERIMAGES::MONSTERIMAGES(int num, char *name, char *mons1, char *mons2, char *mons3, char *mons4)
{
  FILE *fp;
  int cont1,cont2;

  if(num==0)
  {
    flag=0;
    MLENGTH();
    if((fp = fopen(name,"rb"))!=NULL)
    {
      if((desloca=new DESLOC [NUM_DESLOC])==NULL) SETERROR(74);

      for(cont1=0;cont1<NUM_DESLOC;cont1++)
      {
        if((desloca[cont1].desloc=new unsigned char [MAX_DESLOC])==NULL) SETERROR(75);
        if((desloca[cont1].image=new unsigned char [MAX_DESLOC])==NULL) SETERROR(76);
      }

      for(cont1=0;cont1<NUM_DESLOC;cont1++)
      {
        for(cont2=0;cont2<MAX_DESLOC;cont2++)
        {
          desloca[cont1].desloc[cont2]=getc(fp)-48;
        }
        getc(fp);
        getc(fp);
        for(cont2=0;cont2<MAX_DESLOC;cont2++)
        {
          desloca[cont1].image[cont2]=getc(fp)-48;
        }
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
      }
      fclose(fp);
    }
    else
      SETERROR(5);

                                   /* monstro */
    for(cont1=0;cont1<4;cont1++)
      if((monstro[cont1]=new unsigned char [32*16+4])==NULL) SETERROR(76);
    if(ERROR()==0)
    {
      if(READ_BIT(monstro[0],mons1)!=0) SETERROR(18);
      if(READ_BIT(monstro[1],mons2)!=0) SETERROR(19);
      if(READ_BIT(monstro[2],mons3)!=0) SETERROR(20);
      if(READ_BIT(monstro[3],mons4)!=0) SETERROR(21);
    }
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
MONSTERIMAGES::~MONSTERIMAGES(void)
{
  int cont;

  if(flag==0)
  {
    flag=1;
    for(cont=0;cont<4;cont++)
      delete monstro[cont];
    for(cont=0;cont<NUM_DESLOC;cont++)
    {
      delete desloca[cont].desloc;
      delete desloca[cont].image;
    }
    delete desloca;
  }
}


void MONSTERIMAGES::MLENGTH(char *name)
{
  FILE *fp;
  char ch;
  int cont1=0,aux,cont2=0;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    do
    {
      ch=getc(fp);
      cont1++;
    }while(ch!='\r');
    MAX_DESLOC=cont1-1;
    for(aux=0;aux<cont1+1;aux++) getc(fp);
    do
    {
      for(aux=0;aux<cont1<<1;aux++) getc(fp);
      cont2++;
    }while(!feof(fp));
    NUM_DESLOC=cont2;
    fclose(fp);
  }
}

/* MONSTER */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void MONSTER::SET(int num,char *name)
{
  FILE *fp;
  int cont1,cont2;

  counter=0;
  if((fp = fopen(name,"rb"))!=NULL)
  {
    for(cont1=0;cont1<num;cont1++)
    {
      movel.exist=getc(fp)-48;
      getc(fp);
      movel.kind=getc(fp)-48;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      movel.x=cont2;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      movel.y=cont2;
      getc(fp);
      cont2=(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      movel.vel=cont2;
      getc(fp);
      cont2=(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      movel.pointer=cont2;
      getc(fp);
      cont2=(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      movel.shift=cont2;
      getc(fp);
      getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(4);
}

/*---------------------------------------------------------------------------*/
/*  Detecta as colisîes e actualiza as respectivas vari†veis                 */
/*---------------------------------------------------------------------------*/
void MONSTER::COLISION(BOY *boy,MUSIC *musica)
{
  int cont1,aux,aux2,aux3,dx1,dy1,dx,dy;
  long x,y;

  dx1=16+boy->DEITA()*4;
  dy1=32-boy->DEITA()*4;
                                  /* moveis */

    if(movel.exist==0) return;/* se nÑo existe passa ao seguinte */
    aux2=movel.pointer;
    aux3=movel.shift;
    x=movel.x-XCOORD+4;          /* coordenada horizontal em relaáÑo ao ecran de jogo*/
    y=movel.y-YCOORD+4;          /* coordenada horizontal em relaáÑo ao ecran de jogo*/
    dy=(int)monstro[desloca[aux3].image[aux2]][0]-8;
    dx=(int)monstro[desloca[aux3].image[aux2]][2]-8;
    if(((boy->X1()<=x)&&(boy->Y1()<=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()+dy1>=y))||
      ((boy->X1()<=x)&&(boy->Y1()>=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()<=y+dy))||
      ((boy->X1()>=x)&&(boy->Y1()<=y)&&(boy->X1()<=x+dx)&&(boy->Y1()+dy1>=y))||
      ((boy->X1()>=x)&&(boy->Y1()>=y)&&(boy->X1()<=x+dx)&&(boy->Y1()<=y+dy)))
      {
       musica->MUSIC2SOM(0);
       if(boy->DECENERGY()==0) return;
      }
}

/*---------------------------------------------------------------------------*/
/*  Controla a posiáÑo dos objectos m¢veis e est†ticos                       */
/*---------------------------------------------------------------------------*/
void MONSTER::OBJECTS(void)
{
  int cont1,aux,aux2,aux3;
  long x,y;

                                  /* moveis */
counter++;
if(counter==2) counter=0;

    if(movel.exist==0) return;/* se nÑo existe passa ao seguinte */
    x=movel.x-XCOORD;            /* coordenada horizontal em relaáÑo ao ecran de jogo*/
    y=movel.y-YCOORD;            /* coordenada horizontal em relaáÑo ao ecran de jogo*/
    aux2=movel.pointer;
    aux3=movel.shift;
    if(!((int)x<-16 || (int)x>=(int)XMAX || (int)y<-32 || (int)y>=(int)(XMAX-XMIN+8)))
      STORE(monstro[desloca[aux3].image[aux2]],x,y);           /* se se encontra no ecran desenha-o */
//    else continue;                    /* se esta linha estiver activa s¢ se mexem os bonecos que se encontrem no ecran, tornando o programa mais r†pido */
if(counter==1)
{
    aux=movel.vel;
    aux2++;
    if(aux2>=MAX_DESLOC || desloca[aux3].desloc[aux2]==9) aux2=0;
    movel.pointer=aux2;
    switch (desloca[aux3].desloc[aux2]) /* de acordo com o valor assim ser† o sentido do deslocamneto */
    {
      case 1: y=y-aux;                  /* cima */
              break;
      case 2: y=y-aux;                  /* cima e direita */
              x=x+aux;
              break;
      case 3: x=x+aux;                  /* direita */
              break;
      case 4: y=y+aux;                  /* baixo e direita */
              x=x+aux;
              break;
      case 5: y=y+aux;                  /* baixo */
              break;
      case 6: y=y+aux;                  /* baixo e esquerda */
              x=x-aux;
              break;
      case 7: x=x-aux;                  /* esquerda */
              break;
      case 8: y=y-aux;                  /* cima e esquerda */
              x=x-aux;
              break;
    }
    movel.x=x+XCOORD;            /* reajusta o valor da coordenada horizontal */
    movel.y=y+YCOORD;            /* reajusta o valor da coordenada vertical */
}

}


/* WEAPONS */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
WEAPONS::WEAPONS(int num, char *name)
{
  int cont1;

  VELDISPARO=8;                         /* velocidade de disparo */
  xdis=0;
  ydis=0;
  if(num==0)
  {
    flag=0;
    if((atira=new unsigned char [8*8+4+1])==NULL) SETERROR(73);
    if(ERROR()==0)
        if(READ_BIT(atira,name)!=0) SETERROR(12);
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
WEAPONS::~WEAPONS(void)
{
  int cont;

  if(flag==0)
  {
    flag=1;
    delete atira;
  }
}

/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void WEAPONS::SET(int num,int value, char *name)
{
  FILE *fp;
  int cont=0,cont1,cont2;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    for(cont1=0;cont<num;cont1++)
    {
      stop.exist=getc(fp)-48;
      getc(fp);
      stop.kind=getc(fp)-48;
      if(stop.kind==value) cont++;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.x=cont2;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.y=cont2;
      getc(fp);
      getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(3);

}

/*---------------------------------------------------------------------------*/
/*  Calcula os disparos e calcula os choques e desenha no ecran virtual      */
/*    direc tem o valor 0 se anda para a direita e 2 para a esquerda         */
/*---------------------------------------------------------------------------*/
void WEAPONS::SHOOT(BOY *boy)
{
  static unsigned char direcdis=0;
  int xx,yy,x,y;
  int cont1,aux,aux2,aux3,dxx,dyy,dx,dy;

  if(boy->ENDSHOOT())
  {
    direcdis=boy->DIREC();
    xdis=boy->X1()+XCOORD+4*boy->DIREC()+(2-boy->DIREC())*boy->DEITA();
    ydis=boy->Y1()+YCOORD+16-boy->DEITA()*4;
  }
  if(boy->SHOOT()>0)
  {
    boy->DECSHOOT();
    xdis=xdis-(direcdis-1)*VELDISPARO;
    xx=xdis-XCOORD;
    if((int)xx<-16 || (int)xx>=XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
    yy=ydis-YCOORD;
    if((int)yy<-16 || (int)yy>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */
    STORE(atira,xx,yy);
  }
}

/*---------------------------------------------------------------------------*/
/*  Calcula os disparos e calcula os choques e desenha no ecran virtual      */
/*    direc tem o valor 0 se anda para a direita e 2 para a esquerda         */
/*---------------------------------------------------------------------------*/
void WEAPONS::CRASH(MONSTER *monstros,BOY *boy,MUSIC *musica)
{
  int xx,yy,x,y;
  int cont1,aux,aux2,aux3,dxx,dyy,dx,dy;

  if(boy->SHOOT()>0)
  {
    xx=xdis-XCOORD;
    yy=ydis-YCOORD;
    if((int)yy<-16 || (int)yy>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */

    dyy=8;
    dxx=8;
                                  /* moveis */

      if(monstros->EXIST()==0) return;                      /* se nÑo existe passa ao seguinte */
      aux2=monstros->POINTER();;
      aux3=monstros->SHIFT();
      x=monstros->X()-XCOORD+4;        /* coordenada horizontal em relaáÑo ao ecran de jogo*/
      y=monstros->Y()-YCOORD+4;        /* coordenada horizontal em relaáÑo ao ecran de jogo*/
      dy=(int)monstros->MONST(aux3,aux2,0)-8;
      dx=(int)monstros->MONST(aux3,aux2,2)-8;
      if(((xx<=x)&&(yy<=y)&&(xx+dxx>=x)&&(yy+dyy>=y))||
        ((xx<=x)&&(yy>=y)&&(xx+dxx>=x)&&(yy<=y+dy))||
        ((xx>=x)&&(yy<=y)&&(xx<=x+dx)&&(yy+dyy>=y))||
        ((xx>=x)&&(yy>=y)&&(xx<=x+dx)&&(yy<=y+dy)))
        {
          musica->MUSIC2SOM(2);
          monstros->RESETEXIST();
          boy->RESETSHOOT();
          boy->INCPOINTS(20);
          return;
        }
  }
}

/*---------------------------------------------------------------------------*/
/*  Detecta as colisîes e actualiza as respectivas vari†veis                 */
/*---------------------------------------------------------------------------*/
void WEAPONS::COLISION(BOY *boy,MUSIC *musica)
{
  int cont1,aux,aux2,aux3,dx1,dy1,dx,dy;
  long x,y;

  dx1=16+boy->DEITA()*4;
  dy1=32-boy->DEITA()*4;
                                 /* parados */
  if(stop.kind!=0) return;
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD+4;
  if((int)x<-16 || (int)x>=XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD+4;
  if((int)y<-16 || (int)y>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  dy=(int)atira[0]-8;
  dx=(int)atira[2]-8;
  if(stop.kind==0)
  {
    if(((boy->X1()<=x)&&(boy->Y1()<=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()+dy1>=y))||
      ((boy->X1()<=x)&&(boy->Y1()>=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()<=y+dy))||
      ((boy->X1()>=x)&&(boy->Y1()<=y)&&(boy->X1()<x+dx)&&(boy->Y1()+dy1>=y))||
      ((boy->X1()>=x)&&(boy->Y1()>=y)&&(boy->X1()<x+dx)&&(boy->Y1()<y+dy)))
      {
       musica->MUSIC2SOM(1);
       boy->INCAMNO();
       stop.exist=0;
      }
  }
}

/*---------------------------------------------------------------------------*/
/*  Controla a posiáÑo dos objectos m¢veis e est†ticos                       */
/*---------------------------------------------------------------------------*/
void WEAPONS::OBJECTS(void)
{
  int cont1,aux,aux2,aux3;
  long x,y;

                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD;
  if((int)x<-16 || (int)x>=(int)XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD;
  if((int)y<-32 || (int)y>=(int)(YMAX-YMIN+8)) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  if(stop.kind==0) STORE(atira,x,y); /* desenha atira */
}


/* BONUS */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
BONUS::BONUS(int num, char *name1, char *name2, char *name3)
{
  int cont1;

  if(num==0)
  {
    flag=0;
                                    /* parado */
    for(cont1=0;cont1<3;cont1++)
      if((bonus[cont1]=new unsigned char [8*8+4+1])==NULL) SETERROR(73);
    if(ERROR()==0)
    {
      if(READ_BIT(bonus[0],name1)!=0) SETERROR(13);
      if(READ_BIT(bonus[1],name2)!=0) SETERROR(14);
      if(READ_BIT(bonus[2],name3)!=0) SETERROR(15);
    }
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
BONUS::~BONUS(void)
{
  int cont;

  if(flag==0)
  {
    flag=1;
    for(cont=0;cont<3;cont++)
      delete bonus[cont];
  }
}

/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void BONUS::SET(int num,int lowvalue, int uppervalue, char *name)
{
  FILE *fp;
  int cont=0,cont1,cont2;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    for(cont1=0;cont<num;cont1++)
    {
      stop.exist=getc(fp)-48;
      getc(fp);
      stop.kind=getc(fp)-48;
      if(stop.kind>=lowvalue && stop.kind<=uppervalue) cont++;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.x=cont2;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.y=cont2;
      getc(fp);
      getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(3);
}

/*---------------------------------------------------------------------------*/
/*  Detecta as colisîes e actualiza as respectivas vari†veis                 */
/*---------------------------------------------------------------------------*/
void BONUS::COLISION(BOY *boy,MUSIC *musica)
{
  int cont1,aux,aux2,aux3,dx1,dy1,dx,dy;
  long x,y;

  dx1=16+boy->DEITA()*4;
  dy1=32-boy->DEITA()*4;
                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD+4;
  if((int)x<-16 || (int)x>=XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD+4;
  if((int)y<-16 || (int)y>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  {
    dy=(int)bonus[0][0]-8;
    dx=(int)bonus[0][2]-8;
  }
  if(((boy->X1()<=x)&&(boy->Y1()<=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()<=x)&&(boy->Y1()>=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()<=y+dy))||
    ((boy->X1()>=x)&&(boy->Y1()<=y)&&(boy->X1()<x+dx)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()>=x)&&(boy->Y1()>=y)&&(boy->X1()<x+dx)&&(boy->Y1()<y+dy)))
    {
     musica->MUSIC2SOM(1);
     boy->INCPOINTS(stop.kind*stop.kind);
     stop.exist=0;
    }
}

/*---------------------------------------------------------------------------*/
/*  Controla a posiáÑo dos objectos m¢veis e est†ticos                       */
/*---------------------------------------------------------------------------*/
void BONUS::OBJECTS(void)
{
  int cont1,aux,aux2,aux3;
  long x,y;

                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD;
  if((int)x<-16 || (int)x>=(int)XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD;
  if((int)y<-32 || (int)y>=(int)(YMAX-YMIN+8)) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  STORE(bonus[stop.kind-1],x,y);        /* desenha bonus */
}


/* SHARE */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
SHARE::SHARE(int num,char *name)
{
  int cont1;

  if(num==0)
  {
    flag=0;
                                    /* parado */
    if((share=new unsigned char [16*16+4+1])==NULL) SETERROR(73);
    if(ERROR()==0)
      if(READ_BIT(share,name)!=0) SETERROR(13);
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
SHARE::~SHARE(void)
{
  int cont;

  if(flag==0)
  {
    flag=1;
      delete share;
  }
}

/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void SHARE::SET(int num, int value, char *name)
{
  FILE *fp;
  int cont=0,cont1,cont2;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    for(cont1=0;cont<num;cont1++)
    {
      stop.exist=getc(fp)-48;
      getc(fp);
      stop.kind=getc(fp)-48;
      if(stop.kind==value) cont++;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.x=cont2;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.y=cont2;
      getc(fp);
      getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(3);
}

/*---------------------------------------------------------------------------*/
/*  Detecta as colisîes e actualiza as respectivas vari†veis                 */
/*---------------------------------------------------------------------------*/
void SHARE::COLISION(BOY *boy,MUSIC *musica)
{
  int cont1,aux,aux2,aux3,dx1,dy1,dx,dy;
  long x,y;

  dx1=16+boy->DEITA()*4;
  dy1=32-boy->DEITA()*4;
                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD+4;
  if((int)x<-16 || (int)x>=XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD+4;
  if((int)y<-16 || (int)y>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  {
    dy=(int)share[0]-8;
    dx=(int)share[2]-8;
  }
  if(((boy->X1()<=x)&&(boy->Y1()<=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()<=x)&&(boy->Y1()>=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()<=y+dy))||
    ((boy->X1()>=x)&&(boy->Y1()<=y)&&(boy->X1()<x+dx)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()>=x)&&(boy->Y1()>=y)&&(boy->X1()<x+dx)&&(boy->Y1()<y+dy)))
    {
     musica->MUSIC2SOM(1);
     boy->INCPOINTS(stop.kind*stop.kind);
     stop.exist=0;
    }
}

/*---------------------------------------------------------------------------*/
/*  Controla a posiáÑo dos objectos m¢veis e est†ticos                       */
/*---------------------------------------------------------------------------*/
void SHARE::OBJECTS(void)
{
  int cont1,aux,aux2,aux3;
  long x,y;

                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD;
  if((int)x<-16 || (int)x>=(int)XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD;
  if((int)y<-32 || (int)y>=(int)(YMAX-YMIN+8)) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  STORE(share,x,y);        /* desenha share */
}


/* GIFTS */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
GIFTS::GIFTS(int num, char *name)
{
  int cont1;

  if(num==0)
  {
    flag=0;
                                    /* parado */
    if((prenda=new unsigned char [32*16+4+1])==NULL) SETERROR(73);
    if(ERROR()==0)
      if(READ_BIT(prenda,name)!=0) SETERROR(13);
  }
}

/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
GIFTS::~GIFTS(void)
{
  int cont;

  if(flag==0)
  {
    flag=1;
    delete prenda;
  }
}

/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void GIFTS::SET(int num,int value, char *name)
{
  FILE *fp;
  int cont=0,cont1,cont2;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    for(cont1=0;cont<num;cont1++)
    {
      stop.exist=getc(fp)-48;
      getc(fp);
      stop.kind=getc(fp)-48;
      if(stop.kind==value) cont++;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.x=cont2;
      getc(fp);
      cont2=(getc(fp)-48);
      cont2*=10000;
      cont2=cont2+(getc(fp)-48);
      cont2*=1000;
      cont2=cont2+(getc(fp)-48)*100;
      cont2=cont2+(getc(fp)-48)*10;
      cont2=cont2+getc(fp)-48;
      stop.y=cont2;
      getc(fp);
      getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(3);
}

/*---------------------------------------------------------------------------*/
/*  Detecta as colisîes e actualiza as respectivas vari†veis                 */
/*---------------------------------------------------------------------------*/
void GIFTS::COLISION(BOY *boy,MUSIC *musica)
{
  int cont1,aux,aux2,aux3,dx1,dy1,dx,dy;
  long x,y;

  dx1=16+boy->DEITA()*4;
  dy1=32-boy->DEITA()*4;
                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD+4;
  if((int)x<-16 || (int)x>=XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD+4;
  if((int)y<-16 || (int)y>=YMAX-YMIN+8) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  {
    dy=(int)prenda[0]-8;
    dx=(int)prenda[2]-8;
  }
  if(((boy->X1()<=x)&&(boy->Y1()<=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()<=x)&&(boy->Y1()>=y)&&(boy->X1()+dx1>=x)&&(boy->Y1()<=y+dy))||
    ((boy->X1()>=x)&&(boy->Y1()<=y)&&(boy->X1()<x+dx)&&(boy->Y1()+dy1>=y))||
    ((boy->X1()>=x)&&(boy->Y1()>=y)&&(boy->X1()<x+dx)&&(boy->Y1()<y+dy)))
    {
     musica->MUSIC2SOM(1);
     boy->INCPOINTS(stop.kind*stop.kind);
     stop.exist=0;
    }
}

/*---------------------------------------------------------------------------*/
/*  Controla a posiáÑo dos objectos m¢veis e est†ticos                       */
/*---------------------------------------------------------------------------*/
void GIFTS::OBJECTS(void)
{
  int cont1,aux,aux2,aux3;
  long x,y;

                                 /* parados */
  if(stop.exist==0) return;                       /* se nÑo existe passa ao seguinte */
  x=stop.x-XCOORD;
  if((int)x<-16 || (int)x>=(int)XMAX) return;                    /* se nÑo se encontra no ecran passa ao seguinte */
  y=stop.y-YCOORD;
  if((int)y<-32 || (int)y>=(int)(YMAX-YMIN+8)) return;             /* se nÑo se encontra no ecran passa ao seguinte */
  STORE(prenda,x,y);        /* desenha prenda */
}


/* BORDER */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
BORDER::BORDER(char *fundo,int x0,int y0)
{
  unsigned char far *fund;

  if((fund=new unsigned char [10244])==NULL) SETERROR(9);
                                  /* fundo */
  if(READ_BIT(fund,fundo)!=0) SETERROR(10);
  STORE((unsigned char*)fund,x0,y0);
  delete fund;
}

/*---------------------------------------------------------------------------*/
/*  Desenha na mem¢ria de v°deo uma imagem                                   */
/*                                                                           */
/*  Parametros:                                                              */
/*    buf Ç buffer onde se encontram as dimensîes e a imagem                 */
/*    x0 e y0 sÑo as coordenadas em pixels                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BORDER::STORE(unsigned char far *buf,int x0,int y0)
{
  int col,linh;

  asm mov ax,y0
  asm mov bx,x0

                                 /* endereáo */
  asm xchg ah,al                        /* ax=256*y */
  asm add bx,ax                         /* bx=256*y+x */
  asm shr ax,1                          /* ax=128*y */
  asm shr ax,1                          /* ax=64*y */
  asm add bx,ax                         /* bx=230*y+x */
  asm mov ax,0a000h                     /* ax=zona da mem¢ria de v°deo */
  asm mov es,ax                         /* es=zona da mem¢ria de v°deo */

//  asm cli
  asm push ds
  asm lds si,buf                 /* ds:si Ç o in°cio do buffer */

                         /* là o n£mero de linhas */
  asm lodsw
  asm mov linh,ax

                          /* là o n£mero de colunas */
  asm lodsw
  asm mov col,ax

  asm mov cx,linh
loop1:
  asm push cx                           /* guarda o n£mero de linhas */
  asm mov cx,col
loop2:
  asm lodsb                             /* al=pr¢ximo byte de imagem de buf */
  asm mov byte ptr es:[bx],al           /* escreve no ecran virtual o que se encontra em buf */
  asm inc bx                            /* passa Ö coluna seguinte e repete o ciclo */
  asm loop loop2
  asm pop cx                            /* no fim de uma linha tem de somar 320 */
  asm add bx,320                        /* e subtra°r o valor da coluna */
  asm sub bx,col
  asm loop loop1                        /* passa Ö linha seguinte e repete o ciclo */
  asm pop ds
//  asm sti
}




/* BACKGROUND */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
BACKGROUND::BACKGROUND(char *fundo)
{
  if((fund=new unsigned char [64004])==NULL) SETERROR(9);
                                  /* fundo */
  if(READ_BIT(fund,fundo)!=0) SETERROR(10);
}

/*---------------------------------------------------------------------------*/
/*  Rotina de finalizaáÑo                                                    */
/*---------------------------------------------------------------------------*/
BACKGROUND::~BACKGROUND(void)
{
  delete fund;
}


/*---------------------------------------------------------------------------*/
/*  Desenha na mem¢ria de v°deo uma imagem                                   */
/*                                                                           */
/*  Parametros:                                                              */
/*    buf Ç buffer onde se encontram as dimensîes e a imagem                 */
/*    x0 e y0 sÑo as coordenadas em pixels                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void BACKGROUND::PAINT(void)
{
  unsigned char far *auxfund;
  unsigned int desloc,aux1,aux2,aux3,aux4;
  long off;

  off=SPRITE::YCOORD/(3+1);    /* (12*32-4*32)/(200-4*32)=3.5555 */
  off*=320;
  desloc=SPRITE::XCOORD>>2;
  desloc=desloc-desloc/320*320;
  auxfund=fund+4+desloc+off;

  aux1=(XMAX-XMIN-desloc);
  aux2=(XMIN+desloc);
  aux3=YMAX-YMIN;
  aux4=XMAX-XMIN;

  asm push ds
  asm les ax,dword ptr ds:ecran_mem
  asm mov di,ax
  asm mov dx,aux3
  asm lds si,dword ptr auxfund      /* local do ecran virtual */
loop1:
  asm mov cx,aux1
  asm cmp cx,0
  asm jz cont1
  asm rep movsb
cont1:
  asm mov cx,aux2                   /* 20480=64linhas*320colunas do ecran de jogo */
  asm cmp cx,0
  asm jz cont2
  asm sub si,aux4
  asm rep movsb                         /* ciclo de transferància */
  asm add si,aux4
cont2:
  asm dec dx
  asm jnz loop1:
  asm pop ds
}


/* MAP */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
MAP::MAP(char *mapa)
{
  FILE *fp;
  unsigned int cont1,cont2;
  char ch;
  unsigned int aux;

  XMAXMAPA=0;
  YMAXMAPA=0;
  LENGTH(mapa);

  aux=LINHxCOL2;
  if((ecran_mem=new int [aux])==NULL) SETERROR(58);       /* 40960=LINHxCOL*2 */

                       /* inicializa *ecran_mem a zero */

                       /*
  asm mov ax,0
  asm mov cx,LINHxCOL
  asm les di,dword ptr ds:ecran_mem
  asm rep stosd
                         */


  if((buf1=new unsigned char [YMAXMAPA*(XMAXMAPA+1)])==NULL) SETERROR(77);
  if((buf2=new unsigned char [YMAXMAPA*(XMAXMAPA+1)])==NULL) SETERROR(78);

  if((fp = fopen(mapa,"rb"))!=NULL)
  {
    for(cont1=0;cont1<YMAXMAPA;cont1++)
    {
      for(cont2=0;cont2<XMAXMAPA;cont2++)
      {
        ch=getc(fp);
        if(ch==46) ch=0;                /* o caracter nß 46 Ç o ponto */
        else ch-=64;                    /* o caracter nß 64 Ç o A */
        buf1[cont1*(XMAXMAPA+1)+cont2]=ch;
      }
      ch=getc(fp);
      ch=getc(fp);
    }
    fclose(fp);
  }
  else
    SETERROR(2);
  for(cont1=0;cont1<YMAXMAPA;cont1++)
  {
    for(cont2=0;cont2<XMAXMAPA;cont2++)
    {
      ch=buf1[cont1*(XMAXMAPA+1)+cont2];
                            /* chao na 1¶ posiáÑo */
      if(ch==18)
        buf2[cont1*(XMAXMAPA+1)+cont2]=3;
                            /* chao na 2¶ posiáÑo */
      else if(ch>20 && ch<25)
        buf2[cont1*(XMAXMAPA+1)+cont2]=2;
                            /* chao na 3¶ posiáÑo */
      else if((ch>8 && ch<12) || (ch>13 && ch<18))
        buf2[cont1*(XMAXMAPA+1)+cont2]=1;
             /* chao na 2¶ posiáÑo e onde nÑo se pode atravessar */
      else if(ch==12)
        buf2[cont1*(XMAXMAPA+1)+cont2]=10;
                    /* bloco onde nÑo se pode atravessar */
      else if(ch==13)
        buf2[cont1*(XMAXMAPA+1)+cont2]=11;
      else buf2[cont1*(XMAXMAPA+1)+cont2]=0;
    }
  }

//  for(cont1=0;cont1<28;cont1++)
    if((enfeite=new unsigned char [28*(32*32+4)])==NULL) SETERROR(79);

                                 /* enfeites */
  if(READ_BIT(enfeite,"ceu1.bit",0)!=0) SETERROR(22);
  if(READ_BIT(enfeite,"nuvem1.bit",1*(32*32+4))!=0) SETERROR(23);
  if(READ_BIT(enfeite,"nuvem2.bit",2*(32*32+4))!=0) SETERROR(24);
  if(READ_BIT(enfeite,"nuvem3.bit",3*(32*32+4))!=0) SETERROR(25);
  if(READ_BIT(enfeite,"nuvem4.bit",4*(32*32+4))!=0) SETERROR(26);
  if(READ_BIT(enfeite,"arvore1.bit",5*(32*32+4))!=0) SETERROR(27);
  if(READ_BIT(enfeite,"arvore2.bit",6*(32*32+4))!=0) SETERROR(28);
  if(READ_BIT(enfeite,"arvore3.bit",7*(32*32+4))!=0) SETERROR(29);
  if(READ_BIT(enfeite,"arvore4.bit",8*(32*32+4))!=0) SETERROR(30);
  if(READ_BIT(enfeite,"arvore5.bit",9*(32*32+4))!=0) SETERROR(31);
  if(READ_BIT(enfeite,"arvore6.bit",10*(32*32+4))!=0) SETERROR(32);
  if(READ_BIT(enfeite,"arvore7.bit",11*(32*32+4))!=0) SETERROR(33);
  if(READ_BIT(enfeite,"chao1.bit",12*(32*32+4))!=0) SETERROR(34);
  if(READ_BIT(enfeite,"chao2.bit",13*(32*32+4))!=0) SETERROR(35);
  if(READ_BIT(enfeite,"chao3.bit",14*(32*32+4))!=0) SETERROR(36);
  if(READ_BIT(enfeite,"chao4.bit",15*(32*32+4))!=0) SETERROR(37);
  if(READ_BIT(enfeite,"chao5.bit",16*(32*32+4))!=0) SETERROR(38);
  if(READ_BIT(enfeite,"chao6.bit",17*(32*32+4))!=0) SETERROR(39);
  if(READ_BIT(enfeite,"chao7.bit",18*(32*32+4))!=0) SETERROR(40);
  if(READ_BIT(enfeite,"chao8.bit",19*(32*32+4))!=0) SETERROR(41);
  if(READ_BIT(enfeite,"chao9.bit",20*(32*32+4))!=0) SETERROR(42);
  if(READ_BIT(enfeite,"chao10.bit",21*(32*32+4))!=0) SETERROR(43);
  if(READ_BIT(enfeite,"chao11.bit",22*(32*32+4))!=0) SETERROR(44);
  if(READ_BIT(enfeite,"chao12.bit",23*(32*32+4))!=0) SETERROR(45);
  if(READ_BIT(enfeite,"chao13.bit",24*(32*32+4))!=0) SETERROR(46);
  if(READ_BIT(enfeite,"chao14.bit",25*(32*32+4))!=0) SETERROR(47);
  if(READ_BIT(enfeite,"chao15.bit",26*(32*32+4))!=0) SETERROR(48);
  if(READ_BIT(enfeite,"chao16.bit",27*(32*32+4))!=0) SETERROR(49);
}
/*---------------------------------------------------------------------------*/
/*  Destr¢i as vari†veis alocadas                                            */
/*---------------------------------------------------------------------------*/
MAP::~MAP(void)
{
//  int cont;

  delete ecran_mem;
  delete buf1;
  delete buf2;
  delete enfeite;
//  for(cont=0;cont<28;cont++)
//    delete enfeite[cont];

}

/*---------------------------------------------------------------------------*/
/*  Copia  o ecran virtual para a mem¢ria de video                           */
/*---------------------------------------------------------------------------*/
void MAP::REFRESH(void)
{
  asm mov ax,0a000h                     /* in°cio da memoria de video */
  asm mov es,ax                         /* es Ç o offset da memoria de video */
  asm mov di,OFFSETECRAN                /* offset do ecran do jogo em relaáÑo Ö mem¢ria de video */
  asm mov cx,LINHxCOL                   /* 20480=64linhas*320colunas do ecran de jogo */
//  asm cli                               /* disable interrupáîes para poder usar ds*/
  asm{
    mov bx,ds                         /* guarda o valor de ds */
    lds si,dword ptr ds:ecran_mem      /* local do ecran virtual */
    rep movsw                         /* ciclo de transferància */
    mov ds,bx                         /* restaura o valor de ds */
  }
//  asm sti                               /* enable interrupáîes */
}


/*---------------------------------------------------------------------------*/
/*  Desenha no ecran virual o pano de fundo                                  */
/*---------------------------------------------------------------------------*/
void MAP::ECRAN(void) /* permite qualquer deslocamento */
{
  int xx,yy,x,y,cont1,cont2,linh,col,dist,auxsi,auxsi2,auxbx,auxcx,aux;
  int auxds1,auxds2,auxds3,auxmaxmapa,auxxcoord,auxycoord;
  unsigned char far *auxbuf1;
  unsigned char far *auxenfeite;

  auxbuf1=buf1;
  auxenfeite=enfeite;
  auxmaxmapa=XMAXMAPA;
  auxxcoord=XCOORD;
  auxycoord=YCOORD;

  asm push ds
                /* testar a existància de um retraáo do ecran */
  /*
  asm mov dx,3dah
teste:
  asm in al,dx
  asm test al,8
  asm jz teste
  */
                       /* xx,yy=-deslocamento(8,4,2,1) */
                       /* no eixo dos X o deslocamento */
                       /* nÑo pode ser de 1 pixel pois */
                       /* quando se desenha usa-se o   */
                       /* movsw que escreve words em   */
                       /* vez de bytes (ê mais r†pido) */
  asm mov ax,31
  asm and ax,auxxcoord
  asm neg ax
  asm mov xx,ax

  asm mov ax,31
  asm and ax,auxycoord
  asm neg ax
  asm mov yy,ax

              /* para escrever directamente na mem¢ria de v°deo */
/*
  asm mov dist,10240
  asm mov ax,0a000h
  asm mov es,ax
*/
/* se se deseja escrever na mem¢ria de v°deo deve-se anular as 2 linhas seguintes */
  asm les ax,dword ptr ds:ecran_mem      /* si=offset da zona de ecran virtual */
  asm mov dist,ax                       /* dist=ax:offset da zona de ecran virtual */

  asm mov cx,auxxcoord                     /* cx=XCOORD */
  asm mov bx,auxycoord                  /* bx=YCOORD */
  asm push cx
  asm push bx
  asm lds cx,auxenfeite                 /* si=offset enfeite: in°cio dos gr†ficos de fundo */
  asm add cx,4                          /* si=offset+4 */
  asm mov auxsi2,cx                     /* auxsi2=offset+4 */
  asm mov auxds3,ds
  asm pop bx                            /* bx=YCOORD */
  asm pop cx                            /* cx=XCOORD */

  asm sar cx,5                          /* cx=XCOORD/32 */

  asm sar bx,5                          /* bx=YCOORD/32 */

  asm mov ax,auxmaxmapa                   /* ax=100: tamanho x do mapa de jogo */
//  _AX=XMAXMAPA;
  asm inc ax                            /* ax=101 */
  asm imul bx                           /* ax=100*YCOORD/32 */
  asm add ax,cx                         /* ax=100*YCOORD/32+XCOORD/32 */
  asm lds si,auxbuf1                    /* si=offset *buf1 */
  asm mov auxds2,ds
  asm add si,ax                         /* si=offset *buf1 +100*YCOORD/32+XCOORD/32: si aponta para o gr†fico no mapa de jogo */
  asm mov bx,YMAXECRAN                  /* bx=5: nß m†ximo de linhas de gr†ficos que podem estar no ecran */

loop12:
  asm mov cx,XMAXECRAN                  /* cx=11: nß m†ximo de colunas de gr†ficos que podem estar no ecran */

loop22:
  asm mov ds,auxds2
  asm lodsb                             /* al=byte que se encontra em DS:SI */
  asm mov ds,auxds3

  asm mov auxbx,bx                      /* auxbx=bx */
  asm mov auxcx,cx                      /* auxcx=cx */
  asm mov auxsi,si                      /* auxsi=si */

  asm cmp bl,1
  asm je testay1                        /* se bx=1 salta para testay1: a £ltima fila pode ser truncada */
  asm cmp bl,YMAXECRAN
  asm je testay2                        /* se bx=5 salta para testay1: a primeira fila pode ser truncada */

testaxx:
  asm cmp cl,1
  asm je corta                          /* se cx=1 salta para corta: a 1¶ clouna pode ser truncada */
  asm cmp cl,XMAXECRAN
  asm jne storebitb                     /* se cx!=11 salta para storebitb */

                       /* Se for a 1¶ ou a 11¶ colunas */
corta:
  asm mov dx,xx                         /* dx=xx */
  asm cmp dx,0
  asm je storebitb                      /* se xx=0 salta para storebitb pois n∆o h† truncagem */
  asm cmp dx,320
  asm je conti                          /* se xx=320 salta para conti para passar Ö linha seguinte */
  asm jmp sb                            /* salta para sb */

                       /* a 5¶ fila pode ser truncada */
testay1:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,128
  asm je conti                          /* se yy=128 salta para conti */
  asm jmp sby                           /* salta para sby */

                       /* a 1¶ fila pode ser truncada */
testay2:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,0
  asm jl sby                            /* se yy<0 salta para sby */
  asm jmp testaxx                       /* salta para testaxx */

               /* escreve no ecran virtual um gr†fico completo */
storebitb:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

  asm mov dx,288                        /* dx=288: 288=320-32 */
  asm mov di,dist                       /* di=dist */
  asm mov ax,16                         /* ax=16 */
  asm mov bx,32                         /* bx=32 */

lloop1:
  asm mov cx,ax                         /* cx=16 */
  asm rep movsw                         /* escreve no ecran virtual 16 words correspondentes aos 32 pixels de uma linha de um gr†fico */
  asm add di,dx                         /* di=di+dx */
  asm dec bx                            /* bx=bx-1 */
  asm jnz lloop1                        /* se bx!=0 salta para lloop1 e desenha outra linha */

conti:
  asm jmp sbb                           /* salta para sbb */

        /* s¢ h† truncagem vertical, ou seja, na coluna nß 1 ou nß 11 */
sb:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                          /* là o n£mero de colunas */
  asm mov ax,32                         /* ax=32 */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist :offset da zona de ecran virtual */

  asm dec cx                            /* cx=cx-1 :11-cx tem o valor da coluna */
  asm jz ultimo                         /* se cx=0 salta para ultimo: truncagem da £ltima coluna */

                      /*xmaior: truncagem da 1¶ coluna */
  asm add ax,dx                         /* ax=ax+xx :dx=xx */
  asm sub si,dx                         /* si=si-xx :salta as primeiras xx colunas do gr†fico */
  asm add dist,dx                       /* dist=dist+xx */
  asm neg dx                            /* dx=-xx: dx contÇm o nß de colunas do gr†fico que foram truncadas */
  asm jmp xmmenor                       /* salta para xmmenor */

                        /* truncagem da £ltima coluna */
ultimo:
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=ax-288 */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenor:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax: bx tem o valor do deslocamento para o ecran virtual */
//  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax: aux fica com o valor da metade do n£mero de colunas */
  asm mov ax,32                         /* ax=32 */

loop1:
  asm mov cx,aux                        /* cx=aux */
  asm rep movsb                         /* escreve no ecran virtual uma linha de um gr†fico */
  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec ax                            /* ax=ax-1 */
  asm jnz loop1                         /* se ax!=0 salta para loop1 e repete o ciclo para desenhar outra linha */
  asm jmp sbb                           /* salta para sbb */

    /* se h† truncagem horizontal e/ou vertical quer na 1¶ ou na 5¶ fila */
sby:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov dx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal dx,1
  asm sal dx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,dx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist */

  asm mov linh, 32                       /* linh=32: o n£mero de linhas, neste caso Ç 32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp bx,1
  asm je ymaiory                        /* se bx=1 (£ltima fila horizontal) salta para ymaiory: bx tem o nß da coluna */
               /* ymenor: o gr†fico pode ser truncado por cima */
  asm add linh,ax                       /* linh=linh+yy: yy Ç menor que 0 */
  asm sal ax,5                          /* ax=yy*32 */
  asm sub si,ax                         /* si=si-yy*32 */
  asm jmp ymmaiory                      /* salta para ymmaiory */

                  /* o gr†fico pode ser truncado por baixo */
ymaiory:
//  asm sub ax,128                        /* ax=yy-128 */
//  asm add ax,32                         /* ax=yy-128+32 */
  asm sub ax,96                         /* ax=yy-96 */
  asm sub linh,ax                       /* linh=linh-yy+96 */
//  asm mov ax,yy

ymmaiory:
  asm mov ax,32                         /* ax=32 */
  asm mov bx,288                        /* bx=288 */
  asm mov dx,0                          /* dx=0 */
  asm mov aux,32                        /* aux=16 */
  asm cmp xx, 0
  asm jl xmaior                         /* se xx<0 salta para xmaior */
  asm cmp xx,288
  asm jle preloop                       /* se xx<=288 salta para preloop */
  asm jmp ultimoy                       /* salta para ultimoy */

               /*xmaior: o gr†fico pode ser truncado por cima */
xmaior:
  asm mov dx,xx                         /* dx=xx */
  asm add ax,dx                         /* ax=ax+xx */
  asm sub si,dx                         /* si=si-xx */
  asm add dist,dx                       /* dist=dist+dx */
  asm neg dx                            /* dx=-xx */
  asm jmp xmmenory                      /* salta para xmmenory */

ultimoy:
  asm mov dx,xx                         /* dx=xx */
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=xx-dx */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenory:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax */
//  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax/2 */

preloop:
  asm mov ax,linh                       /* ax=linh */

loop1y:
  asm mov cx,aux                        /* cx=aux */
  asm rep movsb                         /* escreve no ecran virtual uma linha do gr†fico */
  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec ax                            /* ax=ax-1 */
  asm jnz loop1y                        /* se ax!=0 salta para looply e desenha outra linha */

sbb:
  asm add dist, 32                       /* dist=32 */
  asm mov si,auxsi                      /* si=auxsi */
  asm mov cx,auxcx                      /* cx=auxcx */
  asm mov bx,auxbx                      /* bx=auxbx */

  asm dec cx                            /* cx=auxcx-1 */
  asm jz exit1                          /* se cx=0 salta para exit1 */
  asm add xx, 32                         /* xx=xx+32 */
  asm jmp loop22                        /* salta para loop22 */

exit1:
  asm dec bx                            /* bx=auxbx-1 */
  asm jz exit                           /* se bx=0 salta para exit */
  asm add dist,9888                     /* dist=dist+9888: 9888=31*320-32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp ax,0
  asm jg exit2                          /* se ax>0 salta para exit2 */
  asm mov dx,320                        /* dx=320 */
  asm imul dx                           /* ax=yy*320 */
  asm add dist,ax                       /* dist=dist+ax */

exit2:
  asm add si,89                         /* si=si+89: 89=99-10 */
  asm sub xx,320                        /* xx=xx-320: 320=tamanho do ecran em pixels */
  asm add yy,32                         /* yy=yy+32 */
  asm jmp loop12                        /* salta para loop12 */
exit:
  asm pop ds
;
}

/*---------------------------------------------------------------------------*/
/*  Desenha no ecran virual o pano de fundo                                  */
/*---------------------------------------------------------------------------*/
void MAP::ECRAN2(void) /* s¢ permita deslocamentos superiores a 1 */
{
  int xx,yy,x,y,cont1,cont2,linh,col,dist,auxsi,auxsi2,auxbx,auxcx,aux;
  int auxds1,auxds2,auxds3,auxmaxmapa,auxxcoord,auxycoord;
  unsigned char far *auxbuf1;
  unsigned char far *auxenfeite;

  auxbuf1=buf1;
  auxenfeite=enfeite;
  auxmaxmapa=XMAXMAPA;
  auxxcoord=XCOORD;
  auxycoord=YCOORD;

  asm push ds
                /* testar a existància de um retraáo do ecran */
  /*
  asm mov dx,3dah
teste:
  asm in al,dx
  asm test al,8
  asm jz teste
  */
                       /* xx,yy=-deslocamento(8,4,2,1) */
                       /* no eixo dos X o deslocamento */
                       /* nÑo pode ser de 1 pixel pois */
                       /* quando se desenha usa-se o   */
                       /* movsw que escreve words em   */
                       /* vez de bytes (ê mais r†pido) */
  asm mov ax,31
  asm and ax,auxxcoord
  asm neg ax
  asm mov xx,ax

  asm mov ax,31
  asm and ax,auxycoord
  asm neg ax
  asm mov yy,ax

              /* para escrever directamente na mem¢ria de v°deo */
/*
  asm mov dist,10240
  asm mov ax,0a000h
  asm mov es,ax
*/
/* se se deseja escrever na mem¢ria de v°deo deve-se anular as 2 linhas seguintes */
  asm les ax,dword ptr ds:ecran_mem      /* si=offset da zona de ecran virtual */
  asm mov dist,ax                       /* dist=ax:offset da zona de ecran virtual */

  asm mov cx,auxxcoord                     /* cx=XCOORD */
  asm mov bx,auxycoord                  /* bx=YCOORD */
  asm push cx
  asm push bx
  asm lds cx,auxenfeite                 /* si=offset enfeite: in°cio dos gr†ficos de fundo */
  asm add cx,4                          /* si=offset+4 */
  asm mov auxsi2,cx                     /* auxsi2=offset+4 */
  asm mov auxds3,ds
  asm pop bx                            /* bx=YCOORD */
  asm pop cx                            /* cx=XCOORD */

  asm sar cx,5                          /* cx=XCOORD/32 */

  asm sar bx,5                          /* bx=YCOORD/32 */

  asm mov ax,auxmaxmapa                   /* ax=100: tamanho x do mapa de jogo */
//  _AX=XMAXMAPA;
  asm inc ax                            /* ax=101 */
  asm imul bx                           /* ax=100*YCOORD/32 */
  asm add ax,cx                         /* ax=100*YCOORD/32+XCOORD/32 */
  asm lds si,auxbuf1                    /* si=offset *buf1 */
  asm mov auxds2,ds
  asm add si,ax                         /* si=offset *buf1 +100*YCOORD/32+XCOORD/32: si aponta para o gr†fico no mapa de jogo */
  asm mov bx,YMAXECRAN                  /* bx=5: nß m†ximo de linhas de gr†ficos que podem estar no ecran */

loop12:
  asm mov cx,XMAXECRAN                  /* cx=11: nß m†ximo de colunas de gr†ficos que podem estar no ecran */

loop22:
  asm mov ds,auxds2
  asm lodsb                             /* al=byte que se encontra em DS:SI */
  asm mov ds,auxds3

  asm mov auxbx,bx                      /* auxbx=bx */
  asm mov auxcx,cx                      /* auxcx=cx */
  asm mov auxsi,si                      /* auxsi=si */

  asm cmp bl,1
  asm je testay1                        /* se bx=1 salta para testay1: a £ltima fila pode ser truncada */
  asm cmp bl,YMAXECRAN
  asm je testay2                        /* se bx=5 salta para testay1: a primeira fila pode ser truncada */

testaxx:
  asm cmp cl,1
  asm je corta                          /* se cx=1 salta para corta: a 1¶ clouna pode ser truncada */
  asm cmp cl,XMAXECRAN
  asm jne storebitb                     /* se cx!=11 salta para storebitb */

                       /* Se for a 1¶ ou a 11¶ colunas */
corta:
  asm mov dx,xx                         /* dx=xx */
  asm cmp dx,0
  asm je storebitb                      /* se xx=0 salta para storebitb pois n∆o h† truncagem */
  asm cmp dx,320
  asm je conti                          /* se xx=320 salta para conti para passar Ö linha seguinte */
  asm jmp sb                            /* salta para sb */

                       /* a 5¶ fila pode ser truncada */
testay1:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,128
  asm je conti                          /* se yy=128 salta para conti */
  asm jmp sby                           /* salta para sby */

                       /* a 1¶ fila pode ser truncada */
testay2:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,0
  asm jl sby                            /* se yy<0 salta para sby */
  asm jmp testaxx                       /* salta para testaxx */

               /* escreve no ecran virtual um gr†fico completo */
storebitb:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

  asm mov dx,288                        /* dx=288: 288=320-32 */
  asm mov di,dist                       /* di=dist */
  asm mov ax,16                         /* ax=16 */
  asm mov bx,32                         /* bx=32 */

lloop1:
  asm mov cx,ax                         /* cx=16 */
  asm rep movsw                         /* escreve no ecran virtual 16 words correspondentes aos 32 pixels de uma linha de um gr†fico */
  asm add di,dx                         /* di=di+dx */
  asm dec bx                            /* bx=bx-1 */
  asm jnz lloop1                        /* se bx!=0 salta para lloop1 e desenha outra linha */

conti:
  asm jmp sbb                           /* salta para sbb */

        /* s¢ h† truncagem vertical, ou seja, na coluna nß 1 ou nß 11 */
sb:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                          /* là o n£mero de colunas */
  asm mov ax,32                         /* ax=32 */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist :offset da zona de ecran virtual */

  asm dec cx                            /* cx=cx-1 :11-cx tem o valor da coluna */
  asm jz ultimo                         /* se cx=0 salta para ultimo: truncagem da £ltima coluna */

                      /*xmaior: truncagem da 1¶ coluna */
  asm add ax,dx                         /* ax=ax+xx :dx=xx */
  asm sub si,dx                         /* si=si-xx :salta as primeiras xx colunas do gr†fico */
  asm add dist,dx                       /* dist=dist+xx */
  asm neg dx                            /* dx=-xx: dx contÇm o nß de colunas do gr†fico que foram truncadas */
  asm jmp xmmenor                       /* salta para xmmenor */

                        /* truncagem da £ltima coluna */
ultimo:
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=ax-288 */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenor:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax: bx tem o valor do deslocamento para o ecran virtual */
  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax: aux fica com o valor da metade do n£mero de colunas */
  asm mov ax,32                         /* ax=32 */

loop1:
  asm mov cx,aux                        /* cx=aux */
  asm rep movsw                         /* escreve no ecran virtual uma linha de um gr†fico */
  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec ax                            /* ax=ax-1 */
  asm jnz loop1                         /* se ax!=0 salta para loop1 e repete o ciclo para desenhar outra linha */
  asm jmp sbb                           /* salta para sbb */

    /* se h† truncagem horizontal e/ou vertical quer na 1¶ ou na 5¶ fila */
sby:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov dx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal dx,1
  asm sal dx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,dx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist */

  asm mov linh, 32                       /* linh=32: o n£mero de linhas, neste caso Ç 32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp bx,1
  asm je ymaiory                        /* se bx=1 (£ltima fila horizontal) salta para ymaiory: bx tem o nß da coluna */
               /* ymenor: o gr†fico pode ser truncado por cima */
  asm add linh,ax                       /* linh=linh+yy: yy Ç menor que 0 */
  asm sal ax,5                          /* ax=yy*32 */
  asm sub si,ax                         /* si=si-yy*32 */
  asm jmp ymmaiory                      /* salta para ymmaiory */

                  /* o gr†fico pode ser truncado por baixo */
ymaiory:
//  asm sub ax,128                        /* ax=yy-128 */
//  asm add ax,32                         /* ax=yy-128+32 */
  asm sub ax,96                         /* ax=yy-96 */
  asm sub linh,ax                       /* linh=linh-yy+96 */
//  asm mov ax,yy

ymmaiory:
  asm mov ax,32                         /* ax=32 */
  asm mov bx,288                        /* bx=288 */
  asm mov dx,0                          /* dx=0 */
  asm mov aux, 16                        /* aux=16 */
  asm cmp xx, 0
  asm jl xmaior                         /* se xx<0 salta para xmaior */
  asm cmp xx,288
  asm jle preloop                       /* se xx<=288 salta para preloop */
  asm jmp ultimoy                       /* salta para ultimoy */

               /*xmaior: o gr†fico pode ser truncado por cima */
xmaior:
  asm mov dx,xx                         /* dx=xx */
  asm add ax,dx                         /* ax=ax+xx */
  asm sub si,dx                         /* si=si-xx */
  asm add dist,dx                       /* dist=dist+dx */
  asm neg dx                            /* dx=-xx */
  asm jmp xmmenory                      /* salta para xmmenory */

ultimoy:
  asm mov dx,xx                         /* dx=xx */
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=xx-dx */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenory:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax */
  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax/2 */

preloop:
  asm mov ax,linh                       /* ax=linh */

loop1y:
  asm mov cx,aux                        /* cx=aux */
  asm rep movsw                         /* escreve no ecran virtual uma linha do gr†fico */
  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec ax                            /* ax=ax-1 */
  asm jnz loop1y                        /* se ax!=0 salta para looply e desenha outra linha */

sbb:
  asm add dist, 32                       /* dist=32 */
  asm mov si,auxsi                      /* si=auxsi */
  asm mov cx,auxcx                      /* cx=auxcx */
  asm mov bx,auxbx                      /* bx=auxbx */

  asm dec cx                            /* cx=auxcx-1 */
  asm jz exit1                          /* se cx=0 salta para exit1 */
  asm add xx, 32                         /* xx=xx+32 */
  asm jmp loop22                        /* salta para loop22 */

exit1:
  asm dec bx                            /* bx=auxbx-1 */
  asm jz exit                           /* se bx=0 salta para exit */
  asm add dist,9888                     /* dist=dist+9888: 9888=31*320-32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp ax,0
  asm jg exit2                          /* se ax>0 salta para exit2 */
  asm mov dx,320                        /* dx=320 */
  asm imul dx                           /* ax=yy*320 */
  asm add dist,ax                       /* dist=dist+ax */

exit2:
  asm add si,89                         /* si=si+89: 89=99-10 */
  asm sub xx,320                        /* xx=xx-320: 320=tamanho do ecran em pixels */
  asm add yy,32                         /* yy=yy+32 */
  asm jmp loop12                        /* salta para loop12 */
exit:
  asm pop ds
;
}


/*---------------------------------------------------------------------------*/
/*  Desenha no ecran virual o pano de fundo                                  */
/*---------------------------------------------------------------------------*/
void MAP::ECRANBACK(void)
{
  int xx,yy,x,y,cont1,cont2,linh,col,dist,auxsi,auxsi2,auxbx,auxcx,aux;
  int auxax,auxds1,auxds2,auxds3,auxmaxmapa,auxxcoord,auxycoord;
  unsigned char far *auxbuf1;
  unsigned char far *auxenfeite;

  auxbuf1=buf1;
  auxenfeite=enfeite;
  auxmaxmapa=XMAXMAPA;
  auxxcoord=XCOORD;
  auxycoord=YCOORD;

  asm push ds
                /* testar a existància de um retraáo do ecran */
  /*
  asm mov dx,3dah
teste:
  asm in al,dx
  asm test al,8
  asm jz teste
  */
                       /* xx,yy=-deslocamento(8,4,2,1) */
                       /* no eixo dos X o deslocamento */
                       /* nÑo pode ser de 1 pixel pois */
                       /* quando se desenha usa-se o   */
                       /* movsw que escreve words em   */
                       /* vez de bytes (ê mais r†pido) */
  asm mov ax,31
  asm and ax,auxxcoord
  asm neg ax
  asm mov xx,ax

  asm mov ax,31
  asm and ax,auxycoord
  asm neg ax
  asm mov yy,ax

              /* para escrever directamente na mem¢ria de v°deo */
/*
  asm mov dist,10240
  asm mov ax,0a000h
  asm mov es,ax
*/
/* se se deseja escrever na mem¢ria de v°deo deve-se anular as 2 linhas seguintes */
  asm les ax,dword ptr ds:ecran_mem      /* si=offset da zona de ecran virtual */
  asm mov dist,ax                       /* dist=ax:offset da zona de ecran virtual */

  asm mov cx,auxxcoord                     /* cx=XCOORD */
  asm mov bx,auxycoord                  /* bx=YCOORD */
  asm push cx
  asm push bx
  asm lds cx,auxenfeite                 /* si=offset enfeite: in°cio dos gr†ficos de fundo */
  asm add cx,4                          /* si=offset+4 */
  asm mov auxsi2,cx                     /* auxsi2=offset+4 */
  asm mov auxds3,ds
  asm pop bx                            /* bx=YCOORD */
  asm pop cx                            /* cx=XCOORD */

  asm sar cx,5                          /* cx=XCOORD/32 */

  asm sar bx,5                          /* bx=YCOORD/32 */

  asm mov ax,auxmaxmapa                   /* ax=100: tamanho x do mapa de jogo */
//  _AX=XMAXMAPA;
  asm inc ax                            /* ax=101 */
  asm imul bx                           /* ax=100*YCOORD/32 */
  asm add ax,cx                         /* ax=100*YCOORD/32+XCOORD/32 */
  asm lds si,auxbuf1                    /* si=offset *buf1 */
  asm mov auxds2,ds
  asm add si,ax                         /* si=offset *buf1 +100*YCOORD/32+XCOORD/32: si aponta para o gr†fico no mapa de jogo */
  asm mov bx,YMAXECRAN                  /* bx=5: nß m†ximo de linhas de gr†ficos que podem estar no ecran */

loop12:
  asm mov cx,XMAXECRAN                  /* cx=11: nß m†ximo de colunas de gr†ficos que podem estar no ecran */

loop22:
  asm mov ds,auxds2
  asm lodsb                             /* al=byte que se encontra em DS:SI */
  asm mov ds,auxds3

  asm mov auxbx,bx                      /* auxbx=bx */
  asm mov auxcx,cx                      /* auxcx=cx */
  asm mov auxsi,si                      /* auxsi=si */

  asm cmp bl,1
  asm je testay1                        /* se bx=1 salta para testay1: a £ltima fila pode ser truncada */
  asm cmp bl,YMAXECRAN
  asm je testay2                        /* se bx=5 salta para testay1: a primeira fila pode ser truncada */

testaxx:
  asm cmp cl,1
  asm je corta                          /* se cx=1 salta para corta: a 1¶ clouna pode ser truncada */
  asm cmp cl,XMAXECRAN
  asm jne storebitb                     /* se cx!=11 salta para storebitb */

                       /* Se for a 1¶ ou a 11¶ colunas */
corta:
  asm mov dx,xx                         /* dx=xx */
  asm cmp dx,0
  asm je storebitb                      /* se xx=0 salta para storebitb pois n∆o h† truncagem */
  asm cmp dx,320
  asm je conti                          /* se xx=320 salta para conti para passar Ö linha seguinte */
  asm jmp sb                            /* salta para sb */

                       /* a 5¶ fila pode ser truncada */
testay1:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,128
  asm je conti                          /* se yy=128 salta para conti */
auxiliar:
  asm jmp sby                           /* salta para sby */

                       /* a 1¶ fila pode ser truncada */
testay2:
  asm cmp xx,320
  asm je conti                          /* se xx=320 salta para conti */
  asm mov dx,yy                         /* dx=yy */
  asm cmp dx,0
  asm jl auxiliar                            /* se yy<0 salta para sby */
  asm jmp testaxx                       /* salta para testaxx */

               /* escreve no ecran virtual um gr†fico completo */
storebitb:

  asm cmp al,0
  asm jne contin
  asm jmp conti
contin:

  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

  asm mov dx,288                        /* dx=288: 288=320-32 */
  asm mov di,dist                       /* di=dist */
  asm mov ax,32                         /* ax=16 */
  asm mov bx,32                         /* bx=32 */

lloop1:
  asm mov cx,32                         /* cx=16 */

//  asm rep movsb

xlloop1:
  asm lodsb                             /* al=pr¢ximo byte de imagem de buf */
  asm cmp al,0                          /* 1 Ç a cor que indica a transparància */
  asm je continua
  asm mov byte ptr es:[di],al           /* escreve no ecran virtual o que se encontra em buf */
continua:
  asm inc di
  asm loop xlloop1

  asm add di,dx                         /* di=di+dx */
  asm dec bx                            /* bx=bx-1 */
  asm jnz lloop1                        /* se bx!=0 salta para lloop1 e desenha outra linha */

conti:
  asm jmp sbb                           /* salta para sbb */

        /* s¢ h† truncagem vertical, ou seja, na coluna nß 1 ou nß 11 */
sb:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov bx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal bx,1
  asm sal bx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,bx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                          /* là o n£mero de colunas */
  asm mov ax,32                         /* ax=32 */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist :offset da zona de ecran virtual */

  asm dec cx                            /* cx=cx-1 :11-cx tem o valor da coluna */
  asm jz ultimo                         /* se cx=0 salta para ultimo: truncagem da £ltima coluna */

                      /*xmaior: truncagem da 1¶ coluna */
  asm add ax,dx                         /* ax=ax+xx :dx=xx */
  asm sub si,dx                         /* si=si-xx :salta as primeiras xx colunas do gr†fico */
  asm add dist,dx                       /* dist=dist+xx */
  asm neg dx                            /* dx=-xx: dx contÇm o nß de colunas do gr†fico que foram truncadas */
  asm jmp xmmenor                       /* salta para xmmenor */

                        /* truncagem da £ltima coluna */
ultimo:
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=ax-288 */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenor:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax: bx tem o valor do deslocamento para o ecran virtual */
//  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax: aux fica com o valor da metade do n£mero de colunas */
  asm mov auxax,32                         /* ax=32 */

loop1:
  asm mov cx,aux                        /* cx=aux */
//  asm rep movsb                         /* escreve no ecran virtual uma linha de um gr†fico */
xloop1:
  asm lodsb                             /* al=pr¢ximo byte de imagem de buf */
  asm cmp al,0                          /* 1 Ç a cor que indica a transparància */
  asm je xcontinua
  asm mov byte ptr es:[di],al           /* escreve no ecran virtual o que se encontra em buf */
xcontinua:
  asm inc di
  asm loop xloop1

  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec auxax
  asm jnz loop1                         /* se ax!=0 salta para loop1 e repete o ciclo para desenhar outra linha */
  asm jmp sbb                           /* salta para sbb */

    /* se h† truncagem horizontal e/ou vertical quer na 1¶ ou na 5¶ fila */
sby:
  asm xor ah,ah                         /* ah=0      :ax contÇm o valor do gr†fico que deve ser desenhado   */
  asm mov dx,ax                         /* bx=ax      como cada gr†fico ocupa 4+32*32 =4+1024=1028 bytes    */
  asm sal dx,1
  asm sal dx,1                          /* bx=ax*4    tem de se multiplicar ax por esse valor para se ficar */
  asm sal ax,10                         /* ax=ax*1024 a apontar para o gr†fico certo                        */
  asm add ax,dx                         /* ax=ax*1024+ax*4: ax=ax*1028                                      */

  asm mov si,auxsi2                     /* si=auxsi2 */
  asm add si,ax                         /* si=auxsi2+ax*1028 :si aponta para o in°cio do gr†fico a ser desenhado */

                                 /* endereáo */
  asm mov di,dist                       /* di=dist */

  asm mov linh, 32                       /* linh=32: o n£mero de linhas, neste caso Ç 32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp bx,1
  asm je ymaiory                        /* se bx=1 (£ltima fila horizontal) salta para ymaiory: bx tem o nß da coluna */
               /* ymenor: o gr†fico pode ser truncado por cima */
  asm add linh,ax                       /* linh=linh+yy: yy Ç menor que 0 */
  asm sal ax,5                          /* ax=yy*32 */
  asm sub si,ax                         /* si=si-yy*32 */
  asm jmp ymmaiory                      /* salta para ymmaiory */

                  /* o gr†fico pode ser truncado por baixo */
ymaiory:
//  asm sub ax,128                        /* ax=yy-128 */
//  asm add ax,32                         /* ax=yy-128+32 */
  asm sub ax,96                         /* ax=yy-96 */
  asm sub linh,ax                       /* linh=linh-yy+96 */
//  asm mov ax,yy

ymmaiory:
  asm mov ax,32                         /* ax=32 */
  asm mov bx,288                        /* bx=288 */
  asm mov dx,0                          /* dx=0 */
  asm mov aux,32                        /* aux=16 */
  asm cmp xx, 0
  asm jl xmaior                         /* se xx<0 salta para xmaior */
  asm cmp xx,288
  asm jle preloop                       /* se xx<=288 salta para preloop */
  asm jmp ultimoy                       /* salta para ultimoy */

               /*xmaior: o gr†fico pode ser truncado por cima */
xmaior:
  asm mov dx,xx                         /* dx=xx */
  asm add ax,dx                         /* ax=ax+xx */
  asm sub si,dx                         /* si=si-xx */
  asm add dist,dx                       /* dist=dist+dx */
  asm neg dx                            /* dx=-xx */
  asm jmp xmmenory                      /* salta para xmmenory */

ultimoy:
  asm mov dx,xx                         /* dx=xx */
  asm sub dx,288                        /* dx=xx-288 */
                                 /* xmmaior */
  asm sub ax,dx                         /* ax=xx-dx */
  asm sub dist,dx                       /* dist=dist-xx+288 */
  asm add dist, 32                       /* dist=dist-xx+320 */

xmmenory:
  asm mov bx,320                        /* bx=320 */
  asm sub bx,ax                         /* bx=320-ax */
//  asm sar ax,1                          /* ax=ax/2 */
  asm mov aux,ax                        /* aux=ax/2 */

preloop:
  asm mov ax,linh                       /* ax=linh */
  asm mov auxax,ax

loop1y:
  asm mov cx,aux                        /* cx=aux */
//  asm rep movsb                         /* escreve no ecran virtual uma linha do gr†fico */
xloop1y:
  asm lodsb                             /* al=pr¢ximo byte de imagem de buf */
  asm cmp al,0                          /* 1 Ç a cor que indica a transparància */
  asm je xcontinuay
  asm mov byte ptr es:[di],al           /* escreve no ecran virtual o que se encontra em buf */
xcontinuay:
  asm inc di
  asm loop xloop1y

  asm add di,bx                         /* di=di+bx */
  asm add si,dx                         /* si=si+dx */
  asm dec auxax                            /* ax=ax-1 */
  asm jnz loop1y                        /* se ax!=0 salta para looply e desenha outra linha */

sbb:
  asm add dist, 32                       /* dist=32 */
  asm mov si,auxsi                      /* si=auxsi */
  asm mov cx,auxcx                      /* cx=auxcx */
  asm mov bx,auxbx                      /* bx=auxbx */

  asm dec cx                            /* cx=auxcx-1 */
  asm jz exit1                          /* se cx=0 salta para exit1 */
  asm add xx, 32                         /* xx=xx+32 */
  asm jmp loop22                        /* salta para loop22 */

exit1:
  asm dec bx                            /* bx=auxbx-1 */
  asm jz exit                           /* se bx=0 salta para exit */
  asm add dist,9888                     /* dist=dist+9888: 9888=31*320-32 */
  asm mov ax,yy                         /* ax=yy */
  asm cmp ax,0
  asm jg exit2                          /* se ax>0 salta para exit2 */
  asm mov dx,320                        /* dx=320 */
  asm imul dx                           /* ax=yy*320 */
  asm add dist,ax                       /* dist=dist+ax */

exit2:
  asm add si,89                         /* si=si+89: 89=99-10 */
  asm sub xx,320                        /* xx=xx-320: 320=tamanho do ecran em pixels */
  asm add yy,32                         /* yy=yy+32 */
  asm jmp loop12                        /* salta para loop12 */
exit:
  asm pop ds
;
}


void MAP::LENGTH(char *name)
{
  FILE *fp;
  int cont=0;
  char ch;
  long length=0;

  if((fp = fopen(name,"rb"))!=NULL)
  {
    do
    {
      ch=getc(fp);
      cont++;
    }while(ch!='\r');
    XMAXMAPA=cont-1;
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fclose(fp);
  }
  length/=(XMAXMAPA);
  YMAXMAPA=(int)length;
}


/* ERROR */


/*---------------------------------------------------------------------------*/
/*  Rotina de inicializaáÑo                                                  */
/*---------------------------------------------------------------------------*/
void ERROR(int coderror)
{
  cout<<"Erro #"<<coderror<<endl;
  switch (coderror)
  {
    case 1:
          cout<<"Erro na leitura da palette \n";
          break;
    case 2:
          cout<<"Erro na leitura do mapa \n";
          break;
    case 3:
          cout<<"Erro na leitura do parados \n";
          break;
    case 4:
          cout<<"Erro na leitura do moveis \n";
          break;
    case 5:
          cout<<"Erro na leitura do deslocamentos \n";
          break;
    case 6:
          cout<<"Erro na leitura de bit-map \n";
          break;
    case 7:
          cout<<"Erro na leitura de bit-map \n";
          break;
    case 8:
          cout<<"Erro na leitura de bit-map \n";
          break;
    case 9:
          cout<<"Falha de mem¢ria \n";
          break;
    case 10:
          cout<<"Erro na leitura do fundo1\n";
          break;
    case 11:
          cout<<"Erro na leitura do fundo2\n";
          break;
    case 12:
          cout<<"Erro na leitura de parado\n";
          break;
    case 13:
          cout<<"Erro na leitura de parado\n";
          break;
    case 14:
          cout<<"Erro na leitura de parado\n";
          break;
    case 15:
          cout<<"Erro na leitura de parado\n";
          break;
    case 16:
          cout<<"Erro na leitura de parado\n";
          break;
    case 17:
          cout<<"Erro na leitura de parado\n";
          break;
    case 18:
          cout<<"Erro na leitura de monstro\n";
          break;
    case 19:
          cout<<"Erro na leitura de monstro\n";
          break;
    case 20:
          cout<<"Erro na leitura de monstro\n";
          break;
    case 21:
          cout<<"Erro na leitura de monstro\n";
          break;
  }
  if(coderror>=22 && coderror<=49) cout<<"Erro na leitura de enfeite\n";
  if(coderror>=50 && coderror<=57) cout<<"Erro na leitura de boneco\n";
  if(coderror==58) cout<<"Falha de mem¢ria \n";
  if(coderror>=59 && coderror<=71) cout<<"Erro na leitura de musica\n";
}

long LENGTH(int value, char *name)
{
  FILE *fp;
  long length=0;

  if(value!=-1)
  {
    if((fp = fopen(name,"rb"))!=NULL)
    {
      do
      {
        getc(fp);
        getc(fp);
        if(getc(fp)-48==value) length++;
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
        getc(fp);
      }while(!feof(fp));
    fclose(fp);
    }
    length;
  }
  else
  {
    if((fp = fopen(name,"rb"))!=NULL)
    {
      fseek(fp, 0L, SEEK_END);
      length = ftell(fp);
      fclose(fp);
    }
    length/=29;
  }
  return(length);
}

