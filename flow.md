```
program		=	function*
function	=	ident "(" params? ")" "{" stmt* "}"
params		=	(ident ("," ident)*)?
stmt		=	"return" expr ";"
				| "if" "(" expr ")" stmt ("else" stmt)?
				| "while" "(" expr ")" stmt
				| "for" "(" expr? ";" expr? ";" expr? ")" stmt
				| "{" stmt* "}"
				| expr ";"
expr		=	assign
assign		=	equality ("=" assign)?
equality	=	relational ("==" relational | "!=" relational)*
relational	=	add ("<" add | "<=" add | ">" add | ">=" add)*
add			=	mul ("+" mul | "-" mul)*
mul			=	unary ("*" unary | "/" unary)*
unary		=	("+" | "-")? unary | primary
primary		=	"(" expr ")"
				| ident args?
				| num
args		=	"(" (assign ("," assign)*)? ")"
```
