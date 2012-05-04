#ifndef RPN_FUNCTIONS
#define RPN_FUNCTIONS

/* function types
 * 0 - value
 * 1 - addition
 * 2 - substraction
 * 3 - multiplication
 * 4 - division
 */

struct rpn_node {
  int type;
  double value;
  struct rpn_node *next;
} rpn_node;

#endif // RPN_FUNCTIONS
