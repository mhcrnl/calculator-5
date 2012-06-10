#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "rpn.h"

int rpn_opcode(char op) {
  int result;
  switch(op) {
    case '+': result = 1; break;
    case '-': result = 2; break;
    case '*': result = 3; break;
    case '/': result = 4; break;
    case '^': result = 5; break;
    default: result = 0; break;
  }
  return result;
}

bool rpn_isop(char token) {
  return token == '+' || token == '-' || token == '*' || token == '/' || token == '^';
}

int rpn_functioncode(char *token) {
  if(strcmp(token, "sqrt") == 0) return 7;
  else return 0;
}

bool rpn_isnum(char *num) {
  bool decimal = false;
  int i;
  for(i=0; i<strlen(num); i++) {
    if(num[i] == '.') {
      if(decimal) return false;
      else decimal = true;
    }
    else if(num[i]-48 < 0 || num[i]-48 > 9) return false;
  }
  return true;
}

int rpn_precedence(int op) {
  switch(op) {
    case 1: op = 1; break;
    case 2: op = 1; break;
    case 3: op = 2; break;
    case 4: op = 2; break;
    case 5: op = 3; break;
  }
  return op;
}

bool rpn_left_assoc(int op) {
  if(op <= 4) return true;
  else return false;
}

double rpn_parsenum(char *num) {
  int decimal = 0, i;
  double result = 0;
  for(i=0; i<strlen(num); i++) {
    if(num[i] == '.') decimal = 10;
    else {
      if(decimal > 0) {
        result += (double)(num[i]-48)/decimal;
        decimal *= 10; 
      }
      else {
        result *= 10;
        result += (num[i]-48);
      }
    }
  }
  return result;
}

void rpn_push(struct rpn_node ** rpn_stack, struct rpn_node * new_node) {
  new_node->next = *rpn_stack;
  *rpn_stack = new_node;
}

void rpn_pop(struct rpn_node ** rpn_stack, bool fr) {
  if(*rpn_stack != NULL) {
    struct rpn_node *stack_ptr = *rpn_stack;
    *rpn_stack = stack_ptr->next;
    if(fr)
      free(stack_ptr);
  }
}

struct rpn_node* rpn_create(int type, double value) {
  struct rpn_node *new_node = (struct rpn_node*)malloc(sizeof(struct rpn_node));
  new_node->type = type;
  new_node->value = value;
  return new_node;
}

void rpn_parse(char **input, struct rpn_node *result[], int *result_size, int *error) {
  int i=0, input_size = *result_size, op = 0;
  *result_size = 0;
  char *token;
  struct rpn_node * op_stack = 0;
  for(i=0; i<input_size; i++) {
    token = input[i];
    if(rpn_isnum(token)) {
      result[*result_size] = rpn_create(0, rpn_parsenum(token));
      (*result_size) ++;
    }
    else if(rpn_functioncode(token) > 0) {
      rpn_push(&op_stack, rpn_create(rpn_functioncode(token), 0));
    }
    else if(rpn_isop(token[0])) {
      op = rpn_opcode(token[0]);
      while(op_stack != NULL && ((rpn_left_assoc(op_stack->type) && rpn_precedence(op_stack->type) >= rpn_precedence(op)) || (!rpn_left_assoc(op_stack->type) && rpn_precedence(op_stack->type) > rpn_precedence(op))) ) {
        result[*result_size] = op_stack;
        (*result_size) ++;
        rpn_pop(&op_stack, false);
      }
      rpn_push(&op_stack, rpn_create(op, 0));
    }
    else if(strcmp(token, "(") == 0) {
      rpn_push(&op_stack, rpn_create(-1, 0));
    }
    else if(strcmp(token, ")") == 0) {
      bool found_parenthesis = false;
      while(op_stack != NULL) {
        if(op_stack->type == -1) {
          found_parenthesis = true;
          rpn_pop(&op_stack, true);
          break;
        }
        else {
          result[*result_size] = op_stack;
          (*result_size) ++;
          rpn_pop(&op_stack, false);
        }
      }
      if(!found_parenthesis) {
        *error = 1;
      }
      if(op_stack != NULL && op_stack->type >= 7) { // function
        result[*result_size] = op_stack;
        (*result_size) ++;
        rpn_pop(&op_stack, false);
      }
    }
    else {
      *error = 1;
      return;
    }
  }
  while(op_stack != NULL) {
    result[*result_size] = op_stack;
    (*result_size) ++;
    rpn_pop(&op_stack, false);
  }
  /*
  printf("finishing! rpn size: %d\n", *result_size);
  for(i=0; i<*result_size; i++) {
    printf("> %.3f - %d\n", result[i]->value, result[i]->type);
  }
  printf("finished\n");
  */
}

