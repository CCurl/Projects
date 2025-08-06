/***************************************************************
   crc's _ _
        (_) | ___
        | | |/ _ \  a tiny virtual computer
        | | | (_) | 64kw RAM, 32-bit, Dual Stack, MISC
        |_|_|\___/  ilo.c (c) charles childers
 **************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define T ds[sp]    /* Top of Data Stack    */
#define N ds[sp-1]  /* Next on Data Stack   */
#define R as[rp]    /* Top of Address Stack */
#define V void
#define I int
#define C char

I ip,        /* instruction pointer             */
  sp,        /* data stack pointer              */
  rp,        /* address stack pointer           */
  ds[33],    /* data stack                      */
  as[257],   /* address stack                   */
  m[65536];  /* memory                          */

C *blocks,   /* name of block file (ilo.blocks) */
  *rom;      /* name of image (ilo.rom)         */

/* the other variables are used by the various
   functions for misc. purposes                   */

I a, b, f, s, d, l;
C i[1];

V push(I v) { ds[sp + 1] = v; sp += 1; }
I pop() { sp -= 1; return ds[sp + 1]; }

V load_image() {
  f = open(rom, O_RDONLY, 0666);
  if (!f) { return; };
  read(f, &m, 65536 * 4);
  close(f);
  ip = sp = rp = 0;
}

V save_image() {
  f = open(rom, O_WRONLY, 0666);
  write(f, &m, 65536 * 4);
  close(f);
}

V block_common() {
  b = pop(); /* block buffer */
  a = pop(); /* block number */
  lseek(f, 4096 * a, SEEK_SET);
}

V read_block() {
  f = open(blocks, O_RDONLY, 0666);
  block_common();
  read(f, m + b, 4096);
  close(f);
}

V write_block() {
  f = open(blocks, O_WRONLY, 0666);
  block_common();
  write(f, m + b, 4096);
  close(f);
}

V save_ip() { rp += 1; R = ip; }
V symmetric() { if (b >= 0 && N < 0) { T += 1; N -= b; } }

V li() { ip += 1; push(m[ip]); }
V du() { push(T); }
V dr() { ds[sp] = 0; sp -= 1; }
V sw() { a = T; T = N; N = a; }
V pu() { rp += 1; R = pop(); }
V po() { push(R); rp -= 1; }
V ju() { ip = pop() - 1; }
V ca() { save_ip(); ip = pop() - 1; }
V cc() { a = pop(); if (pop()) { save_ip(); ip = a - 1; } }
V cj() { a = pop(); if (pop()) { ip = a - 1; } }
V re() { ip = R; rp -= 1; }
V eq() { N = (N == T) ? -1 : 0; sp -= 1; }
V ne() { N = (N != T) ? -1 : 0; sp -= 1; }
V lt() { N = (N <  T) ? -1 : 0; sp -= 1; }
V gt() { N = (N >  T) ? -1 : 0; sp -= 1; }
V fe() { T = m[T]; }
V st() { m[T] = N; sp -= 2; }
V ad() { N += T; sp -= 1; }
V su() { N -= T; sp -= 1; }
V mu() { N *= T; sp -= 1; }
V di() { a = T; b = N; T = b / a; N = b % a; symmetric(); }
V an() { N = T & N; sp -= 1; }
V or() { N = T | N; sp -= 1; }
V xo() { N = T ^ N; sp -= 1; }
V sl() { N = N << T; sp -= 1; }
V sr() { N = N >> T; sp -= 1; }
V cp() { l = pop(); d = pop(); s = T; T = -1;
         while (l) { if (m[d] != m[s]) { T = 0; }
                     l -= 1; s += 1; d += 1; } }
V cy() { l = pop(); d = pop(); s = pop();
         while (l) { m[d] = m[s]; l -= 1; s += 1; d += 1; } }
V ioa() { i[0] = (char)pop(); write(1, &i, 1); }
V iob() { read(0, &i, 1); push(i[0]); }
V ioc() { read_block(); }
V iod() { write_block(); }
V ioe() { save_image(); }
V iof() { load_image(); ip = -1; }
V iog() { ip = 65536; }
V ioh() { push(sp); push(rp); }
V io() {
  switch (pop()) {
    case 0: ioa(); break;  case 1: iob(); break;
    case 2: ioc(); break;  case 3: iod(); break;
    case 4: ioe(); break;  case 5: iof(); break;
    case 6: iog(); break;  case 7: ioh(); break;
    default: break;
  }
}

/* Using a switch here instead of a jump table to avoid   */
/* some issues w/relocation stuff when building w/o libc  */

V process(I o) {
  switch (o) {
    case  0:       break;   case  1: li(); break;
    case  2: du(); break;   case  3: dr(); break;
    case  4: sw(); break;   case  5: pu(); break;
    case  6: po(); break;   case  7: ju(); break;
    case  8: ca(); break;   case  9: cc(); break;
    case 10: cj(); break;   case 11: re(); break;
    case 12: eq(); break;   case 13: ne(); break;
    case 14: lt(); break;   case 15: gt(); break;
    case 16: fe(); break;   case 17: st(); break;
    case 18: ad(); break;   case 19: su(); break;
    case 20: mu(); break;   case 21: di(); break;
    case 22: an(); break;   case 23: or(); break;
    case 24: xo(); break;   case 25: sl(); break;
    case 26: sr(); break;   case 27: cp(); break;
    case 28: cy(); break;   case 29: io(); break;
    default: break;
  }
}

V process_bundle(I opcode) {
  process(opcode & 0xFF);
  process((opcode >> 8) & 0xFF);
  process((opcode >> 16) & 0xFF);
  process((opcode >> 24) & 0xFF);
}

V execute() {
  while (ip < 65536) {
    process_bundle(m[ip]);
    ip += 1;
  }
}

#ifndef NOSTDLIB

I main(I argc, C **argv) {
  blocks = (argc > 1) ? argv[1] : "ilo.blocks";
  rom    = (argc > 2) ? argv[2] : "ilo.rom";
  load_image();
  execute();
  for (; sp > 0; sp -= 1) printf(" %d", ds[sp]); printf("\n");
  return 0;
}

#else

I main(I argc, C **argv) {
  blocks = "ilo.blocks";
  rom = "ilo.rom";
  load_image();
  execute();
  return 0;
}

#endif