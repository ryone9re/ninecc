```
program		=	function*

function	=	type ident params "{" stmt* "}"

params		=	"(" (type ident ("," type ident)*)? ")"

stmt		=	"return" expr ";"
			|	"if" "(" expr ")" stmt ("else" stmt)?
			|	"while" "(" expr ")" stmt
			|	"for" "(" expr? ";" expr? ";" expr? ")" stmt
			|	"{" stmt* "}"
			|	declaration ";"
			|	expr ";"

declaration	=	type ident

expr		=	assign

assign		=	equality ("=" assign)?

equality	=	relational ("==" relational | "!=" relational)*

relational	=	add ("<" add | "<=" add | ">" add | ">=" add)*

add			=	mul ("+" mul | "-" mul)*

mul			=	unary ("*" unary | "/" unary)*

unary		=	("+" | "-" | "&" | "*")? unary | primary

primary		=	"(" expr ")"
			|	ident args?
			|	num

args		=	"(" (assign ("," assign)*)? ")"

type		=	"int" "*"*
```
