  mem = [], dictionary = [];
  sB = 50, rB = 100;  // Stack and Return stack base addresses
  sSp = sB, rSp = rB; // Stack and Return stack pointers
  here = 150, last = -1, pc = -1;
  tib = '', wd = '';
  pos = 0, tibLen = 0;
  compiling = false;

  // define(name, immediate) adds an entry to the dictionary
  function define(name, immediate = false) {
    dictionary[++last] = { name, xt: here, immediate };
  }

  // definePrim(name, fn) adds a primitive to the dictionary
  function definePrim(name, fn) {
    define(name);
    dictionary[last].xt = fn;
  }

  function under()     { throw new Error('Stack underflow'); }
  function push(val)   { mem[++sSp] = val; }
  function rPush(val)  { mem[++rSp] = val; }
  function pop()       { return (sSp > sB) ? mem[sSp--] : 0; }
  function rPop()      { return (rSp > rB) ? mem[rSp--] : undefined; }
  function TOS()       { return mem[sSp]; }
  function NOS()       { return mem[sSp-1]; }
  function setTOS(val) { mem[sSp] = val; }
  function setNOS(val) { mem[sSp-1] = val; }
  function Comma(x)    { mem[here++] = x; }
  function exit()      { pc = rPop(); }
  function lit()       { push(mem[pc++]); }

  function definePrimitives() {
    definePrim('+',    () => { t=pop(); setTOS(TOS() + t); });
    definePrim('-',    () => { t=pop(); setTOS(TOS() - t); });
    definePrim('*',    () => { t=pop(); setTOS(TOS() * t); });
    definePrim('/',    () => { t=pop(); setTOS(TOS() / t); });
    definePrim('<',    () => { t=pop(); setTOS(TOS() < t); });
    definePrim('=',    () => { t=pop(); setTOS(TOS()===t); });
    definePrim('>',    () => { t=pop(); setTOS(TOS() > t); });
    definePrim('0=',   () => { setTOS(TOS() === 0); });
    definePrim('and',  () => { t=pop(); setTOS(TOS() & t); });
    definePrim('or',   () => { t=pop(); setTOS(TOS() | t); });
    definePrim('xor',  () => { t=pop(); setTOS(TOS() ^ t); });
    definePrim('com',  () => { t=pop(); setTOS(~TOS()); });
    definePrim('dup',  () => { push(TOS()); });
    definePrim('drop', () => { pop(); });
    definePrim('swap', () => { n=NOS(); t=TOS(); setTOS(n); setNOS(t); });
    definePrim('over', () => { n=NOS(); push(n); });
    definePrim('@',    () => { setTOS(mem[TOS()]); });
    definePrim('!',    () => { t=pop(); n=pop(); mem[t] = n; });
    definePrim(',',    () => { Comma(pop()); });
    definePrim('.',    () => console.log(pop()));
    definePrim('exit', exit);
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
      if ((delim === tib[pos])) { break; }
      if (isSpace && isWS(pos)) { break; }
      pos++;
    }
    wd = tib.slice(start, pos);
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

function runForth(src) {
  const input = src ?? document.getElementById('forth-input').value;
  const output = document.getElementById('forth-output');
  const lines = [];
  const origLog = console.log;
  console.log = (...args) => lines.push(args.join(' '));
  try {
    outer(input);
    output.textContent = lines.join('\n');
} catch (e) {
    lines.push(`Error: ${e.message}`);
    output.textContent = lines.join('\n');
  } finally {
    console.log = origLog;
  }
}

definePrimitives();

// For handling embedded Forth scripts in the HTML document
window.addEventListener('load', async ()=>{              ///< load event handler
    let slst = document.getElementsByTagName('script')   ///< get HTMLcollection
    for (let i=0; i<slst.length; i++) {
        let s = slst[i]
        if (s.type != 'application/forth') continue;     /// * handle embedded Forth 
        if (s.src) {                                     /// * handle nested scripts
            await fetch(s.src)                           /// * fetch remote Forth script
            .then(r=>r.text())                           /// * get Forth commands
            .then(cmd=>runForth(cmd))                    /// * send it to Forth VM
        }
        else runForth(s.innerText)
    }
});
