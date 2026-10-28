// jsforth.js - (c) Chris Curl, MIT license

const EXIT=1, LIT=2, JMP=3, JMPZ=4, JMPNZ=5, NJMPZ=6, NJMPNZ=7, DUP=8,
    DROP=9, SWAP=10, OVER=11, STO=12, FET=13, CSTO=14, CFET=15, RTO=16,
    RAT=17, RFROM=18, MULT=19, ADD=20, SUB=21, SLMOD=22, LT=23, EQ=24,
    GT=25, PLSTO=26, FOR=27, I=28, NEXT=29, AND=30, OR=31, XOR=32,
    ZTYPE=33, FIND=34, KEY=35, QKEY=36, EMIT=37, TIMER=38, ADDW=39,
    OUTER=40, state=99, HERE_START=150;

let mem = new Int32Array(1000000), dictionary = [];
let sBs = 0,   sE = 49;
let rBs = 50,  rE = 99;
let lBs = 100, lE = 149;
let here = HERE_START, last = -1, pc = -1, op = 0;
let tib = 256, toin = 255, wdSt = 200;

inner(start) {
  pc = start;
  while (op = mem[pc++]) {
    switch (op) {
      case EXIT:   pc = rPop(); break;
      case LIT:    push(mem[pc++]); break;
      case JMP:    pc = mem[pc]; break;
      case JMPZ:   tgt = mem[pc++]; if (pop() === 0) { pc = tgt; }; break;
      case JMPNZ:  tgt = mem[pc++]; if (pop() !== 0) { pc = tgt; }; break;
      case NJMPZ:  tgt = mem[pc++]; if (TOS() === 0) { pc = tgt; }; break;
      case NJMPNZ: tgt = mem[pc++]; if (TOS() !== 0) { pc = tgt; }; break;
      case DUP:    push(TOS()); break;
      case DROP:   pop(); break;
      case SWAP:   n=pop(); t=pop(); push(n); push(t); break;
      case OVER:   n=NOS(); push(n); break;
      case STO:    t=pop(); mem[t] = pop(); break;
      case FET:    push(mem[pop()]); break;
      case CSTO:   t=pop(); mem[t] = pop(); break;
      case CFET:   push(mem[pop()]); break;
      case RTO:    push(rPop()); break;
      case RAT:    push(mem[rSp]); break;
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
function push(val)   { if (mem[sBs] < sE) { mem[++mem[sBs]] = val; } }
function rPush(val)  { if (mem[rBs] < rE) { mem[++mem[rBs]] = val; } }
function lPush(val)  { if (mem[lBs] < lE) { mem[++mem[lBs]] = val; } }
function pop()       { return (mem[sBs] > sBs) ? mem[mem[sBs]--] : under(); }
function rPop()      { return (mem[rBs] > rBs) ? mem[mem[rBs]--] : under(); }
function lPop()      { return (mem[lBs] > lBs) ? mem[mem[lBs]--] : under(); }
function Comma(x)    { mem[here++] = x; }
function emit(x)     { console.log(String.fromCharCode(x)) }
function type(str)   { console.log(str?.toString() ?? "-undef-"); }
function doType(x)   { type(x); }
function dot(x)      { type(x); if (typeof x === 'number') { type(' '); } }
function definePrim(name, fn) { define(name).xt = fn; }
function defineImm(name, fn)  { define(name, true).xt = fn; }

function stringToTib(str) {
  let i = 0;
  while (str[i]) { mem[tib+i] = str.charCodeAt(i++); }
}

function strLen(str) {
  let len = 0;
  while (mem[str]) { ++len; ++str; }
  return len;
}

function strCmp(src, dst) {
  while (mem[src] && mem[dst]) { if (mem[src++] !== mem[dst++]) { return false; } }
  return (mem[src] === mem[dst]);
}

function nextWord() {
  let len = 0;
  while (mem[toIn] && mem[toIn] < 33) { toIn++; }
  while (mem[toIn] > 32) { mem[wdSt+len++] = mem[toIn++]; }
  mem[wdSt+len] = 0;
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
    if (wd === ':') { Comma(exit); mem[state = 1]; continue; }
    if (wd === ';') { Comma(exit); mem[state = 0]; continue; }
    if (doNum(wd)) { continue; }
    const entry = find(wd);
    if (!entry) { throw new Error(`unknown word: ${wd}`); }
    if (entry.immediate || mem[state] === 0) { exec(entry.xt); continue; }
    Comma(entry.xt);
  }
  tib = cTib, tibLen = cTibLen, toIn = cToIn;
}
