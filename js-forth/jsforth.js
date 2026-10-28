// jsforth.js - (c) Chris Curl, MIT license

const EXIT=1, LIT=2, JMP=3, JMPZ=4, JMPNZ=5, NJMPZ=6, NJMPNZ=7, DUP=8,
    DROP=9, SWAP=10, OVER=11, STO=12, FET=13, CSTO=14, CFET=15, RTO=16,
    RAT=17, RFROM=18, MULT=19, ADD=20, SUB=21, SLMOD=22, LT=23, EQ=24,
    GT=25, PLSTO=26, FOR=27, I=28, NEXT=29, AND=30, OR=31, XOR=32,
    ZTYPE=33, FIND=34, KEY=35, QKEY=36, EMIT=37, TIMER=38, ADDW=39,
    OUTER=40, state=99, HERE_START=150;

const M = new Int32Array(250000), DV = new DataView(M.buffer), MB = new Uint8Array(M.buffer);
const dictionary = [];
const sBs = 0,   sE = 49;
const rBs = 50,  rE = 99;
const lBs = 100, lE = 149;
let here = HERE_START, last = -1, pc = -1, op = 0;
let tib = 256, toin = 255, wdSt = 200;
const stk = [], sp = 0;

function inner(start) {
  pc = start;
  while (op = M[pc++]) {
    switch (op) {
      case EXIT:   pc = rPop(); break;
      case LIT:    push(DV.getFloat64(pc)); pc += 8; break;
      case JMP:    pc = M[pc]; break;
      case JMPZ:   tgt = M[pc++]; if (pop() === 0) { pc = tgt; }; break;
      case JMPNZ:  tgt = M[pc++]; if (pop() !== 0) { pc = tgt; }; break;
      case NJMPZ:  tgt = M[pc++]; if (TOS() === 0) { pc = tgt; }; break;
      case NJMPNZ: tgt = M[pc++]; if (TOS() !== 0) { pc = tgt; }; break;
      case DUP:    push(TOS()); break;
      case DROP:   pop(); break;
      case SWAP:   n=pop(); t=pop(); push(n); push(t); break;
      case OVER:   n=NOS(); push(n); break;
      case STO:    t=pop(); M[t] = pop(); break;
      case FET:    push(M[pop()]); break;
      case CSTO:   t=pop(); M[t] = pop(); break;
      case CFET:   push(M[pop()]); break;
      case RTO:    push(rPop()); break;
      case RAT:    push(M[rSp]); break;
      case RFROM:  push(rPop()); break;
      case MULT:   t=pop(); n=pop(); push(n * t); break;
      case ADD:    t=pop(); n=pop(); push(n + t); break;
      case SUB:    t=pop(); n=pop(); push(n - t); break;
      case SLMOD:  t=pop(), n=pop(); push(t%n); push(n/t); break;
      case LT:     t=pop(); n=pop(); push(n < t   ? -1 : 0); break;
      case EQ:     t=pop(); n=pop(); push(n === t ? -1 : 0); break;
      case GT:     t=pop(); n=pop(); push(n > t   ? -1 : 0); break;
      case AND:    t=pop(); n=pop(); push(n & t); break;
      case OR:     t=pop(); n=pop(); push(n | t); break;
      case XOR:    t=pop(); n=pop(); push(n ^ t); break;
      case ZTYPE:  doType(pop()); break;
      case FIND:   push(find()); break;
      case KEY:    push(key()); break;
      case QKEY:   push(qkey()); break;
      case EMIT:   emit(pop()); break;
      case TIMER:  push(timer()); break;
      case ADDW:   define(undefined); break;
      case OUTER:  outer(pop()); break;
      default: throw new Error(`Unknown opcode: ${op}`);
    }
  }
}

// define(name, immediate) adds an entry to the dictionary
function define(name, immediate = false) {
  dictionary[++last] = { name, xt: here, immediate };
  return dictionary[last];
}

function find(name) {
  for (let i = last; i > 0; i--) {
    if (strCmp(dictionary[i].name, name)) { return dictionary[i]; }
  }
  return null;
}

function under()     { throw new Error('Stack underflow'); }
function push(val)   { stk[sp++] = val; }
function pop()       { return (sp > 0) ? stk[--sp] : under(); }
function rPush(val)  { if (rSp < rE) { stk[rSp++] = val; } }
function rPop()      { return (rSp > rBs) ? stk[--rSp] : under(); }
function lPush(val)  { if (lSp < lE) { stk[lSp++] = val; } }
function lPop()      { return (lSp > lBs) ? stk[--lSp] : under(); }
function Comma(x)    { M[here++] = x; }
function emit(x)     { console.log(String.fromCharCode(x)) }
function type(str)   { console.log(str?.toString() ?? "-undef-"); }
function doType(x)   { type(x); }
function dot(x)      { type(x); if (typeof x === 'number') { type(' '); } }
function definePrim(name, fn) { define(name).xt = fn; }
function defineImm(name, fn)  { define(name, true).xt = fn; }

function stringToTib(str) {
  let i = 0;
  while (str[i]) { M[tib+i] = str.charCodeAt(i++); }
}

function strLen(str) {
  let len = 0;
  while (M[str]) { ++len; ++str; }
  return len;
}

function strCmp(src, dst) {
  while (M[src] && M[dst]) { if (M[src++] !== M[dst++]) { return false; } }
  return (M[src] === M[dst]);
}

function nextWord() {
  let len = 0;
  while (M[toIn] && M[toIn] < 33) { toIn++; }
  while (M[toIn] > 32) { M[wdSt+len++] = M[toIn++]; }
  M[wdSt+len] = 0;
  return len;
}

function doNum(token) {
  const num = Number(token);
  if (isNaN(num)) { return false; }
  if (compiling) { Comma(lit); Comma(num); }
  else { push(num); }
  return true;
}

function outer(src) {
  const cTib = tib, cTibLen = tibLen, cToIn = toIn;
  tib = src, tibLen = src.length, toIn = 0;
  while (nextWord() > 0) {
    if (wd === ':') { Comma(exit); M[state = 1]; continue; }
    if (wd === ';') { Comma(exit); M[state = 0]; continue; }
    if (doNum(wd)) { continue; }
    const entry = find(wd);
    if (!entry) { throw new Error(`unknown word: ${wd}`); }
    if (entry.immediate || M[state] === 0) { exec(entry.xt); continue; }
    Comma(entry.xt);
  }
  tib = cTib, tibLen = cTibLen, toIn = cToIn;
}
