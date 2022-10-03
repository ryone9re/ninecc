#include "ninecc.h"

static VarList *globals;
static VarList *locals;

// 変数を変数リストから探す
Var *find_var_from(Token *tok, VarList *vars) {
  if (!vars) return (NULL);
  for (VarList *vl = vars; vl; vl = vl->next) {
    if (strlen(vl->var->name) == tok->len &&
        !strncmp(vl->var->name, tok->str, tok->len))
      return (vl->var);
  }
  return (NULL);
}

// 宣言済み変数を探す
Var *find_lvar(Token *tok) {
  Var *v;

  v = find_var_from(tok, locals);
  if (v) return (v);
  v = find_var_from(tok, globals);
  if (v) return (v);
  return (NULL);
}

// 空Nodeを作成
static Node *new_node(NodeKind kind, Token *tok) {
  Node *node = (Node *)calloc(1, sizeof(Node));
  if (!node) exit_with_error();
  node->kind = kind;
  node->tok = tok;
  return (node);
}

// 二項演算子Nodeを作成
static Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  if (!node) exit_with_error();
  node->lhs = lhs;
  node->rhs = rhs;
  return (node);
}

// 単項演算子Nodeを作成
static Node *new_unary_node(NodeKind kind, Node *expr, Token *tok) {
  Node *node = new_node(kind, tok);
  if (!node) exit_with_error();
  node->lhs = expr;
  return (node);
}

// 整数値Nodeを作成
static Node *new_number_node(int val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  if (!node) exit_with_error();
  node->val = val;
  return (node);
}

// 変数Nodeを作成
static Node *new_var_node(Var *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  if (!node) exit_with_error();
  node->var = var;
  return (node);
}

// 新規変数定義
static Var *new_var(Type *type, char *name, VarList **add_to) {
  Var *var = (Var *)calloc(1, sizeof(Var));
  if (!var) exit_with_error();

  var->type = type;
  var->name = name;

  VarList *vl = (VarList *)calloc(1, sizeof(VarList));
  if (!vl) exit_with_error();
  vl->var = var;
  vl->next = *add_to;
  *add_to = vl;
  return (var);
}

// 新規ローカル変数定義
static Var *new_local_var(Type *type, char *name) {
  Var *v = new_var(type, name, &locals);
  v->is_local = true;
  return (v);
}

// 新規データラベル作成
static char *new_label() {
  char buf[sizeof(int) * 8];
  static size_t count = 0;
  sprintf(buf, ".L.data.%zu", count++);
  return (substr(buf, sizeof(int) * 8));
}

static Type *basetype(void);
static Type *suffix(Type *type);
static void func_dec(void);
static void global_var(void);
static Function *function(void);
static VarList *params(void);
static Node *stmt(void);
static Node *declaration(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *postfix(void);
static Node *primary(void);
static Node *args(void);

// 返り値を宣言してるかどうか
static bool read_basetype(void) {
  expect_type();
  while (consume("*"))
    ;
  return (true);
}

// 関数定義かどうか
static bool is_funcdef(void) {
  Token *tok = token;
  bool isfunc = read_basetype() && consume_ident() && consume("(");
  if (!isfunc) {
    token = tok;
    return (isfunc);
  }
  while (token->kind != TK_RESERVED || strncmp(token->str, ")", token->len))
    token = token->next;
  isfunc = consume(")") && isfunc && consume("{");
  token = tok;
  return (isfunc);
}

// 関数宣言かどうか
static bool is_function() {
  Token *tok = token;
  bool isfunc = read_basetype() && consume_ident() && peek("(");
  token = tok;
  return (isfunc);
}

// 型名かどうか
static bool is_type_dec(void) { return (peek("char") || peek("int")); }

Program *program(void) {
  Program *prog;
  Function func = {};
  Function *fn = &func;

  globals = NULL;

  while (!at_eof()) {
    if (is_funcdef()) {
      fn->next = function();
      fn = fn->next;
    } else if (is_function())
      func_dec();
    else
      global_var();
  }

  prog = (Program *)calloc(1, sizeof(Program));
  if (!prog) exit_with_error();
  prog->functions = func.next;
  prog->globals = globals;
  return (prog);
}

static Type *basetype(void) {
  Token *tok = expect_type();
  Type *type = new_type_from_str(tok->str);

  while (consume("*")) type = new_type(TYPE_PTR, type);

  return (type);
}

static Type *suffix(Type *type) {
  if (!consume("[")) return (type);
  size_t len = expect_number();
  expect("]");
  type = suffix(type);
  return (new_type_array(TYPE_ARRAY, type, len));
}

static void func_dec(void) {
  basetype();
  Token *dec = consume_ident();
  if (!dec) error_at(dec->str, "返り値の方が不正です");
  expect("(");
  while (token->kind != TK_RESERVED || strncmp(token->str, ")", token->len))
    token = token->next;
  expect(")");
  expect(";");
}

static void global_var(void) {
  Type *type = basetype();
  Token *dec = consume_ident();
  if (!dec) error_at(dec->str, "返り値の型が不正です");

  if (peek("[")) type = suffix(type);
  expect(";");
  if (find_var_from(dec, globals)) error_at(dec->str, "すでに宣言されています");
  new_var(type, substr(dec->str, dec->len), &globals);
}

static Function *function(void) {
  locals = NULL;

  // 戻り値の型
  basetype();

  // 関数名定義
  char *funcname = expect_ident();
  if (!funcname) error_at(token->str, "不正な関数名です");

  // 関数生成
  Function *func = (Function *)calloc(1, sizeof(Function));
  if (!func) exit_with_error();
  func->name = funcname;

  // 仮引数宣言
  func->params = params();

  expect("{");

  Node head = {};
  Node *cur = &head;

  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }

  func->node = head.next;
  func->locals = locals;
  return (func);
}

