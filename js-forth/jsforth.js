  stk = [];
  rstk = [];
  mem = [];
  dictionary = [];
  here = 1;
  lastPrim = -1;
  last = -1;
  pc = -1;
  compiling = false;
  theinput = undefined;

  definePrimitives();

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
  function push(val)   { stk.push(val); }
  function pop()       { return (0 < stk.length) ? stk.pop() : under(); }
  function rPush(val)  { rstk.push(val); }
  function rPop()      { return (0 < rstk.length) ? rstk.pop() : undefined; }

  function getTOS()    { return (0 < stk.length) ? stk[stk.length-1] : undefined; }
  function getNOS()    { return (1 < stk.length) ? stk[stk.length-2] : undefined; }
  function setTOS(val) { if (0 < stk.length) stk[stk.length-1] = val; }
  function setNOS(val) { if (1 < stk.length) stk[stk.length-2] = val; }
  
  function Comma(x) { mem[here++] = x; }
  
  function exit()  { pc = rPop(); }
  function lit()   { push(mem[pc++]); }

  function definePrimitives() {
    definePrim('+',    () => { t=pop(); setTOS(getTOS() + t); });
    definePrim('-',    () => { t=pop(); setTOS(getTOS() - t); });
    definePrim('*',    () => { t=pop(); setTOS(getTOS() * t); });
    definePrim('/',    () => { t=pop(); setTOS(getTOS() / t); });
    definePrim('<',    () => { t=pop(); setTOS(getTOS() < t); });
    definePrim('=',    () => { t=pop(); setTOS(getTOS()===t); });
    definePrim('>',    () => { t=pop(); setTOS(getTOS() > t); });
    definePrim('0=',   () => { setTOS(getTOS() === 0); });
    definePrim('and',  () => { t=pop(); setTOS(getTOS() & t); });
    definePrim('or',   () => { t=pop(); setTOS(getTOS() | t); });
    definePrim('xor',  () => { t=pop(); setTOS(getTOS() ^ t); });
    definePrim('com',  () => { t=pop(); setTOS(~getTOS()); });
    definePrim('dup',  () => { push(getTOS()); });
    definePrim('drop', () => { pop(); });
    definePrim('swap', () => { n=getNOS(); t=getTOS(); setTOS(n); setNOS(t); });
    definePrim('over', () => { n=getNOS(); push(n); });
    definePrim(',',    () => { Comma(pop()); });
    definePrim('.',    () => console.log(pop()));
    definePrim('exit', exit);
  }

  function inner(start) {
    // const curPC = pc;
    pc = start;
    while ((pc)  && (pc < mem.length)) {
      const op = mem[pc++];
      if (op === undefined) { return; }
      if (typeof op === 'function') {
        op();
      } else {
        if (mem[pc] != exit) { rPush(pc); }
        pc = op;
      }
    }
    // pc = curPC;
  }

  tib = '';
  wd = '';
  pos = 0;
  tibLen = 0;
  
  function nextWord(delim) {
    wd = '';
    const isSpace = (delim === ' ');
    const isWS = (p) => { return tib.charCodeAt(p) < 33; };

    // Skip leading whitespace if delim is space
    if (isSpace) {
      while ((pos < tibLen) && isWS(pos)) { pos++; }
    }
    // Collect word
    const start = pos;
    while (pos < tibLen) {
      if ((delim === tib[pos])) { break; }
      if (isSpace && isWS(pos)) { break; }
      pos++;
    }
    wd = tib.slice(start, pos);
    return wd.length;
  }

  function outer(source) {
    tib = source;
    tibLen = tib.length;
    pos = 0;
    while (nextWord(' ') > 0) {
      if (wd == ':') {
        if (nextWord(' ') === 0) { throw new Error('expected a name after ":"'); }
        define(wd);
        compiling = true;
        continue;
      }
      
      if (wd == ';') {
        Comma(exit);
        compiling = false;
        continue;
      }
      
      const num = Number(wd);
      if (!isNaN(num)) {
        if (compiling) { Comma(lit); Comma(num); }
        else { push(num); }
        continue;
      }
    
      const entry = dictionary.find(e => e.name === wd);
      if (!entry) {
        throw new Error(`unknown word: ${wd}`);
      }

      // Found in dictionary
      if (entry.immediate || !compiling) {
        const x = here+100;
        mem[x] = entry.xt;
        mem[x+1] = undefined;
        inner(x);
      } else {
        Comma(entry.xt); // compile reference
      }
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
