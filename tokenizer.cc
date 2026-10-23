#include "tokenizer.h"
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdio>

static void pushWord(std::vector<Token> &tokens, std::string &word, bool &inWord, bool &quoted)
{
    // Classify and push the word built
    if (!inWord)
        return;
    Token token;
    if (!tokens.empty())
    {
        Token &prev = tokens.back();
        bool isArgument = prev.type == TOKEN_REDIRECT ||
                          prev.type == TOKEN_APPEND ||
                          prev.type == TOKEN_INPUT ||
                          prev.type == TOKEN_ERROR ||
                          prev.type == TOKEN_REDIRECT_AND_ERROR ||
                          prev.type == TOKEN_COMMAND ||
                          prev.type == TOKEN_ARGUMENT ||
                          word[0] == '-' ||                      // if the word starts with a dash, it's an argument
                          word.find('/') != std::string::npos || // if path = argument not command
                          word.find('.') != std::string::npos;   // no command has a dot

        if (isArgument)
            token.type = TOKEN_ARGUMENT;
        else
            token.type = TOKEN_COMMAND;
    }
    else
        token.type = TOKEN_COMMAND;

    token.value = word;
    token.quoted = quoted;
    tokens.push_back(token);
    word.clear();
    inWord = false;
    quoted = false;
}

static void pushOperator(std::vector<Token> &tokens, const std::string &op)
{
    // Operators are only created here but never from quoted text
    Token token;
    if (op == "|")
        token.type = TOKEN_PIPE;
    else if (op == ">")
        token.type = TOKEN_REDIRECT;
    else if (op == ">>")
        token.type = TOKEN_APPEND;
    else if (op == "<")
        token.type = TOKEN_INPUT;
    else if (op == "&")
        token.type = TOKEN_BACKGROUND;
    else if (op == "2>")
        token.type = TOKEN_ERROR;
    else if (op == ">>&")
        token.type = TOKEN_REDIRECT_AND_ERROR;
    token.value = op;
    tokens.push_back(token);
}

// writing static before both functions to limit their scope to this file only; nadafa bs

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::string word;
    bool inWord = false;   // a word has started
    bool quoted = false;   // the current word contained quotes
    bool inQuotes = false; // we are inside quotes
    char quoteChar = 0;    // the type of quote we are inside, either ' or "

    for (size_t i = 0; i < input.size(); i++)
    {
        char c = input[i];

        if (inQuotes)
        {
            if (c == quoteChar)
                inQuotes = false; // closing quote reached
            else
                word += c; // add to the word (including spaces and operators)
        }
        else if (c == '"' || c == '\'') // starting a quote
        {
            inQuotes = true;
            quoteChar = c;
            inWord = true; // so "" still produces an empty argument
            quoted = true;
        }
        else if (isspace((unsigned char)c))
        {
            pushWord(tokens, word, inWord, quoted);
        }
        else if (c == '|' || c == '<' || c == '>' || c == '&')
        {
            std::string op;
            if (c == '>' && word == "2" && !quoted) // special case for "2>" operator
            {
                word.clear(); // the "2" belongs to the operator
                inWord = false;
                op = "2>";
            }
            else
            {
                pushWord(tokens, word, inWord, quoted); // push the word before the operator
                op = std::string(1, c);
                if (c == '>' && i + 1 < input.size() && input[i + 1] == '>') // handle ">>" and ">>&" (because they are 1+ chars)

                {
                    op = ">>";
                    i++;
                    if (i + 1 < input.size() && input[i + 1] == '&')
                    {
                        op = ">>&";
                        i++;
                    }
                }
            }
            pushOperator(tokens, op); // one of the previous special ops or a single char operator
        }
        else
        {
            word += c;
            inWord = true;
        }
    }

    if (inQuotes) // if we reach the end of the input and we are still inside quotes
    {
        fprintf(stderr, "Error: unclosed quote.\n");
        return {}; // nothing to run
    }

    pushWord(tokens, word, inWord, quoted); // last word
    tokens.push_back({TOKEN_EOF, ""});      // add an EOF token to indicate the end of the input
    return tokens;
}