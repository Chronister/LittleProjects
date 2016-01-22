#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define internal static
#define inline

typedef struct 
{
    char* At;
} tokenizer;

typedef enum 
{
    Token_Unknown,

    Token_OpenBrace,
    Token_CloseBrace,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenParen,
    Token_CloseParen,

    Token_Colon,
    Token_Semicolon,
    Token_Comma,
    Token_Asterisk,
    Token_Equals,
    Token_Pound,

    Token_Identifier,
    Token_String,
    Token_Char,
    Token_Number,

    Token_EOF,
} token_type;

typedef struct
{
    token_type Type;

    size_t TextLength;
    char* Text;
} token;

internal bool 
IsNumeric(char C)
{
    bool Result = ('0' <= C && C <= '9') ||
                    C == '+' || C == '-';
    return Result;
}

internal bool
IsConceivablyPartOfANumber(char C)
{
    bool Result = ('0' <= C && C <= '9') || 
                  ('a' <= C && C <= 'f') || 
                  ('A' <= C && C <= 'F') ||
                  C == '+' || C == '-' || C == '.' || C == 'x'; 
    return Result;
}

internal bool 
IsIdentifierChar(char C)
{
    bool Result = ('a' <= C && C <= 'z') ||
                  ('A' <= C && C <= 'Z') ||
                  ('0' <= C && C <= '9') ||
                  C == '_';
    return Result;
}

internal bool
IsWhitespace(char C)
{
    bool Result = C == ' ' || C == '\t' ||
                  C == '\n' || C == '\r' ||
                  C == 'f';
    return Result;
}

internal bool
TokenEquals(token Test, char* Expected)
{
    for (int TextIndex = 0;
        TextIndex < Test.TextLength && *Expected;
        ++TextIndex, ++Expected)
    {
        if (Test.Text[TextIndex] != Expected[TextIndex]) { return false; }
    }

    if (Expected != '\0') { return false; }
    return true;
}

internal void
EatAllWhitespace(tokenizer* Tokenizer)
{
    for(;;)
    {
        switch(Tokenizer->At[0])
        {
            case '\0':
            {
                return;
            } break;
            
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\f':
            {
                ++Tokenizer->At;
            } break;

            case '/':
            {
                switch(Tokenizer->At[1])
                {
                    case '/':
                    {
                        while (Tokenizer->At[0] && 
                               Tokenizer->At[0] != '\r' &&
                               Tokenizer->At[0] != '\n')
                        {
                            ++Tokenizer->At;
                        }

                    } break;

                    case '*':
                    {
                        while(Tokenizer->At[0] &&
                             !(Tokenizer->At[0] == '*' && Tokenizer->At[1] == '/'))
                        {
                            ++Tokenizer->At;
                        }
                        ++Tokenizer->At;
                    } break;

                    default:
                    {
                        return;
                    } break;
                }
            } break;

            case '#':
            {
                // Preprocessor command. We want to ignore it.
                while (Tokenizer->At[1] && 
                       !(Tokenizer->At[0] != '\\' && Tokenizer->At[1] == '\r') &&
                       !(Tokenizer->At[0] != '\\' && Tokenizer->At[1] == '\n'))
                {
                    ++Tokenizer->At;
                }
                ++Tokenizer->At;
            } break;

            default:
            {
                return;
            } break;
        }
    }
}

