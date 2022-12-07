const fs = require("fs");

function tokenize(s) {
  let tokens = [];
  for (let i = 0; i < s.length; i++) {
    if (" \n\r".includes(s[i])) {
    } else if ("[](){}=:@<>+-*/".includes(s[i])) {
      tokens.push({ t: s[i] });
    } else if (s[i] === '"') {
      const start = ++i;
      while (i < s.length && s[i] != '"') {
        i++;
      }

      tokens.push({ t: "String", value: s.substr(start, i - start) });
    } else {
      const start = i++;
      while (
        i < s.length &&
        ((s[i] >= "a" && s[i] <= "z") ||
          (s[i] >= "A" && s[i] <= "Z") ||
          (s[i] >= "0" && s[i] <= "9") ||
          s[i] == ".")
      ) {
        i++;
      }

      const value = s.substr(start, i - start);
      if (isNaN(value)) {
        if (value === "true" || value === "false") {
          tokens.push({ t: "Boolean", value: value === "true" });
        } else {
          tokens.push({ t: "word", value });
        }
      } else {
        tokens.push({ t: "Number", value: parseInt(value, 10) });
      }
      i--;
    }
  }

  let pos = 0;
  return {
    next() {
      return tokens[pos++];
    },
    peek() {
      return tokens[pos];
    },
    hasMore() {
      return pos < tokens.length;
    },
  };
}

function parse(tokens) {
  return {
    block: consumeStatements(tokens, null),
  };
}

function consume(tokens, t) {
  const token = tokens.next();
  if (token.t !== t) {
    throw Error(`expected ${t}, but found ${token.t}`);
  }
  return token;
}

function consumeBlock(tokens, parent) {
  consume(tokens, "{");
  const b = consumeStatements(tokens, parent);
  consume(tokens, "}");

  return b;
}

function consumeStatements(tokens, parent) {
  const block = {
    vars: {},
    stmts: [],
    parent,
  };

  while (tokens.hasMore() && tokens.peek().t != "}") {
    block.stmts.push(consumeStatement(tokens, block));
  }

  return block;
}

function consumeStatement(tokens, block) {
  if (tokens.peek().t === "word") {
    switch (tokens.peek().value) {
      case "break":
        return consumeBreak(tokens);

      case "return":
        return consumeReturn(tokens, block);

      case "while":
        return consumeWhile(tokens, block);

      default:
        return consumeAssignment(tokens, block);
    }
  }

  const expr = consumeExpression(block, tokens);
  if (tokens.hasMore() && tokens.peek().t === "=") {
    consume(tokens, "=");
    return {
      t: "assign",
      name: expr.f.replace(/:/g, () => ":" + expr.a.shift().value),
      value: consumeExpression(block, tokens),
    };
  }

  return expr;
}

function consumeAssignment(tokens, block) {
  const name = consume(tokens, "word").value;
  consume(tokens, "=");
  const value = consumeExpression(block, tokens);
  return { t: "assign", name, value };
}

function consumeReturn(tokens, block) {
  consume(tokens, "word"); // return
  return { t: "return", value: consumeExpression(block, tokens) };
}

function consumeBreak(tokens) {
  consume(tokens, "word"); // break
  return { t: "break" };
}

function consumeWhile(tokens, block) {
  consume(tokens, "word"); // while
  return {
    t: "while",
    condition: consumeExpression(block, tokens),
    block: consumeBlock(tokens, block),
  };
}

function consumeExpression(block, tokens) {
  switch (tokens.peek().t) {
    case "word":
      return { t: "var", value: tokens.next().value };

    case "Boolean":
    case "String":
    case "Number":
      return { t: tokens.peek().t, value: tokens.next().value };

    case "[":
      return consumeCall(block, tokens, true);

    case "(":
      return consumeBinaryExpression(block, tokens);

    case "{":
      return consumeBlock(tokens, block);

    case "@":
      return consumeRef(tokens);
  }
}

function consumeCall(block, tokens, isCall) {
  consume(tokens, "[");
  let a = [];
  let func = consume(tokens, "word").value;

  if (tokens.peek().t === ":") {
    func += ":";
    consume(tokens, ":");
    if (isCall) a.push(consumeExpression(block, tokens));

    while (tokens.peek().t !== "]") {
      func += " " + consume(tokens, "word").value + ":";
      consume(tokens, ":");
      if (isCall) a.push(consumeExpression(block, tokens));
    }
  }

  consume(tokens, "]");
  return { t: "call", f: `[${func}]`, a };
}

function consumeBinaryExpression(block, tokens) {
  consume(tokens, "(");
  const left = consumeExpression(block, tokens);
  const op = tokens.next().t;
  const right = consumeExpression(block, tokens);
  consume(tokens, ")");

  return { t: op, left, right };
}

