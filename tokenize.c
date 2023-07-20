#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LEN 255
#define MAX_TOKEN_LEN 255
#define MAX_NUM_TOKENS 100

// Define the types of tokens
typedef enum {
    TOKEN_NORMAL,
    TOKEN_INPUT_REDIRECTION,
    TOKEN_OUTPUT_REDIRECTION,
    TOKEN_PIPE,
    TOKEN_BACKGROUND,
    TOKEN_SEMICOLON,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN
} TokenType;

// Define a structure to hold a token
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

/**
 * Check if a character is whitespace.
 */
int is_whitespace(char ch) {
    return ch == ' ' || ch == '\t';
}

/**
 * Check if a character is a special token character.
 */
int is_special_token_char(char ch) {
    return ch == '<' || ch == '>' || ch == '|' || ch == '&' || ch == ';' || ch == '(' || ch == ')';
}

/**
 * Get the type of a token based on its value.
 */
TokenType get_token_type(const char *token) {
    if (strcmp(token, "<") == 0) {
        return TOKEN_INPUT_REDIRECTION;
    } else if (strcmp(token, ">") == 0) {
        return TOKEN_OUTPUT_REDIRECTION;
    } else if (strcmp(token, "|") == 0) {
        return TOKEN_PIPE;
    } else if (strcmp(token, "&") == 0) {
        return TOKEN_BACKGROUND;
    } else if (strcmp(token, ";") == 0) {
        return TOKEN_SEMICOLON;
    } else if (strcmp(token, "(") == 0) {
        return TOKEN_LEFT_PAREN;
    } else if (strcmp(token, ")") == 0) {
        return TOKEN_RIGHT_PAREN;
    } else {
        return TOKEN_NORMAL;
    }
}

/**
 * Tokenize the input string and return an array of tokens.
 */
Token *tokenize(const char *input, int *num_tokens) {
    Token *tokens = (Token *) malloc(sizeof(Token) * MAX_NUM_TOKENS);
    *num_tokens = 0;

    int i = 0;
    while (input[i] != '\0') {
        // Skip whitespace
        while (is_whitespace(input[i])) {
            i++;
        }

        // Check for special token characters
         if (is_special_token_char(input[i])) {
            tokens[*num_tokens].type = get_token_type(&input[i]);
            strncpy(tokens[*num_tokens].value, &input[i], 1);
            tokens[*num_tokens].value[1] = '\0';
            i++;
            (*num_tokens)++;
            continue;
        }

        // Read a normal token
        int j = 0;
	if (input[i] == '\"'){
	i++;
        while (input[i] != '\"' && input[i]!= '\0') {
            tokens[*num_tokens].type = TOKEN_NORMAL;
            tokens[*num_tokens].value[j] = input[i];
            i++;
            j++;

            // If the token is too long, exit with an error
            if (j >= MAX_TOKEN_LEN) {
                fprintf(stderr, "Token too long.\n");
                exit(1);
            }
        }
	if (input[i] == '\"') {
                i++;
            }
        } else {
            // Read a normal token
            while (input[i] != '\0' && !is_whitespace(input[i]) && !is_special_token_char(input[i])) {
                tokens[*num_tokens].type = TOKEN_NORMAL;
                tokens[*num_tokens].value[j] = input[i];
                i++;
                j++;

                // If the token is too long, exit with an error
                if (j >= MAX_TOKEN_LEN) {
                    fprintf(stderr, "Token too long.\n");
                    exit(1);
                }
            }
        }
        tokens[*num_tokens].value[j] = '\0';
        (*num_tokens)++;
    }

    return tokens;
}
//int main(int argc, char **argv) {
//    char input[MAX_INPUT_LEN];
//    int num_tokens;
//    Token *tokens;
//    fgets(input, MAX_INPUT_LEN, stdin); 
//    input[strlen(input)-1] = '\0'; // Remove newline character
//tokens = tokenize(input, &num_tokens);

// Print out the tokens
//for (int i = 0; i < num_tokens; i++) {
//    printf("%s\n",tokens[i].value);
//}

// Free the memory used by the tokens
//free(tokens);

//return 0;}
