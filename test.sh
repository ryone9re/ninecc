#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 200 "		  		 200 + 0;"
assert 0 "0+0;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 100 "-10*-10;"
assert 35 "-5*-(10-3);"
assert 10 "-10+20;"
assert 30 "+(7+8)++(20-5);"
assert 45 "-(10-40)--(10+5);"
assert 1 "1<2;"
assert 1 "2>1;"
assert 1 "1<=1;"
assert 1 "1>=1;"
assert 1 "1==1;"
assert 1 "1!=2;"
assert 0 "2<1;"
assert 0 "1>2;"
assert 0 "2<=1;"
assert 0 "1>=2;"
assert 0 "1==2;"
assert 0 "1!=1;"
assert 14 "a = 3; b = 5 * 6 - 8; a + b / 2;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"
assert 5 "return 5;"
assert 6 "return (6);"
assert 14 "a = 3; b = 5 * 6 - 8; return a + b / 2;"

echo OK
