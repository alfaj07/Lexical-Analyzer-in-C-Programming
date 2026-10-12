#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static FILE* sourceFile = NULL;
static int currentLine = 1;

#define MAX_PENDING_TOKENS 16
static Token pendingTokens[MAX_PENDING_TOKENS];
static int pendingHead = 0;
static int pendingTail = 0;
static int pendingCount = 0;

static void enqueueToken(const Token* t) {
    if (pendingCount < MAX_PENDING_TOKENS) {
        pendingTokens[pendingTail] = *t;
        pendingTail = (pendingTail + 1) % MAX_PENDING_TOKENS;
        pendingCount++;
    }
}

static int dequeueToken(Token* t) {
    if (pendingCount > 0) {
        *t = pendingTokens[pendingHead];
        pendingHead = (pendingHead + 1) % MAX_PENDING_TOKENS;
        pendingCount--;
        return 1;
    }
    return 0;
}

static const char* keywords[MAX_KEYWORDS] = {
    "int", "float", "return", "if", "else", "while", "for", "do", "break", "continue",
    "char", "double", "void", "switch", "case", "default", "const", "static", "sizeof", "struct"
};

static const char* operators = "+-*/%=!<>|&";
static const char* specialCharacters = ",;{}()[]";
static const char* escapeChars = "ntr0abfv\\'\"";

static int parenDepth = 0;
static int braceDepth = 0;

void initializeLexer(const char* filename) {
    pendingHead = 0;
    pendingTail = 0;
    pendingCount = 0;
    parenDepth = 0;
    braceDepth = 0;
    currentLine = 1;

    if (sourceFile != NULL && sourceFile != stdin) {
        fclose(sourceFile);
        sourceFile = NULL;
    }

    if (filename == NULL) {
        fprintf(stderr, "Usage: lexical_analyzer <filename>\n");
        sourceFile = NULL;
        return;
    }

    sourceFile = fopen(filename, "r");
    if (sourceFile == NULL) {
        perror("Error opening source file");
    }
}

