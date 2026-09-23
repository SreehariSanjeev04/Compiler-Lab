#ifndef CONSTANTS_H
#define CONSTANTS_H

// Maximum limit for registers and variables
#define MAX_REGISTERS 20
#define MAX_VARIABLES 26
#define MAX_ARRAY_DIMENSION 10
#define MAX_ARRAY_ADDRESS 4121

// Node type values

#define NODE_TYPE_PLUS 0
#define NODE_TYPE_MINUS 1
#define NODE_TYPE_MUL 2
#define NODE_TYPE_DIV 3
#define NODE_TYPE_CONNECTOR 4
#define NODE_TYPE_READ 5
#define NODE_TYPE_WRITE 6
#define NODE_TYPE_ASSIGN 7
#define NODE_TYPE_NUM 8
#define NODE_TYPE_ID 9
#define NODE_TYPE_IF 10
#define NODE_TYPE_IF_ELSE 11
#define NODE_TYPE_WHILE 12
#define NODE_TYPE_GT 13
#define NODE_TYPE_LT 14
#define NODE_TYPE_GE 15
#define NODE_TYPE_LE 16
#define NODE_TYPE_EQ 17
#define NODE_TYPE_NE 18
#define NODE_TYPE_BREAKPOINT 19
#define NODE_TYPE_BREAK 20
#define NODE_TYPE_CONTINUE 21
#define NODE_TYPE_REPEAT_UNTIL 22
#define NODE_TYPE_DO_WHILE 23
#define NODE_TYPE_STRING 24
#define NODE_TYPE_ARRAY 25
#define NODE_TYPE_DEREF 26
#define NODE_TYPE_ADDRESS 27
#define NODE_TYPE_FUNC_CALL 28
#define NODE_TYPE_ARG 29
#define NODE_TYPE_RETURN 30
#define NODE_TYPE_AND 31
#define NODE_TYPE_OR 32
#define NODE_TYPE_NOT 33
#define NODE_TYPE_MOD 34
#define NODE_TYPE_TUPLE 35

// Type values
#define TYPE_INT 0
#define TYPE_BOOL 1
#define TYPE_VOID 2
#define TYPE_STRING 3
#define TYPE_TUPLE 4

// To do - look into void type pointer

#define DEFAULT_BINDING_ADDRESS 4096
#define DEFAULT_VAR_SIZE 1

#endif // CONSTANTS_H