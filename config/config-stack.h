#ifndef _CONFIG_STACK_H
#define _CONFIG_STACK_H

int yyparse (void);
extern FILE *yyin;

struct value {
	char *id;
	char *val;
	struct value *next;
};


struct node {
	char *id;
	struct node *nodes;
	struct value *values;
	struct node *next;
};


struct parser_context {
	struct value *val_list;
	struct node *node_list;
	struct parser_context *next;
};

extern struct value *val_list;
extern struct node *node_list;
extern struct parser_context *context_stack;

#endif