function consumeRef(tokens) {
  consume(tokens, "@");
  return { t: "var", value: consumeCall(null, tokens, false).f };
}

function runProgram(program) {
  runBlock(program.block);
}

function runBlock(block) {
  for (const stmt of block.stmts) {
    runStatement(block, stmt);
    if (stmt.t === "return") {
      return block.vars._return;
    }
  }
}

function runStatement(block, stmt) {
  switch (stmt.t) {
    case "assign":
      return setVariable(block, stmt.name, runExpression(block, stmt.value));
    case "break":
      block.break = true;
      return;
    case "call":
      return runCall(block, stmt);
    case "return":
      return (block.vars._return = runExpression(block, stmt.value));
    case "while":
      return runWhile(block, stmt);
  }
}

function runWhile(block, stmt) {
  stmt.block.break = false;

  while (!stmt.block.break) {
    const condition = runExpression(block, stmt.condition);
    if (!condition.value) {
      break;
    }

    runBlock(stmt.block, stmt.block);
  }
}

function runCall(block, stmt) {
  let args = stmt.a.map((e) => runExpression(block, e));
  let types = args.map((a) => a.t);
  const realType = stmt.f.replace(/:/g, () => ":" + types.shift());

  try {
    const b = findVariable(block, realType);
    stmt.f.match(/[^\s:\[\]]+/g).forEach((v) => (b.vars[v] = args.shift()));
    return runBlock(b);
  } catch (e) {}

  if (functions[realType]) {
    return functions[realType](args);
  }

  throw new Error(`no such function: ${realType}`);
}

function findVariable(block, name) {
  if (name === "String") {
    return { t: "Type", value: "String" };
  }

  while (block) {
    if (block.vars[name]) {
      return block.vars[name];
    }

    block = block.parent;
  }

  throw new Error(`no such variable: ${name}`);
}

function setVariable(originalBlock, name, value) {
  let block = originalBlock;
  while (block) {
    if (block.vars[name]) {
      return (block.vars[name] = value);
    }

    block = block.parent;
  }

  originalBlock.vars[name] = value;
}

const binaryOperators = {
  "+": (left, right) => ({ t: "Number", value: left + right }),
  "-": (left, right) => ({ t: "Number", value: left - right }),
  "*": (left, right) => ({ t: "Number", value: left * right }),
  "/": (left, right) => ({ t: "Number", value: left / right }),
  "<": (left, right) => ({ t: "Boolean", value: left < right }),
  ">": (left, right) => ({ t: "Boolean", value: left > right }),
};

function runExpression(block, expr) {
  if (expr.t === "var") {
    return findVariable(block, expr.value);
  }

  if (expr.t === "call") {
    return runCall(block, expr);
  }

  if (binaryOperators[expr.t]) {
    const left = runExpression(block, expr.left).value;
    const right = runExpression(block, expr.right).value;
    return binaryOperators[expr.t](left, right);
  }

  // literal
  return expr;
}

const functions = {
  "[printLine:String]": ([a]) => process.stdout.write(a.value + "\n"),
  "[printLine:Number]": ([a]) => process.stdout.write(a.value + "\n"),
  "[printLine:Array]": ([a]) => process.stdout.write(JSON.stringify(a.value) + "\n"),
  "[print:String]": ([a]) => process.stdout.write(a.value),
  "[print:Number]": ([a]) => process.stdout.write(a.value),
  "[arrayOf:Type size:Number]": ([of, size]) => ({ t: "Array", value: new Array(size.value) }),
  "[on:Array set:String at:Number]": ([on, set, at]) => on.value[at.value] = set.value,
  "[len:Array]": ([a]) => ({ t: "Number", value: a.value.length }),
  "[append:String onto:Array]": ([v, a]) => ({
    t: "Array",
    value: [...a.value, v.value],
  }),
  "[on:Array at:Number]": ([on, at]) => ({
    t: "String",
    value: on.value[at.value],
  }),
  "[readFile:String]": ([path]) => ({
    t: "String",
    value: fs.readFileSync(path.value, { encoding: "utf8", flag: "r" }),
  }),
  "[openFile:String]": ([path]) => ({
    t: "File",
    value: fs.openSync(path.value, "r"),
  }),
  "[readBytes:Number from:File]": ([bytes, f]) => {
    const buf = Buffer.alloc(bytes.value);
    fs.readSync(f.value, buf);
    return { t: "String", value: buf.toString() };
  },
};

for (const path of process.argv.slice(2)) {
  fs.readFile(path, "utf8", (err, data) => {
    if (err) {
      throw err;
    }

    const tokens = tokenize(data);
    const program = parse(tokens);
    runProgram(program);
  });
}
