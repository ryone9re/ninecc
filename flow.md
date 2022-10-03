# Flow

```text
program  = (func_dec | global_var | function)*

basetype = ("char" | "int") "*"*

suffix  = ("[" num "]")

func_dec = basetype ident params ";"

global_var = basetype ident suffix* ";"

function = basetype ident params "{" stmt* "}"

params  = "(" (basetype ident ("," basetype ident)*)? ")"

stmt  = "return" expr ";"
   | "if" "(" expr ")" stmt ("else" stmt)?
   | "while" "(" expr ")" stmt
   | "for" "(" (declaration | (expr ("," expr)*))? ";" expr? ";" (expr, ("," expr)*)? ")" stmt
   | "{" stmt* "}"
   | declaration ";"
   | expr ";"

declaration = basetype ident suffix* ("=" assign)?

expr  = assign

assign  = equality ("=" assign)?

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | ">=" add)*

add   = mul ("+" mul | "-" mul)*

mul   = unary ("*" unary | "/" unary | "%" unary)*

unary  = "sizeof" unary
   | ("+" | "-" | "&" | "*")? unary
   | ("++" | "--") unary
   | postfix

postfix  = primary ("[" expr "]" | "++" | "--")*

primary  = "(" expr ")"
   | ident args?
   | string
   | num

args  = "(" (assign ("," assign)*)? ")"
```
