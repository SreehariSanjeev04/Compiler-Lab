#ifndef CONSTANTS_H
#define CONSTANTS_H

// Maximum limit for registers and variables
#define MAX_REGISTERS 20
#define MAX_VARIABLES 26

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

// Type values
#define TYPE_INT 0
#define TYPE_BOOL 1
#define TYPE_VOID 2

#endif // CONSTANTS_H