internal token
GetToken(tokenizer* Tokenizer)
{
    EatAllWhitespace(Tokenizer);

    token Result;
    switch (Tokenizer->At[0])
    {
        case '\0': Result.Type = Token_EOF; break;
        case '{': Result.Type = Token_OpenBrace; ++Tokenizer->At; break;
        case '}': Result.Type = Token_CloseBrace; ++Tokenizer->At; break;
        case '[': Result.Type = Token_OpenBracket; ++Tokenizer->At; break;
        case ']': Result.Type = Token_CloseBracket; ++Tokenizer->At; break;
        case '(': Result.Type = Token_OpenParen; ++Tokenizer->At; break;
        case ')': Result.Type = Token_CloseParen; ++Tokenizer->At; break;
        case ';': Result.Type = Token_Semicolon; ++Tokenizer->At; break;
        case ':': Result.Type = Token_Colon; ++Tokenizer->At; break;
        case ',': Result.Type = Token_Comma; ++Tokenizer->At; break;
        case '*': Result.Type = Token_Asterisk; ++Tokenizer->At; break;
        case '=': Result.Type = Token_Equals; ++Tokenizer->At; break;

        case '\'':
        {
            Result.Type = Token_Char;
            Tokenizer->At += 2;
        } break;

        case '"':
        {
            Result.Type = Token_String;
            Result.Text = Tokenizer->At + 1;

            // Push the cursor along while:
            //  - The string has not ended
            //  - We have not encountered one of the following:
            //    - __" where _ is any character other than \
            //    - \\"
            while(Tokenizer->At[2] &&
                  !(!(Tokenizer->At[0] != '\\' && Tokenizer->At[1] == '\\') && Tokenizer->At[2] == '"'))
            {
                ++Tokenizer->At;
            }

            Tokenizer->At += 2; // We consider 2 characters ahead in the above condition
            Result.TextLength = Tokenizer->At - Result.Text;
            ++Tokenizer->At; // For the quotation mark

        } break;

        default:
        {
            if (IsNumeric(Tokenizer->At[0]))
            {
                while (IsConceivablyPartOfANumber(Tokenizer->At[0])) ++Tokenizer->At;
                Result.Type = Token_Number;
            }
            else if (IsIdentifierChar(Tokenizer->At[0]))
            {
                Result.Type = Token_Identifier;
                Result.Text = Tokenizer->At;
                while (IsIdentifierChar(Tokenizer->At[0])) ++Tokenizer->At;

                Result.TextLength = Tokenizer->At - Result.Text;
            }
            else
            {
                Result.Type = Token_Unknown;
                ++Tokenizer->At;
            }
        } break;
    }

    return Result;
}

internal bool
RequireToken(tokenizer* Tokenizer, token_type Type)
{
    token Found = GetToken(Tokenizer);
    bool Result = (Found.Type == Type);
    return Result;
}


internal char* 
ReadFileIntoCString(char* Filename)
{
    FILE* File = fopen(Filename, "r");
    if (File != NULL) 
    {
        fseek(File, 0, SEEK_END);
        size_t Size = ftell(File);
        rewind(File);

        char* Memory = (char*)malloc(Size+1);
        size_t Result = fread((void*)Memory, 1, Size, File);
        Memory[Size] = '\0';
        if (Result == Size
            || feof(File)) 
        { 
            return Memory; 
        }
        else 
        {
            free(Memory);
            return NULL; 
        }
    }
    return NULL;
}

inline char
ToUppercase(char C)
{
    if ('a' <= C && C <= 'z') 
    {
        C = (C - 'a') + 'A';
    }
    return C;
}

internal char*
HeaderifyFilepath(char* Filepath)
{
    char* Str = Filepath;
    char Buf[_MAX_PATH];
    int LastSlash = -1;
    int Index;
    for (Index = 0;
        Str[Index] != '\0' && Index < _MAX_PATH;
        ++Index)
    {
        Buf[Index] = ToUppercase(Str[Index]);
        if (Str[Index] == '.' || Str[Index] == ' ') { Buf[Index] = '_'; }
        if (Str[Index] == '.' && Str[Index + 1] == 'c') 
        { 
            Buf[Index + 1] = 'H'; 
            ++Index;
        }
        if (Str[Index] == '/' || Str[Index] == '\\') { LastSlash = Index; }
    }
    int ResultLen = Index - LastSlash;
    char* Result = (char*)malloc(ResultLen);
    int ResultIndex;
    for (ResultIndex = 0;
        LastSlash + 1 + ResultIndex < Index;
        ++ResultIndex)
    {
        Result[ResultIndex] = Buf[LastSlash + 1 + ResultIndex];
    }
    Result[ResultIndex] = '\0';
    return Result;
}