static VarList *params(void) {
  Type *type;

  expect("(");

  if (consume(")")) return (NULL);

  type = basetype();

  VarList *head = (VarList *)calloc(1, sizeof(VarList));
  if (!head) exit_with_error();
  VarList *cur = head;
  cur->var = new_local_var(type, expect_ident());

  while (!consume(")")) {
    expect(",");
    type = basetype();
    cur->next = (VarList *)calloc(1, sizeof(VarList));
    cur->next->var = new_local_var(type, expect_ident());
    cur = cur->next;
  }

  return (head);
}

static Node *stmt(void) {
  Node *node;
  Token *tok;

  if ((tok = consume("return"))) {
    node = new_unary_node(ND_RETURN, expr(), tok);
    expect(";");
    return (node);
  }

  if ((tok = consume("if"))) {
    expect("(");
    node = new_node(ND_IF, tok);
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else")) node->els = stmt();
    return (node);
  }

  if ((tok = consume("while"))) {
    expect("(");
    node = new_node(ND_WHILE, tok);
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return (node);
  }

  if ((tok = consume("for"))) {
    expect("(");
    node = new_node(ND_FOR, tok);
    if (!consume(";")) {
      if (is_type_dec())
        node->init = declaration();
      else {
        node->init = expr();
        for (Node *n = node->init->next; consume(","); n = n->next) n = expr();
      }
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->inc = expr();
      for (Node *n = node->inc->next; consume(","); n = n->next) n = expr();
      expect(")");
    }
    node->then = stmt();
    return (node);
  }

  if ((tok = consume("{"))) {
    Node head = {};
    Node *cur = &head;

    while (!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }

    node = new_node(ND_BLOCK, tok);
    node->body = head.next;
    return (node);
  }

  if (is_type_dec()) {
    node = declaration();
    expect(";");
    return (node);
  }

  node = expr();
  expect(";");
  return (node);
}

static Node *declaration(void) {
  Token *tok = token;
  Type *ty = basetype();
  Token *vardec;
  Var *var;

  vardec = consume_ident();
  if (!vardec) error_at(vardec->str, "不正な変数宣言です");

  ty = suffix(ty);

  var = new_local_var(ty, substr(vardec->str, vardec->len));

  if (!peek("=")) return (new_node(ND_NULL, tok));

  expect("=");

  return (new_binary_node(ND_ASSIGN, new_var_node(var, tok), assign(), tok));
}

static Node *expr(void) { return (assign()); }

static Node *assign(void) {
  Node *node = equality();
  Token *tok;

  if ((tok = consume("=")))
    node = new_binary_node(ND_ASSIGN, node, assign(), tok);
  return (node);
}