int isKeyword(const char* str) {
    if (str == NULL) return 0;
    for (int i = 0; i < MAX_KEYWORDS; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int isSpecialCharacter(char ch) {
    return (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\0' ||
            ch == '\a' || ch == '\b' || ch == '\f' || ch == '\v' ||
            strchr(specialCharacters, ch) != NULL);
}

int isSpecialEscapeSequence(const char* str) {
    if (str == NULL || str[0] != '\\' || str[1] == '\0' || str[2] != '\0') {
        return 0;
    }
    return (strchr(escapeChars, str[1]) != NULL);
}

int isOperator(const char* str) {
    if (str == NULL || str[0] == '\0') return 0;
    if (strlen(str) == 1) {
        return (strchr(operators, str[0]) != NULL);
    }
    if (strlen(str) == 2) {
        const char* compoundOps[] = {
            "==", "!=", "<=", ">=", "&&", "||",
            "++", "--", "+=", "-=", "*=", "/=", "%="
        };
        for (size_t i = 0; i < sizeof(compoundOps) / sizeof(compoundOps[0]); i++) {
            if (strcmp(str, compoundOps[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

int isConstant(const char* str) {
    if (str == NULL || str[0] == '\0') return 0;
    size_t len = strlen(str);

    // String literal: "..."
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        return 1;
    }

    // Character literal: '...'
    if (len >= 2 && str[0] == '\'' && str[len - 1] == '\'') {
        return 1;
    }

    // Numeric constant (integer or floating-point)
    size_t i = 0;
    if (str[i] == '+' || str[i] == '-') {
        i++;
    }
    if (i == len) return 0;

    int hasDigits = 0;
    int hasDot = 0;
    for (; i < len; i++) {
        if (isdigit((unsigned char)str[i])) {
            hasDigits = 1;
        } else if (str[i] == '.' && !hasDot) {
            hasDot = 1;
        } else {
            return 0;
        }
    }
    return hasDigits;
}

int isIdentifier(const char* str) {
    if (str == NULL || str[0] == '\0') return 0;
    if (!isalpha((unsigned char)str[0]) && str[0] != '_') {
        return 0;
    }
    for (size_t i = 1; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_') {
            return 0;
        }
    }
    return 1;
}

void categorizeToken(Token* token) {
    if (token == NULL) return;
    if (token->lexeme[0] == '\0') {
        token->type = UNKNOWN;
        return;
    }

    if (isSpecialEscapeSequence(token->lexeme)) {
        token->type = SPECIAL_CHARACTER;
        return;
    }

    if (isKeyword(token->lexeme)) {
        token->type = KEYWORD;
    } else if (isIdentifier(token->lexeme)) {
        token->type = IDENTIFIER;
    } else if (isConstant(token->lexeme)) {
        token->type = CONSTANT;
    } else if (isOperator(token->lexeme)) {
        token->type = OPERATOR;
    } else if (token->lexeme[1] == '\0' && isSpecialCharacter(token->lexeme[0])) {
        token->type = SPECIAL_CHARACTER;
    } else {
        token->type = UNKNOWN;
        if (token->warning[0] == '\0') {
            snprintf(token->warning, MAX_TOKEN_SIZE, "Unrecognized symbol");
        }
    }
}

Token getNextToken() {
    Token token;
    memset(&token, 0, sizeof(Token));
    token.type = UNKNOWN;

    // Return any pending buffered tokens first
    if (dequeueToken(&token)) {
        return token;
    }

    if (sourceFile == NULL) {
        return token;
    }

    int ch;
    while ((ch = fgetc(sourceFile)) != EOF) {
        // Track line numbers on newlines
        if (ch == '\n') {
            currentLine++;
        }

        // Skip whitespace
        if (isspace((unsigned char)ch)) {
            continue;
        }

        // Handle preprocessor directives: #include, #define, etc.
        if (ch == '#') {
            while ((ch = fgetc(sourceFile)) != EOF && ch != '\n') {
                // skip until end of line
            }
            if (ch == '\n') {
                currentLine++;
            }
            continue;
        }

        // Handle comments: // or /* ... */
        if (ch == '/') {
            int next = fgetc(sourceFile);
            if (next == '/') {
                while ((ch = fgetc(sourceFile)) != EOF && ch != '\n') {
                    // skip until end of line
                }
                if (ch == '\n') {
                    currentLine++;
                }
                continue;
            } else if (next == '*') {
                int prev = 0;
                while ((ch = fgetc(sourceFile)) != EOF) {
                    if (ch == '\n') {
                        currentLine++;
                    }
                    if (prev == '*' && ch == '/') {
                        break;
                    }
                    prev = ch;
                }
                continue;
            } else {
                if (next != EOF) {
                    ungetc(next, sourceFile);
                }
            }
        }

        // 1. String literal (with escape sequences separated and flagged)
        if (ch == '"') {
            char fullStr[MAX_TOKEN_SIZE * 2];
            int strLen = 0;
            int escaped = 0;
            int closed = 0;
            int startLine = currentLine;

            while ((ch = fgetc(sourceFile)) != EOF && strLen < (int)(sizeof(fullStr) - 1)) {
                if (ch == '\n') {
                    currentLine++;
                }
                if (ch == '"' && !escaped) {
                    closed = 1;
                    break;
                }
                fullStr[strLen++] = (char)ch;
                if (ch == '\\' && !escaped) {
                    escaped = 1;
                } else {
                    escaped = 0;
                }
            }
            fullStr[strLen] = '\0';

            char segment[MAX_TOKEN_SIZE];
            int segIdx = 0;
            int tokensEnqueued = 0;

            for (int i = 0; i < strLen; i++) {
                if (fullStr[i] == '\\' && i + 1 < strLen && strchr(escapeChars, fullStr[i + 1]) != NULL) {
                    if (segIdx > 0) {
                        Token strTok;
                        memset(&strTok, 0, sizeof(Token));
                        snprintf(strTok.lexeme, MAX_TOKEN_SIZE, "\"%.*s\"", segIdx, segment);
                        strTok.type = CONSTANT;
                        strTok.line = startLine;
                        if (!closed) {
                            snprintf(strTok.warning, MAX_TOKEN_SIZE, "Unclosed string");
                        }
                        enqueueToken(&strTok);
                        tokensEnqueued++;
                        segIdx = 0;
                    }
                    Token escTok;
                    memset(&escTok, 0, sizeof(Token));
                    snprintf(escTok.lexeme, MAX_TOKEN_SIZE, "\\%c", fullStr[i + 1]);
                    escTok.type = SPECIAL_CHARACTER;
                    escTok.line = startLine;
                    enqueueToken(&escTok);
                    tokensEnqueued++;
                    i++; // skip escape character
                } else {
                    if (segIdx < MAX_TOKEN_SIZE - 3) {
                        segment[segIdx++] = fullStr[i];
                    }
                }
            }

            if (segIdx > 0 || tokensEnqueued == 0) {
                Token strTok;
                memset(&strTok, 0, sizeof(Token));
                snprintf(strTok.lexeme, MAX_TOKEN_SIZE, "\"%.*s\"", segIdx, segment);
                strTok.type = CONSTANT;
                strTok.line = startLine;
                if (!closed) {
                    snprintf(strTok.warning, MAX_TOKEN_SIZE, "Unclosed string");
                }
                enqueueToken(&strTok);
            }

            if (dequeueToken(&token)) {
                return token;
            }
        }

        // 2. Character literal
        if (ch == '\'') {
            int idx = 0;
            token.lexeme[idx++] = (char)ch;
            token.line = currentLine;
            int escaped = 0;
            int closed = 0;
            while ((ch = fgetc(sourceFile)) != EOF && idx < MAX_TOKEN_SIZE - 1) {
                if (ch == '\n') currentLine++;
                token.lexeme[idx++] = (char)ch;
                if (ch == '\'' && !escaped) {
                    closed = 1;
                    break;
                }
                if (ch == '\\' && !escaped) {
                    escaped = 1;
                } else {
                    escaped = 0;
                }
            }
            token.lexeme[idx] = '\0';
            token.type = CONSTANT;
            if (!closed) {
                snprintf(token.warning, MAX_TOKEN_SIZE, "Unclosed char literal");
            }
            return token;
        }

        // 3. Special characters
        if (isSpecialCharacter((char)ch)) {
            token.lexeme[0] = (char)ch;
            token.lexeme[1] = '\0';
            token.type = SPECIAL_CHARACTER;
            token.line = currentLine;

            if (ch == '(') {
                parenDepth++;
            } else if (ch == ')') {
                if (parenDepth > 0) parenDepth--;
            } else if (ch == '{') {
                braceDepth++;
            } else if (ch == '}') {
                if (braceDepth > 0) braceDepth--;
            } else if (ch == ';') {
                if (parenDepth > 0) {
                    snprintf(token.warning, MAX_TOKEN_SIZE, "Missing ')' before ';'");
                    parenDepth = 0;
                }
            }
            return token;
        }

        // 4. Numbers (Constants)
        if (isdigit((unsigned char)ch)) {
            int idx = 0;
            token.lexeme[idx++] = (char)ch;
            token.line = currentLine;
            int hasDot = 0;
            while ((ch = fgetc(sourceFile)) != EOF) {
                if (isdigit((unsigned char)ch)) {
                    if (idx < MAX_TOKEN_SIZE - 1) token.lexeme[idx++] = (char)ch;
                } else if (ch == '.' && !hasDot) {
                    hasDot = 1;
                    if (idx < MAX_TOKEN_SIZE - 1) token.lexeme[idx++] = (char)ch;
                } else {
                    ungetc(ch, sourceFile);
                    break;
                }
            }
            token.lexeme[idx] = '\0';
            token.type = CONSTANT;
            return token;
        }

        // 5. Identifiers and Keywords
        if (isalpha((unsigned char)ch) || ch == '_') {
            int idx = 0;
            token.lexeme[idx++] = (char)ch;
            token.line = currentLine;
            while ((ch = fgetc(sourceFile)) != EOF) {
                if (isalnum((unsigned char)ch) || ch == '_') {
                    if (idx < MAX_TOKEN_SIZE - 1) token.lexeme[idx++] = (char)ch;
                } else {
                    ungetc(ch, sourceFile);
                    break;
                }
            }
            token.lexeme[idx] = '\0';
            categorizeToken(&token);
            return token;
        }

        // 6. Operators
        char singleOp[2] = {(char)ch, '\0'};
        if (isOperator(singleOp)) {
            token.line = currentLine;
            int next = fgetc(sourceFile);
            if (next != EOF) {
                char doubleOp[3] = {(char)ch, (char)next, '\0'};
                if (isOperator(doubleOp)) {
                    token.lexeme[0] = (char)ch;
                    token.lexeme[1] = (char)next;
                    token.lexeme[2] = '\0';
                    token.type = OPERATOR;
                    return token;
                }
                ungetc(next, sourceFile);
            }
            token.lexeme[0] = (char)ch;
            token.lexeme[1] = '\0';
            token.type = OPERATOR;
            return token;
        }

        // 7. Unknown character
        token.lexeme[0] = (char)ch;
        token.lexeme[1] = '\0';
        token.type = UNKNOWN;
        token.line = currentLine;
        snprintf(token.warning, MAX_TOKEN_SIZE, "Unrecognized symbol");
        return token;
    }

    // EOF reached: close file and return UNKNOWN
    if (sourceFile != stdin && sourceFile != NULL) {
        fclose(sourceFile);
        sourceFile = NULL;
    }
    token.lexeme[0] = '\0';
    token.type = UNKNOWN;
    token.line = currentLine;
    return token;
}