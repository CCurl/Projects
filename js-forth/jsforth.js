  stk = [], rstk = [], mem = [];
  dictionary = [];
  here = 1, last = -1, pc = -1;
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