static Node *equality(void) {
  Node *node = relational();
  Token *tok;

  while (true) {
    if ((tok = consume("==")))
      node = new_binary_node(ND_EQ, node, relational(), tok);
    else if ((tok = consume("!=")))
      node = new_binary_node(ND_NEQ, node, relational(), tok);
    else
      return (node);
  }
}

static Node *relational(void) {
  Node *node = add();
  Token *tok;

  while (true) {
    if ((tok = consume("<=")))
      node = new_binary_node(ND_LTE, node, add(), tok);
    else if ((tok = consume(">=")))
      node = new_binary_node(ND_GTE, node, add(), tok);
    else if ((tok = consume("<")))
      node = new_binary_node(ND_LT, node, add(), tok);
    else if ((tok = consume(">")))
      node = new_binary_node(ND_GT, node, add(), tok);
    else
      return (node);
  }
}

static Node *add(void) {
  Node *node = mul();
  Token *tok;

  while (true) {
    if ((tok = consume("+")))
      node = new_binary_node(ND_ADD, node, mul(), tok);
    else if ((tok = consume("-")))
      node = new_binary_node(ND_SUB, node, mul(), tok);
    else
      return (node);
  }
}

static Node *mul(void) {
  Node *node = unary();
  Token *tok;

  while (true) {
    if ((tok = consume("*")))
      node = new_binary_node(ND_MUL, node, unary(), tok);
    else if ((tok = consume("/")))
      node = new_binary_node(ND_DIV, node, unary(), tok);
    else if ((tok = consume("%")))
      node = new_binary_node(ND_MOD, node, unary(), tok);
    else
      return (node);
  }
}

static Node *unary(void) {
  Token *tok;

  if ((tok = consume("sizeof")))
    return (new_unary_node(ND_SIZEOF, unary(), tok));
  if ((tok = consume("+"))) return (unary());
  if ((tok = consume("-")))
    return (new_binary_node(ND_SUB, new_number_node(0, token), unary(), tok));
  if ((tok = consume("&"))) return (new_unary_node(ND_ADDR, unary(), tok));
  if ((tok = consume("*"))) return (new_unary_node(ND_DEREF, unary(), tok));
  if ((tok = consume("++"))) return (new_unary_node(ND_PRE_INC, unary(), tok));
  if ((tok = consume("--"))) return (new_unary_node(ND_PRE_DEC, unary(), tok));
  return (postfix());
}

static Node *postfix(void) {
  Token *tok;
  Node *node = primary();

  while (true) {
    if ((tok = consume("["))) {
      node = new_binary_node(ND_ADD, node, expr(), tok);
      expect("]");
      node = new_unary_node(ND_DEREF, node, tok);
      continue;
    }
    if ((tok = consume("++"))) {
      node = new_unary_node(ND_POS_INC, node, tok);
      continue;
    }
    if ((tok = consume("--"))) {
      node = new_unary_node(ND_POS_DEC, node, tok);
      continue;
    }
    break;
  }
  return (node);
}

static Node *primary(void) {
  if (consume("(")) {
    Node *node = expr();
    consume(")");
    return (node);
  }

  Token *tok = consume_ident();
  if (tok) {
    // 関数呼び出し
    if (consume("(")) {
      Node *node = new_node(ND_FUNCALL, tok);
      node->funcname = substr(tok->str, tok->len);
      node->args = args();
      return (node);
    }

    // 変数
    Var *lvar = find_lvar(tok);
    if (!lvar) error_at(tok->str, "未定義の変数です");
    return (new_var_node(lvar, tok));
  }

  tok = consume_string();
  if (tok) {
    Type *ty = new_type_array(TYPE_ARRAY, new_type(TYPE_CHAR, NULL), tok->clen);
    Var *v = new_var(ty, new_label(), &globals);
    v->ctx = tok->ctx;
    v->clen = tok->clen;
    return (new_var_node(v, tok));
  }

  return (new_number_node(expect_number(), token));
}

static Node *args(void) {
  if (consume(")")) return (NULL);

  Node *head = assign();
  Node *cur = head;
  while (consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return (head);
}
