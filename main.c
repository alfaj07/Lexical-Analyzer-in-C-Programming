#include <stdio.h>
#include <string.h>
#include "lexer.h"

#define MAX_ISSUES 200

typedef struct {
    int line;
    char lexeme[MAX_TOKEN_SIZE];
    char message[MAX_TOKEN_SIZE];
} Issue;

static const char* getTokenTypeName(TokenType type) {
    switch (type) {
        case KEYWORD:           return "KEYWORD";
        case OPERATOR:          return "OPERATOR";
        case SPECIAL_CHARACTER: return "SPECIAL_CHARACTER";
        case CONSTANT:          return "CONSTANT";
        case IDENTIFIER:        return "IDENTIFIER";
        case UNKNOWN:           return "UNKNOWN";
        default:                return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file.c>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening source file");
        return 1;
    }
    fclose(fp);

    initializeLexer(argv[1]);

    Issue issues[MAX_ISSUES];
    int issueCount = 0;

    printf("+----------------------+--------------------+-------+\n");
    printf("| Lexeme               | Token Type Name    | Code  |\n");
    printf("+----------------------+--------------------+-------+\n");

    Token token;
    while ((token = getNextToken()).lexeme[0] != '\0') {
        if (token.warning[0] != '\0' && strcmp(token.warning, "-") != 0) {
            if (issueCount < MAX_ISSUES) {
                issues[issueCount].line = token.line;
                snprintf(issues[issueCount].lexeme, MAX_TOKEN_SIZE, "%s", token.lexeme);
                snprintf(issues[issueCount].message, MAX_TOKEN_SIZE, "%s", token.warning);
                issueCount++;
            }
        }

        printf("| %-20s | %-18s |   %d   |\n",
               token.lexeme,
               getTokenTypeName(token.type),
               token.type);
    }

    printf("+----------------------+--------------------+-------+\n");

    // If any warnings or errors occurred, print a summary table with line numbers
    if (issueCount > 0) {
        printf("\n+---------+----------------------+------------------------------------+\n");
        printf("| Line No | Lexeme               | Warning / Error Description        |\n");
        printf("+---------+----------------------+------------------------------------+\n");
        for (int i = 0; i < issueCount; i++) {
            printf("| %-7d | %-20s | %-34s |\n",
                   issues[i].line,
                   issues[i].lexeme,
                   issues[i].message);
        }
        printf("+---------+----------------------+------------------------------------+\n");
    }

    return 0;
}
