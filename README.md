# Squirrel

Squirrel is a simple compiled language.

## Lex Tokens

```txt
int float bool char string func void

true false

for if else elseif continue break return

identifier lit

== != > >= < <=

+ - * / % & | ^ << >> && ||

~ !

= ++ --

( ) [ ] { } -> , . ; : ? //

eof, not_exist, illegal
```

## Syntax

### Program

```txt
Program         :   CodeBlock

Statement       :   Decl
                |   Expr
                |   Ctrl
                |   CodeBlock

CodeBlock   :   "{" [ { Statement ";" }... ] "}"
```

### Declaration

```txt
Decl            :   FieldDecl
                |   FuncDecl

FieldDecl       :   TypeDecl identifier [ "=" Expr ]

FuncDecl        :   "func" identifier "(" [ { FieldDecl "," }... ] ")" TypeDecl CodeBlock

TypeDecl        :   BasicTypeDecl

BasicTypeDecl   :   "int"
                |   "float"
                |   "bool"
                |   "char"
                |   "string"
                |   "void"
```

### Expression

```txt
Expr            :   BinaryExpr

BinaryExpr      :   UnaryExpr
                |   BinaryExpr BinaryOp BinaryExpr

BinaryOp        :   "==" | "!=" | "<" | "<=" | ">" | ">="
                |   "+"| "-" | "*" | "/" | "%" | "|" | "^" | "<<" | ">>"
                |   "="
                |   "&&" | "||"

UnaryExpr       :   PrimaryExpr
                |   UnaryOp UnaryExpr

UnaryOp         :   "!" | "~"

PrimaryExpr     :   Operand
                |   PrimaryExpr Arguments
                |   PrimaryExpr { "++" | "--" }
                |   { "++" | "--" } PrimaryExpr

Operand         :   Literal
                |   identifier
                |   "(" Expr ")"

Literal         :   BasicLit

BasicLit        :   int_lit
                |   float_lit
                |   char_lit
                |   string_lit
                |   "true"
                |   "false"

Arguments       :   "(" [ { Expr "," }... ] ")"
```

### Control

```txt
Ctrl            :   Break
                |   Continue
                |   Return
                |   If
                |   For

Break           :   "break"

Continue        :   "continue"

Return          :   "return" [ Expr ]
```

#### If Statement

```txt
If              :   "if" "(" Expr ")" CodeBlock [ Else ]

Else            :   [ ElseIf... ] [ EndElse ]

ElseIf          :   "elseif" "(" Expr ")" CodeBlock

EndElse         :   "else" CodeBlock
```

#### For Statement

```txt
For             :   "for" "(" [ ForInit ] ";" [ Expr ] ";" [ ForUpdate ] ")" CodeBlock

ForInit         :   { { FieldDecl | Expr } "," }...

ForUpdate       :   { Expr "," }...
```
