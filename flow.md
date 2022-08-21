```
program		=	(global_var | function)*

basetype	=	("char" | "int") "*"*

global_var	=	basetype ident ("[" num "]")* ";"

function	=	basetype ident params "{" stmt* "}"

params		=	"(" (basetype ident ("," basetype ident)*)? ")"

stmt		=	"return" expr ";"
			|	"if" "(" expr ")" stmt ("else" stmt)?
			|	"while" "(" expr ")" stmt
			|	"for" "(" expr? ";" expr? ";" expr? ")" stmt
			|	"{" stmt* "}"
			|	declaration ";"
			|	expr ";"

declaration	=	basetype ident ("[" num "]")* ("=" assign)?

expr		=	assign

assign		=	equality ("=" assign)?

equality	=	relational ("==" relational | "!=" relational)*

relational	=	add ("<" add | "<=" add | ">" add | ">=" add)*

add			=	mul ("+" mul | "-" mul)*

mul			=	unary ("*" unary | "/" unary | "%" unary)*

unary		=	"sizeof" unary
			|	("+" | "-" | "&" | "*")? unary
			|	postfix

postfix		=	primary ("[" expr "]")*

primary		=	"(" expr ")"
			|	ident args?
			|	string
			|	num

args		=	"(" (assign ("," assign)*)? ")"
```
