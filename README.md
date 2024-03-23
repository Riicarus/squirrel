# Squirrel

Squirrel is a simple compiled language.

## Lex Tokens

```txt
int float bool char string func void array

true false null

for while if else elseif continue break return

identifier lit

== != > >= < <=

+ - * / % & | ^ ~ << >>

= ++ --

&& || !

( ) [ ] { } -> , . ; : ? //

eof, not_exist, illegal
```

## Syntax

### Program

```txt
Program     :   [ { Statement ";" }... ]

Statement   :   Decl
            |   Expr
            |   Ctrl
            |   CodeBlock

CodeBlock   :   "{" [ { Statement ";" }... ] "}"
```

### Declaration

```txt
Decl        :   FieldDecl
            |   FuncDecl

FieldDecl   :   Type identifier [ "=" Expr ]

FuncDecl    :   "func" identifier "(" [ { FieldDecl "," }... ] ")" Type CodeBlock

Type        :   BasicType
            |   ArrayType

BasicType   :   "int"
            |   "float"
            |   "bool"
            |   "char"
            |   "string"
            |   "void"

ArrayType   :   "@" BasicType
```

### Expression

```txt
Expr        :   BinaryExpr

BinaryExpr  :   UnaryExpr
            |   BinaryExpr  BinaryOp BinaryExpr

BinaryOp    :   "==" | "!=" | "<" | "<=" | ">" | ">="
            |   "+"| "-" | "*" | "/" | "%" | "|" | "^" | "<<" | ">>"
            |   "="
            |   "&&" | "||"

UnaryExpr   :   PrimaryExpr
            |   UnaryOp UnaryExpr

UnaryOp     :   "!" | "~"

PrimaryExpr :   Operand
            |   PrimaryExpr Index
            |   PrimaryExpr Arguments
            |   PrimaryExpr { "++" | "--" }
            |   { "++" | "--" } PrimaryExpr
            |   "sizeof" "(" PrimaryExpr ")"

Operand     :   Literal
            |   identifier
            |   "(" Expr ")"

Literal     :   BasicLit
            |   ArrayLit

BasicLit    :   int_lit
            |   float_lit
            |   char_lit
            |   string_lit
            |   "true"
            |   "false"
            |   "null"

ArrayLit    :   "@" "{" [ { BasicLit "," }... ] "}"

Index       :   "[" Expr "]"

Arguments   :   "(" [ { Exprs "," }... ] ")"
```

### Control

```txt
Ctrl        :   Break
            |   Continue
            |   Return
            |   If
            |   For

Break       :   "break"

Continue    :   "continue"

Return      :   "return" [ Expr ]
```

#### If Statement

```txt
If          :   "if" "(" Expr ")" Statement [ Else ]

Else        :   [ ElseIf... ] [ EndElse ]

ElseIf      :   "elseif" "(" Expr ")" Statement

EndElse     :   "else" Statement
```

#### For Statement

```txt
For         :   "for" "(" [ ForInit ] ";" [ Expr ] ";" [ ForUpdate ] ")" { Statement | ";" }

ForInit     :   { { FieldDecl | Expr } "," }...

ForUpdate   :   { Expr "," }...
```
