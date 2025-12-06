# BLISSP = BLISS + LISP
A tiny interpreter in 128 lines of code
### Syntax
```
expr = [term] {op term}.
op   = "+" | "-" | "*" | "/" | "%" | 
       "&" | "|" | "!" | 
       "=" | "#" | "<" | ">" | 
       ":" | "?" | "@" | "$" | ",".
term = var ["'"] | num | "(" expr ")".
var  = letter {letter}.
num  = digit {digit}.
```