void rpn_getargs(struct rpn_node ** rpn_stack, int size, double result[]) {
  while(size>0) {
    result[size-1] = (*rpn_stack)->value;
    rpn_pop(rpn_stack, true);
    size--;
  }
}

void rpn_addition(struct rpn_node ** rpn_stack) {
  double args[2];
  rpn_getargs(rpn_stack, 2, args);
  rpn_push(rpn_stack, rpn_create(0, args[0]+args[1]));
}

void rpn_substraction(struct rpn_node ** rpn_stack) {
  double args[2];
  rpn_getargs(rpn_stack, 2, args);
  rpn_push(rpn_stack, rpn_create(0, args[0]-args[1]));
}

void rpn_multiplication(struct rpn_node ** rpn_stack) {
  double args[2];
  rpn_getargs(rpn_stack, 2, args);
  rpn_push(rpn_stack, rpn_create(0, args[0]*args[1]));
}

void rpn_division(struct rpn_node ** rpn_stack) {
  double args[2];
  rpn_getargs(rpn_stack, 2, args);
  rpn_push(rpn_stack, rpn_create(0, args[0]/args[1]));
}

void rpn_exponentation(struct rpn_node ** rpn_stack) {
  double args[2];
  rpn_getargs(rpn_stack, 2, args);
  rpn_push(rpn_stack, rpn_create(0, pow(args[0], args[1])));
}

void rpn_square_root(struct rpn_node ** rpn_stack) {
  double args[1];
  rpn_getargs(rpn_stack, 1, args);
  rpn_push(rpn_stack, rpn_create(0, sqrt(args[0])));
}

void rpn_tokenize(char *cinput, char *output[], int *size, int *error) {
  char *buffer = malloc(100*sizeof(char));
  char *nbuffer = malloc(100*sizeof(char));
  char *input = malloc(100*sizeof(char));
  strcpy(buffer, "");
  strcpy(nbuffer, "");
  strcpy(input, cinput);
  char current;
  int i;
  *size = 0;
  for(i=0; i<strlen(input); i++) {
    current = input[i];
    if(current == ' ') continue;
    else if((current-48 >= 0 && current-48 <=9) || current == 46) {
      if(strlen(buffer) > 0) {
        output[*size] = malloc(30*sizeof(char));
        strcpy(output[*size], buffer);
        strcpy(buffer, "");
        (*size) ++;
      }
      if(strlen(nbuffer)==0) strcpy(nbuffer, "");
      strncat(nbuffer, &current, 1);
    }
    else if(rpn_isop(current) || current == '(' || current == ')') {
      if(strlen(nbuffer) > 0) {
        output[*size] = malloc(30*sizeof(char));
        strcpy(output[*size], nbuffer);
        strcpy(nbuffer, "");
        (*size) ++;
      }
      if(strlen(buffer) > 0) {
        output[*size] = malloc(30*sizeof(char));
        strcpy(output[*size], buffer);
        strcpy(buffer, "");
        (*size) ++;
      }
      output[*size] = malloc(30*sizeof(char));
      output[*size][0] = current;
      output[*size][1] = 0;
      (*size) ++;
    }
    else {
      strncat(buffer, &current, 1);
    }
  }
  if(strlen(nbuffer) > 0) {
    output[*size] = malloc(30*sizeof(char));
    strcpy(output[*size], nbuffer);
    strcpy(nbuffer, "");
    (*size) ++;
  }
  free(nbuffer);
  free(buffer);
  free(input);
}

void rpn_resolve(char *input, double *result, int *error) {
  *error = 0;
  *result = 0;
  if(strlen(input) == 0) {
    *error = 1;
    return;
  }
  int rpn_size = 100, i;

  char *tokenized_input[1000];
  rpn_tokenize(input, tokenized_input, &rpn_size, error);

  struct rpn_node *rpn_expression[rpn_size];
  rpn_parse(tokenized_input, rpn_expression, &rpn_size, error);

  if(*error > 0) return;
  struct rpn_node *rpn_stack = 0;
  double (*functions[8])(struct rpn_node ** rpn_stack) = {
    NULL,
    rpn_addition,
    rpn_substraction,
    rpn_multiplication,
    rpn_division,
    rpn_exponentation,
    NULL,
    rpn_square_root
  };
  for(i=0; i<rpn_size; i++) {
    if(rpn_expression[i]->type == 0) {
      rpn_push(&rpn_stack, rpn_expression[i]);
    }
    else {
      functions[rpn_expression[i]->type](&rpn_stack);
      free(rpn_expression[i]);
    }
  }
  *result = rpn_stack->value;
  while(rpn_stack != NULL) rpn_pop(&rpn_stack, true);
}

