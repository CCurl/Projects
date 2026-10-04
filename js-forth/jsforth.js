// jsforth.js - (c) Chris Curl, MIT license

  mem = [], dictionary = [];
  sBs = 0,   rBs = 50,  lBs = 100;  // Data, Return, Loop stack bases
  sSp = sBs, rSp = rBs, lSp = lBs;  // Stack pointers
  here = 150, last = -1, pc = -1;
  tib = '', wd = '';
  pos = 0, tibLen = 0;
  compiling = false;

  // define(name, immediate) adds an entry to the dictionary
  function define(name, immediate = false) {
    dictionary[++last] = { name, xt: here, immediate };
    return dictionary[last];
  }

  function under()     { throw new Error('Stack underflow'); }
  function push(val)   { mem[++sSp] = val; }
  function rPush(val)  { mem[++rSp] = val; }
  function lPush(val)  { mem[++lSp] = val; }
  function pop()       { return (sSp > sBs) ? mem[sSp--] : under(); }
  function rPop()      { return (rSp > rBs) ? mem[rSp--] : undefined; }
  function lPop()      { return (lSp > lBs) ? mem[lSp--] : undefined; }
  function TOS()       { return mem[sSp]; }
  function NOS()       { return mem[sSp-1]; }
  function setTOS(val) { mem[sSp] = val; }
  function setNOS(val) { mem[sSp-1] = val; }
  function Comma(x)    { mem[here++] = x; }
  function exit()      { pc = rPop(); }
  function lit()       { push(mem[pc++]); }
  function jmp()       { tgt = mem[pc++]; pc = tgt; }
  function jmpz()      { tgt = mem[pc++]; if (pop() === 0) { pc = tgt; } }
  function jmpnz()     { tgt = mem[pc++]; if (pop() !== 0) { pc = tgt; } }
  function njmpz()     { tgt = mem[pc++]; if (TOS() === 0) { pc = tgt; } }
  function njmpnz()    { tgt = mem[pc++]; if (TOS() !== 0) { pc = tgt; } }
  function emit(x)     { console.log(String.fromCharCode(x)) }
  function type(str)   { console.log(str?.toString() ?? "-undef-"); }
  function doType()    { type(pop()); }
  function dot(x)      { type(x); if (typeof x === 'number') { type(' '); } }
  function definePrim(name, fn) { define(name).xt = fn; }
  function defineImm(name, fn)  { define(name, true).xt = fn; }
  
  function sQuote() {
    ++pos; // skip the initial space
    nextWord('"');
    const str = new String(wd);  // this creates a copy of the current word
    if (compiling) { Comma(lit); Comma(str); }
    else { push(str); }
  }
  
  function doWords() {
    num = 0, cnt = 0;
    for (let i = last; i >= 0; i--) {
      type(dictionary[i].name);
      type(' ');
      ++cnt; ++num;
      if (7 < num) { type('\n'); num = 0; }
    }
    type(` (${cnt} words)`);
  }

  function definePrimitives() {
    definePrim('+',      () => { t=pop(); setTOS(TOS() + t); });
    definePrim('-',      () => { t=pop(); setTOS(TOS() - t); });
    definePrim('*',      () => { t=pop(); setTOS(TOS() * t); });
    definePrim('/',      () => { t=pop(); setTOS(TOS() / t); });
    definePrim('<',      () => { t=pop(); setTOS((TOS() < t) ? -1 : 0); });
    definePrim('=',      () => { t=pop(); setTOS((TOS()===t) ? -1 : 0); });
    definePrim('>',      () => { t=pop(); setTOS((TOS() > t) ? -1 : 0); });
    definePrim('0=',     () => { setTOS(TOS() === 0 ? -1 : 0); });
    definePrim('and',    () => { t=pop(); setTOS(TOS() & t); });
    definePrim('or',     () => { t=pop(); setTOS(TOS() | t); });
    definePrim('xor',    () => { t=pop(); setTOS(TOS() ^ t); });
    definePrim('com',    () => { t=pop(); setTOS(~TOS()); });
    definePrim('dup',    () => { push(TOS()); });
    definePrim('drop',   () => { pop(); });
    definePrim('swap',   () => { n=NOS(); t=TOS(); setTOS(n); setNOS(t); });
    definePrim('over',   () => { n=NOS(); push(n); });
    definePrim('@',      () => { setTOS(mem[TOS()]); });
    definePrim('!',      () => { t=pop(); n=pop(); mem[t] = n; });
    definePrim(',',      () => { Comma(pop()); });
    definePrim('.',      () => { dot(pop()); });
    definePrim('for',    () => { lPush(pc); lPush(pop()); lPush(0); });
    definePrim('i',      () => { push(mem[lSp]); });
    definePrim('next',   () => { if (++mem[lSp] < mem[lSp-1]) { pc = mem[lSp-2]; } else { lSp -= 3; } });
    definePrim('emit',   () => { emit(pop()); });
    definePrim('exit',   () => { exit(); });
    definePrim('type',   () => { doType(); });
    definePrim('words',  () => { doWords(); });
    definePrim('immediate', () => { dictionary[last].immediate = true; });
    defineImm('s"',      () => { sQuote(); });
    defineImm('."',      () => { sQuote(); if (compiling) { Comma(doType); } else { doType(); } });
    defineImm('if',      () => { Comma(jmpz); push(here); Comma(0); });
    defineImm('then',    () => { mem[pop()] = here; });
    defineImm('begin',   () => { push(here); });
    defineImm('while',   () => { Comma(jmpnz); Comma(pop()); });
    defineImm('until',   () => { Comma(jmpz);  Comma(pop()); });
    defineImm('again',   () => { Comma(jmp);   Comma(pop()); });
  }

  function inner(start) {
    pc = start;
    while ((pc)  && (pc < mem.length)) {
      const op = mem[pc++];
      if (op === undefined) { return; }
      if (typeof op === 'function') { op(); }
      else {
        if (mem[pc] != exit) { rPush(pc); }
        pc = op;
      }
    }
  }
  
  function nextWord(delim) {
    wd = '';
    const isSpace = (delim === ' ');
    const isWS = (p) => { return tib.charCodeAt(p) < 33; };

    if (isSpace) {
      while ((pos < tibLen) && isWS(pos)) { pos++; }
    }
    const start = pos;
    while (pos < tibLen) {
      if (isSpace && isWS(pos)) { break; }
      if ((delim === tib[pos])) { break; }
      pos++;
    }
    wd = tib.slice(start, pos);
    if (!isSpace) { pos++; }
    return wd.length;
  }
  
  function doNum(token) {
    const num = Number(token);
    if (isNaN(num)) { return false; }
    if (compiling) { Comma(lit); Comma(num); }
    else { push(num); }
    return true;
  }

  function doWord(token) {
    const entry = dictionary.find(e => e.name === token);
    if (!entry) { return false; }
    if (entry.immediate || !compiling) {
      const x = here+100;
      mem[x] = entry.xt;
      mem[x+1] = undefined;
      inner(x);
    } else {
      Comma(entry.xt); // compile reference
    }
    return true;
  }

  function doColon(token) {
    if (token != ':') { return false; }
    if (nextWord(' ') === 0) { throw new Error('expected a name after ":"'); }
    define(wd);
    compiling = true;
    return true;
  }

  function doSemi(token) {
    if (token != ';') { return false; }
    Comma(exit);
    compiling = false;
    return true;
  }

  function outer(source) {
    tib = source;
    tibLen = tib.length;
    pos = 0;
    while (nextWord(' ') > 0) {
      if (doColon(wd)) { continue; }
      if (doSemi(wd)) { continue; }
      if (doNum(wd)) { continue; }
      if (doWord(wd)) { continue; }
      throw new Error(`unknown word: ${wd}`);
    }
  }
  
definePrimitives();

function runForth(src) {
  const input = src ?? document.getElementById('forth-input').value;
  const output = document.getElementById('forth-output');
  const lines = [];
  const origLog = console.log;
  console.log = (...args) => lines.push(args.join(' '));
  try {
    outer(input);
    if (!src){ type(' ok\n'); }
    output.textContent = lines.join('');
  } catch (e) {
    output.textContent = lines.join('');
    output.textContent += `\nError: ${e.message}`;
  } finally {
    console.log = origLog;
  }
}

// For handling embedded Forth scripts in the HTML document
window.addEventListener('load', async ()=>{              // load event handler
    let slst = document.getElementsByTagName('script')   // get scripts
    for (let i=0; i<slst.length; i++) {
        let s = slst[i]
        if (s.type != 'application/forth') continue;     // handle embedded Forth 
        if (s.src) {                                     // handle nested scripts
            await fetch(s.src)                           // fetch remote Forth script
            .then(r=>r.text())                           // get Forth commands
            .then(cmd=>runForth(cmd))                    // send it to Forth VM
        }
        else runForth(s.innerText)
    }
});
