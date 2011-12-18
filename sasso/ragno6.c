#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

double l1 = 4;
double l2 = 6;
double l3 = 10;

struct leg2 {
   unsigned char mask;
   signed char angolo[6];
};

struct packet {
   struct leg2 coppia[3];
};

#define TODEG (180.0/M_PI)
#define TORAD (M_PI/180.0)
#define SQ(x)  ((x)*(x))

/* Prendo  i valori di x, y, z e restituisco gli angoli a,b,c come int */
int angoli_zampa(double *pos, int *angoli)
{
   double x = pos[0];
   double y = pos[1];
   double z = pos[2];

   double a, b, b_rad, c, r, lx, rif;

   /* a e` il primo angolo: non dip. da z */
   a = TODEG * atan2(y,x);
   /* il resto dipende da rho e theta (lx), non piu` da A */
   r = sqrt(x*x + y*y) - l1;
   lx = sqrt(SQ(z) + SQ(r));
   /* rif e` l'angolo della punta risp. alla verticale */
   rif = atan2(r,z);
   /* b e` l'angolo del secondo motore (teorema di eulero) */
   b_rad = (rif + acos(((pow(lx,2)-pow(l3,2)+pow(l2,2))/(2*l2*lx))));
   b = -90 + TODEG * b_rad;
   c = -TODEG * (asin((lx/l3) * sin(b_rad-rif)));

   /* i tre angoli li ritorno come interi */
   angoli[0] = (int)a;
   angoli[1] = (int)b;
   angoli[2] = (int)c;
   //printf("%d\n%d\n%d\n",angoli[0],angoli[1],angoli[2]);
   return 0;
}

 
int impacchetta_zampa(struct packet *pacc, int nz /* 0..5 */, int *angoli)
{
   int j;
   int mask = 0x7; /* tre bit */
   int ang0 = 0;
   struct leg2 *l2;

   if (nz % 2) {
       mask <<= 3;
       ang0 = 3;
   }
   l2 = pacc->coppia+(nz/2);
   l2->mask |= mask;


   for (j=ang0; j < ang0+3; j++)
   {
       l2->angolo[j]=(char)angoli[j];
   }

   return 0;
}




int sincrono(double *pos, int *angoli)
{
   struct packet pack;
   int i;
   for (i=0; i<6; i++)
   {
      /* riempio la mia zampa i */
      if (angoli_zampa(pos,angoli)) {
         /* .... */
         return 1;
      }
      if (impacchetta_zampa(&pack, i, angoli)) {
         /* ....  */
         return 1;
      }
   
      //printf("%f %f %f\n",pos[0],pos[1],pos[2]);
      printf("%d%d %d %d%d %d %d%d %d\n",i,0,angoli[0],i,1,-angoli[1],i,2,-angoli[2]);
   }
   return 0;
}


//faccio alzare il ragno in piedi
int alzati()
{
   int passi;
   double pos[3];
   int angoli[3];
   pos[1]=0;
   double offset_angolo = atan2(l2,l3);
   double l_ipot = sqrt(l2*l2+l3*l3);
   //piego il polso
   for (passi=0; passi<11; passi++){
      pos[0] = l1+l2+l3*cos(((double)passi/10)*(M_PI/2));
      pos[2] = l3*sin(((double)passi/10)*(M_PI/2));
      sincrono(pos,angoli);
   }
   //piego il ginocchio
   for (passi=0; passi<11; passi++){
      pos[0] = l1+l_ipot*cos(offset_angolo+((double)passi/10)*(M_PI/2-offset_angolo));
      pos[2] = l_ipot*sin(offset_angolo+((double)passi/10)*(M_PI/2-offset_angolo));
      sincrono(pos,angoli);
   }
   //raddrizzo
   for (passi=0; passi<11; passi++){
      pos[2] = l_ipot+((double)passi/10)*(l2+l3-l_ipot);
      sincrono(pos,angoli);
   }
   return 0;
}

//a partire dalla posizione eretta il ragno fa "il molleggiato"
int molleggiato()
{
   int passi, angoli[3];
   double pos[3], riferimento;
   double disc = 4;
   pos[0] = l1;
   pos[1] = 0;
   while(1){
      for (passi=0; passi<21; passi++){
         riferimento = (cos(2*M_PI*((double)passi/20))-1)/2;
         pos[2]=l2+l3+riferimento*disc;
         sincrono(pos,angoli);
      }
   }
   return 0;
}
      
         
         



int main(int argc, char **argv)
{
   alzati();
   molleggiato();
   return 0;
}