int main(int ArgCount, char* ArgValues[])
{
    if (ArgCount <= 1) {
        fprintf(stderr, "Please provide at least one file to parse.\n");
        return EXIT_FAILURE;
    }

    for (int ArgIndex = 1;
        ArgIndex < ArgCount;
        ++ArgIndex)
    {
        char* Filename = ArgValues[ArgIndex];
        char* FileContents = ReadFileIntoCString(Filename);

        if (FileContents == NULL) {
            fprintf(stderr, "Unable to open %s for parsing\n", Filename);
            continue;
        }

        char* HeaderName = HeaderifyFilepath(Filename);
        fprintf(stdout, "#ifndef %s\n", HeaderName);

        tokenizer Tokenizer;
        Tokenizer.At = FileContents;

        token TokenHistory[3];
        memset(TokenHistory, sizeof(TokenHistory), 0);

        token LastIdentifier[2];

        bool Parsing = true;
        int Scope = 0;
        while (Parsing)
        {
            token Next = GetToken(&Tokenizer);
            switch (Next.Type)
            {
                case Token_EOF: Parsing = false; break;

                case Token_OpenParen:
                {
                    // Either a function definition or a function invocation.
                    // The easiest difference to see -- scope.
                    if (Scope == 0)
                    {
                        if (TokenHistory[0].Type == Token_Identifier &&
                            (TokenHistory[1].Type == Token_Identifier ||
                             TokenHistory[1].Type == Token_Asterisk))
                        {
                            char* Start = Tokenizer.At;
                            // *probably* a function definition. Continue based on that assumption.
                            token Name = TokenHistory[0];
                            token Type = TokenHistory[1];
                            int Indirect = 0;
                            if (Type.Type == Token_Asterisk)
                            {
                                // Go back to what should be the type identifier and parse forward from there
                                Tokenizer.At = LastIdentifier[1].Text + LastIdentifier[1].TextLength;
                                token Between;
                                Between.Type = Token_Unknown;
                                // Figure out how indirect it is by counting the asterisks!
                                while ((Between = GetToken(&Tokenizer)).Type == Token_Asterisk) ++Indirect;
                                if (!(Between.Type == Token_Identifier && Between.Text == Next.Text)) 
                                {
                                    goto default_parse;
                                }
                            }

                            fprintf(stdout, "\n");
                            fprintf(stdout, "%.*s", Type.TextLength, Type.Text);
                            while (Indirect--) { fprintf(stdout, "*"); }
                            fprintf(stdout, " %.*s(", Name.TextLength, Name.Text);

                            token Ahead;
                            while((Ahead = GetToken(&Tokenizer)).Type != Token_CloseParen);

                            fprintf(stdout, "%.*s", Tokenizer.At - Start - 1, Start);

                            fprintf(stdout, ");\n");
                        }
                    }

                    goto default_parse;
                } break;

                case Token_Identifier:
                {
                    LastIdentifier[1] = LastIdentifier[0];
                    LastIdentifier[0] = Next;
                    goto default_parse;
                } break;

                case Token_OpenBrace:
                {
                    ++Scope;
                    goto default_parse;
                } break;

                case Token_CloseBrace:
                {
                    --Scope;
                    goto default_parse;
                } break;

            default_parse:
                default:
                {
                    TokenHistory[2] = TokenHistory[1];
                    TokenHistory[1] = TokenHistory[0];
                    TokenHistory[0] = Next;
                } break;
            }
        }

        fprintf(stdout, "\n#define %s\n#endif\n", HeaderName);
    }

    return EXIT_SUCCESS;
}
