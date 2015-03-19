#include <stdio.h>
#include <stdlib.h>
#include "shellparser.h"
#include "y.tab.h"

// Functions for creating different types of nodes and
// adding values and node pointers to them.
// A CommandNode, PipeNode, ParamNode or ParamsNode is
// inserted into a Node.
Node *new_command(char* command, Node *childparams) {
  Node* newnode = (Node*) malloc(sizeof(Node));
  newnode->label = command_node;
  newnode->type.CommandNode.command = command;
  newnode->type.CommandNode.childparams = childparams;
  return newnode;
}

Node *new_pipe(Node *command, Node *pipe) {
  Node* newnode = (Node*) malloc(sizeof(Node));
  newnode->label = pipe_node;
  newnode->type.PipeNode.command = command;
  newnode->type.PipeNode.pipe = pipe;
  return newnode;
}

Node *new_param(char* param) {
  Node* newnode = (Node*) malloc(sizeof(Node));
  newnode->label = param_node;
  newnode->type.ParamNode.param = param;
  return newnode;
}

Node *new_params(Node *first, Node *second) {
  Node* newnode = (Node*) malloc(sizeof(Node));
  newnode->label = params_node;
  newnode->type.ParamsNode.first = first;
  newnode->type.ParamsNode.second = second;
  return newnode; 
}

// Recursive function to free nodes...
void freeNode(Node *np){
  if (np == NULL) { // Return if there are no nodes left.
    return;
  } else {
    switch (np->label) {
      // For each case, based on the node tag,
      // free the value, or free the nodes it 
      // points to recursively.
      case command_node:
        free(np->type.CommandNode.command);
        freeNode(np->type.CommandNode.childparams);
        break;
      case pipe_node:
        freeNode(np->type.PipeNode.command);
        freeNode(np->type.PipeNode.pipe);
        break;
      case params_node:
        freeNode(np->type.ParamsNode.first);
        freeNode(np->type.ParamsNode.second);
        break;
      case param_node:
        free(np->type.ParamNode.param);
        break;
      default:
        fprintf(stderr, "Error in freeNode(): Invalid Node Type.\n");
        exit(-1);
    }   
    free(np); // Free the np.
  }
}

// Recursive function to print node values. We only need this
// for debugging. Same general idea as freeNode() above,
// except returning error values.
int printNode(Node *np) { 
  if (np == NULL) {
    return 0;
  } else {
    int ret = 0;
    switch (np->label) {
      case command_node:
        printf("%s \n", (np->type.CommandNode.command));
        if (printNode(np->type.CommandNode.childparams) < 0) {
          ret = -1;
          break;
        }
        break;
      case pipe_node:
        if (printNode(np->type.PipeNode.command) < 0) {
          ret = -1;
          break;
        }
        if (np->type.PipeNode.pipe != NULL) {
          printf(" | \n");
          if (printNode(np->type.PipeNode.pipe) < 0) {
            ret = -1;
            break;
          }
        }
        break;
      case param_node:
        printf("%s \n", (np->type.ParamNode.param));
        break;
      case params_node:
        if (printNode(np->type.ParamsNode.first) < 0) {
          ret = -1;
          break;
        }
        if (printNode(np->type.ParamsNode.second) < 0) {
          ret = -1;
          break;
        }
        break;
      default:
        fprintf(stderr, "Error: cannot print node of invalid type");
        ret = -1;
        break;
    }
    return ret;
  }
}

// This is what will be called for each line of input. Determine the
// type of node and pass it to a function for further evaluation.
int evalNode(Node *np) {  
  if (np == NULL) {
    return 0;
  } else {
    int ret = 0;
    switch (np->label) {
      case command_node:
        evalCommand(np);
        break;
      case pipe_node:
        evalPipe(np);
        break;
      default:
        fprintf(stderr, "Error: cannot print node of invalid type");
        ret = -1;
        break;
    }
    return ret;
  }
}

// Recursively count the number of params. The params node is a
// right-skewed binary tree, so we only need to follow the
// second node pointers.
int countArgs(Node *paramsptr) {
  int count = 0;
  if (paramsptr != NULL) {
    count = 1 + countArgs(paramsptr->type.ParamsNode.second);
  }
  return count;
}

// Evaluate a single command.
void evalCommand(Node *np) 
{
  printf("Evaluating command\n"); // Debug statement.
  int process;
  process = fork(); // Create a new process for the command.
  if (process > 0) { // If the process is > 0, we are still the parent,
    wait((int *) 0); // so wait. (Null pointer - return value not saved.)
  }
  else if (process == 0) { // If process == 0, we are in the child...
    char *command = np->type.CommandNode.command; // Get the command name string.
    Node *childparams = np->type.CommandNode.childparams; // Get the childparams node.
    int numparams = countArgs(childparams); // Count the number of params.
    printf("Numparams: %d\n", numparams); // Print numparams, for debugging.
    char *paramslist[numparams+2]; // Array of the params.
    int i = 0;
    paramslist[0] = command; // The first param in the array is always the command name.
    paramslist[numparams+1] = NULL; // The last thing in the array must be null.
    Node *curr = childparams; // A node pointer that will indicate the current node.
    for (i = 0; i < numparams; i++) {
      // Basically, add the name of the param that is in the first node.
      // (Remember, the params are a right-skewed binary tree.)
      paramslist[i+1] = (curr->type.ParamsNode.first)->type.ParamNode.param;
      printf("Param str: %s\n", (curr->type.ParamsNode.first)->type.ParamNode.param); // Debugging
      curr = curr->type.ParamsNode.second; // Make curr point to the second node.
    }
    if (execvp(command, paramslist) == -1) { // Execute the command with execvp().
      fprintf(stderr, "Can't execute %s\n.", command); // Tell us if there's an error.
      exit(1); // Exit with a non-zero status.
    } else {
      exit(0); // Exit with a zero status (no problems).
    }
  }
  else if (process == -1) { // If process == -1, then something went wrong.
    fprintf(stderr, "Can't fork!"); // Error message.
    exit(2); // Exit with a non-zero status.
  }
}

// Here we will evaluate multiple commands that are 
// piped together.
void evalPipe(Node *np) 
{
  printf("Evaluating pipe\n");

}
