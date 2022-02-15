#include <stdio.h>
#include <string.h>

struct ITHeader {
  unsigned int IMPM;
  char songName[26];
  short highlight;
  short orders, instruments, samples, patterns, version, compat, flags, special;
  unsigned char gv, mv, is, it, sep, pwd;
  short msglen;
  unsigned int msgptr, reserved;
  char chPan[64];
  char chVol[64];
} h;

struct Pattern {
  unsigned short len, rows;
  unsigned int x;
};

FILE* f;
FILE* of;
FILE* offLl;
FILE* offRl;
FILE* offLh;
FILE* offRh;

unsigned char* orders;

unsigned int* insptr;
unsigned int* smpptr;
unsigned int* patptr;

unsigned short* patlocaL;
unsigned short* patlocaR;

unsigned char* patL[256];
int patLenL[256];
unsigned char* patR[256];
int patLenR[256];

int patMap[256][2];
int totalL, totalR;

const unsigned char insMap[11]={
  0, 1, 2, 3, 4, 6, 7, 8, 15, 14, 12
};

int main(int argc, char** argv) {
  totalL=0;
  totalR=0;
  if (argc<2) {
    printf("usage: %s file\noutputs to m26data.bin and m26off.bin\n",argv[0]);
    return 1;
  }

  f=fopen(argv[1],"rb");
  of=fopen("m26data.bin","wb");
  offLl=fopen("m26offLl.bin","wb");
  offRl=fopen("m26offRl.bin","wb");
  offLh=fopen("m26offLh.bin","wb");
  offRh=fopen("m26offRh.bin","wb");
  if (f==NULL) {
    perror("open");
    return 1;
  }

  if (fread(&h,1,0xc0,f)<0xc0) {
    printf("incomplete file.\n");
    return 1;
  }

  if (h.IMPM!=0x4d504d49) {
    printf("not Impulse Tracker/OpenMPT module!\n");
    return 1;
  }

  orders=new unsigned char[h.orders];
  fread(orders,1,h.orders,f);

  insptr=new unsigned int[h.instruments];
  smpptr=new unsigned int[h.samples];
  patptr=new unsigned int[h.patterns];
  
  patlocaL=new unsigned short[h.patterns];
  patlocaR=new unsigned short[h.patterns];

  fread(insptr,4,h.instruments,f);
  fread(smpptr,4,h.samples,f);
  fread(patptr,4,h.patterns,f);

  // skip the rest. we are only interested in the patterns.
  char* pat;
  unsigned char* tpatL;
  unsigned char* tpatR;
  Pattern path;
  unsigned char chanVar, maskVar[2], note[2], ins[2], vol[2];
  unsigned short curRow;
  for (int i=0; i<h.patterns; i++) {
    fseek(f,patptr[i],SEEK_SET);
    fread(&path,1,8,f);
    chanVar=0; maskVar[0]=0; note[0]=0; ins[0]=0; vol[0]=0; curRow=0;
               maskVar[1]=0; note[1]=0; ins[1]=0; vol[1]=0;
    pat=new char[path.rows*6]; // period, color, volume x2
    memset(pat,0,path.rows*6);
    if (patptr[i]) while (curRow<path.rows) {
      chanVar=fgetc(f);
      if (chanVar==0) {
        curRow++;
        continue;
      }
      if ((chanVar&127)>2) {
        printf("error: there shall be 2 channels! in pattern %d row %d chanVar %d\n",i,curRow,chanVar);
        return 1;
      }
      if (chanVar&128) {
        chanVar&=127;
        maskVar[chanVar-1]=fgetc(f);
      }
      if (maskVar[chanVar-1]&1) {
        note[chanVar-1]=fgetc(f);
        pat[(curRow*6)+((chanVar-1)*3)]=note[chanVar-1];
      }
      if (maskVar[chanVar-1]&2) {
        ins[chanVar-1]=fgetc(f);
        pat[(curRow*6)+((chanVar-1)*3)+1]=ins[chanVar-1];
      }
      if (maskVar[chanVar-1]&4) {
        vol[chanVar-1]=fgetc(f)+1;
        pat[(curRow*6)+((chanVar-1)*3)+2]=vol[chanVar-1];
      }
      if (maskVar[chanVar-1]&8) {
        fgetc(f); fgetc(f); // ignore
      }
      if (maskVar[chanVar-1]&16) {
        pat[(curRow*6)+((chanVar-1)*3)]=note[chanVar-1];
      }
      if (maskVar[chanVar-1]&32) {
        pat[(curRow*6)+((chanVar-1)*3)+1]=ins[chanVar-1];
      }
      if (maskVar[chanVar-1]&64) {
        pat[(curRow*6)+((chanVar-1)*3)+2]=vol[chanVar-1];
      }
    } else {
      path.rows=64;
    }
    //printf("PATTERN %d:\n",i);
    unsigned char oldAF[2];
    unsigned char oldAC[2];
    unsigned char oldAV[2];
    oldAF[0]=0; oldAC[0]=0; oldAV[0]=0;
    oldAF[1]=0; oldAC[1]=0; oldAV[1]=0;
    tpatL=new unsigned char[2*path.rows];
    memset(tpatL,0,2*path.rows);
    tpatR=new unsigned char[2*path.rows];
    memset(tpatR,0,2*path.rows);
    for (int j=0; j<path.rows; j++) {
      //printf("%.2x %.2x %.2x | %.2x %.2x %.2x\n",pat[j*6],pat[1+j*6],pat[2+j*6],pat[3+j*6],pat[4+j*6],pat[5+j*6]);
      if (pat[j*6]>0) {
        oldAF[0]=(pat[j*6]-0x3c)&0x1f;
      }
      if (pat[1+j*6]>0 && pat[1+j*6]<11) {
        oldAC[0]=insMap[pat[1+j*6]];
        oldAV[0]=15;
      }
      if (pat[j*6]==255 || pat[j*6]==254) {
        oldAC[0]=0;
      }
      if (pat[2+j*6]>0) {
        if (pat[2+j*6]>16) {
          oldAV[0]=0x0f;
        } else {
          oldAV[0]=(pat[2+j*6]-1)&0x0f;
        }
      }

      if (pat[3+j*6]>0) {
        oldAF[1]=(pat[3+j*6]-0x3c)&0x1f;
      }
      if (pat[4+j*6]>0 && pat[4+j*6]<11) {
        oldAC[1]=insMap[pat[4+j*6]];
        oldAV[1]=15;
      }
      if (pat[3+j*6]==255 || pat[3+j*6]==254) {
        oldAC[1]=0;
      }
      if (pat[5+j*6]>0) {
        if (pat[5+j*6]>16) {
          oldAV[1]=0x0f;
        } else {
          oldAV[1]=(pat[5+j*6]-1)&0x0f;
        }
      }

      tpatL[j*2]=oldAF[0];
      tpatL[1+j*2]=(oldAC[0]<<4)|oldAV[0];
      tpatR[j*2]=oldAF[1];
      tpatR[1+j*2]=(oldAC[1]<<4)|oldAV[1];
    }

    bool found;

    found=false;
    for (int j=0; j<totalL; j++) {
      if (patLenL[j]!=2*path.rows) {
        printf("differing length.\n");
        continue;
      }
      if (memcmp(tpatL,patL[j],2*path.rows)==0) {
        patMap[i][0]=j;
        found=true;
        delete[] tpatL;
      }
    }
    if (!found) {
      patMap[i][0]=totalL;
      patL[totalL]=tpatL;
      patLenL[totalL]=2*path.rows;
      totalL++;
    }

    found=false;
    for (int j=0; j<totalR; j++) {
      if (patLenR[j]!=2*path.rows) {
        printf("differing length.\n");
        continue;
      }
      if (memcmp(tpatR,patR[j],2*path.rows)==0) {
        patMap[i][1]=j;
        found=true;
        delete[] tpatR;
      }
    }
    if (!found) {
      patMap[i][1]=totalR;
      patR[totalR]=tpatR;
      patLenR[totalR]=2*path.rows;
      totalR++;
    }

    delete[] pat;
  }
  // write data
  for (int i=0; i<totalL; i++) {
    patlocaL[i]=ftell(of);

    unsigned char oldF, oldVC;
    int tim;
    oldF=patL[i][0];
    oldVC=patL[i][1];
    tim=-1;
    fputc(oldF|0xc0,of);
    fputc(oldVC,of);
    for (int j=0; j<patLenL[i]; j+=2) {
      if (oldF==patL[i][j] && oldVC==patL[i][j+1]) {
        if (++tim>=30) {
          fputc(tim,of);
          tim=0;
        }
      } else {
        if (tim>0) {
          fputc(tim,of);
        }
        tim=0;
        if (oldVC!=patL[i][j+1]) {
          if ((oldVC&0xf0)==(patL[i][j+1]&0xf0)) {
            oldVC=patL[i][j+1];
            if (oldF!=patL[i][j]) {
              oldF=patL[i][j];
              fputc(oldF|0xc0,of);
              fputc(oldVC,of);
            } else {
              fputc((oldVC&15)|0x20,of);
            }
          } else {
            oldVC=patL[i][j+1];
            if (oldF!=patL[i][j]) {
              oldF=patL[i][j];
              fputc(oldF|0xc0,of);
              fputc(oldVC,of);
            } else {
              fputc(0x81,of);
              fputc(oldVC,of);
            }
          }
        } else {
          oldF=patL[i][j];
          fputc(oldF|0x40,of);
        }
      }
    }
    if (tim>0) {
      fputc(tim,of);
    }
    fputc(0,of);
  }
  for (int i=0; i<totalR; i++) {
    patlocaR[i]=ftell(of);

    unsigned char oldF, oldVC;
    int tim;
    oldF=patR[i][0];
    oldVC=patR[i][1];
    tim=-1;
    fputc(oldF|0xc0,of);
    fputc(oldVC,of);
    for (int j=0; j<patLenR[i]; j+=2) {
      if (oldF==patR[i][j] && oldVC==patR[i][j+1]) {
        if (++tim>=30) {
          fputc(tim,of);
          tim=0;
        }
      } else {
        if (tim>0) {
          fputc(tim,of);
        }
        tim=0;
        if (oldVC!=patR[i][j+1]) {
          if ((oldVC&0xf0)==(patR[i][j+1]&0xf0)) {
            oldVC=patR[i][j+1];
            if (oldF!=patR[i][j]) {
              oldF=patR[i][j];
              fputc(oldF|0xc0,of);
              fputc(oldVC,of);
            } else {
              fputc((oldVC&15)|0x20,of);
            }
          } else {
            oldVC=patR[i][j+1];
            if (oldF!=patR[i][j]) {
              oldF=patR[i][j];
              fputc(oldF|0xc0,of);
              fputc(oldVC,of);
            } else {
              fputc(0x81,of);
              fputc(oldVC,of);
            }
          }
        } else {
          oldF=patR[i][j];
          fputc(oldF|0x40,of);
        }
      }
    }
    if (tim>0) {
      fputc(tim,of);
    }
    fputc(0,of);
  }
  // dump orders
  for (int i=0; i<h.orders; i++) {
    printf("%.2x | %.2x\n",patMap[orders[i]][0],patMap[orders[i]][1]);
  }

  // write orders
  unsigned short addr;
  for (int i=0; i<h.orders; i++) {
    addr=patlocaL[patMap[orders[i]][0]]+0xf000;
    fputc(addr&0xff,offLl);
    fputc(addr>>8,offLh);
  }
  fputc(0,offLl);
  fputc(0,offLh);

  for (int i=0; i<h.orders; i++) {
    addr=patlocaR[patMap[orders[i]][1]]+0xf000;
    fputc(addr&0xff,offRl);
    fputc(addr>>8,offRh);
  }
  fputc(0,offRl);
  fputc(0,offRh);

  fclose(f);
  fclose(of);
  fclose(offLl);
  fclose(offRl);
  fclose(offLh);
  fclose(offRh);

  return 0;
}
