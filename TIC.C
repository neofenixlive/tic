#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MB_ERROR(s) printf("ERROR! %s\n", (s)); exit(1)

enum MB_INSTRUCTIONS {
    LET, IF, END, WHILE, LOOP, GOTO, POS, PRINT, SCAN, EXIT, REM,
    ADD, SUB, MUL, DIV, EQUAL, MORE, LESS, AND, OR, NOT
};

struct MB_Tok {
    int Instruction;
    int Int0[2];
    int Int1[2];
};

struct MB_Env {
    struct MB_Tok** TokV;
    int TokC;
    int TokIndex;    
    int Var[26][10];
};

void* MB_Lex(char* S) {
    struct MB_Tok* T = malloc(sizeof(struct MB_Tok));
    
    int Idx = 0;
    int Len = strlen(S);
    int SkipIdx = 0;
    int SetValue = 0;
    
    S[Len--] = '\0';
    T->Instruction = -1;
    T->Int0[0] = 0;
    T->Int0[1] = -1;
    T->Int1[0] = 0;
    T->Int1[1] = -1;
    
    Idx = 0;
    while(S[Idx] == ' ' || S[Idx] == '\t') {
        Idx++;
    }
    memmove(S, S+Idx, (Len-Idx) * sizeof(char));
    
    if(strncmp(S, "LET", 3) == 0) { T->Instruction = LET; SkipIdx = 3; }
     else if(strncmp(S, "IF", 2) == 0) { T->Instruction = IF; }
    else if(strncmp(S, "END", 3) == 0) { T->Instruction = END; }
    else if(strncmp(S, "WHILE", 5) == 0) { T->Instruction = WHILE; }
    else if(strncmp(S, "LOOP", 4) == 0) { T->Instruction = LOOP; }
    else if(strncmp(S, "GOTO", 4) == 0) { T->Instruction = GOTO; SkipIdx = 4; }
    else if(strncmp(S, "POS", 4) == 0) { T->Instruction = POS; SkipIdx = 4; }
    else if(strncmp(S, "PRINT", 5) == 0) { T->Instruction = PRINT; SkipIdx = 5; }
    else if(strncmp(S, "SCAN", 5) == 0) { T->Instruction = SCAN; SkipIdx = 5; }
    else if(strncmp(S, "EXIT", 4) == 0) { T->Instruction = EXIT; SkipIdx = 4; }
    else if(strncmp(S, "REM", 3) == 0) { T->Instruction = REM; }
    else if(strncmp(S, "ADD", 3) == 0) { T->Instruction = ADD; SkipIdx = 3; }
    else if(strncmp(S, "SUB", 3) == 0) { T->Instruction = SUB; SkipIdx = 3; }
    else if(strncmp(S, "MUL", 3) == 0) { T->Instruction = MUL; SkipIdx = 3; }
    else if(strncmp(S, "DIV", 3) == 0) { T->Instruction = DIV; SkipIdx = 3; }
    else if(strncmp(S, "EQUAL", 5) == 0) { T->Instruction = EQUAL; SkipIdx = 5; }
    else if(strncmp(S, "MORE", 5) == 0) { T->Instruction = MORE; SkipIdx = 5; }
    else if(strncmp(S, "LESS", 4) == 0) { T->Instruction = LESS; SkipIdx = 4; }
    else if(strncmp(S, "AND", 3) == 0) { T->Instruction = AND; }
    else if(strncmp(S, "OR", 2) == 0) { T->Instruction = OR; }
    else if(strncmp(S, "NOT", 3) == 0) { T->Instruction = NOT; }
    else { MB_ERROR("Instruction not found or mispelled"); }
    
    if(SkipIdx == 0) { return T; }
    
    for(Idx = SkipIdx; Idx < Len; Idx++) {
        if(S[Idx] >= 'A' && S[Idx] <= 'Z' && S[Idx+1] >= '0' && S[Idx+1] <= '9') {
            if(SetValue == 1) {
                T->Int0[0] = S[Idx]-65;
                T->Int0[1] = S[Idx+1]-48;
            } else {
                T->Int1[0] = S[Idx]-65;
                T->Int1[1] = S[Idx+1]-48;
            }
            Idx++;
        } else if(S[Idx] >= '0' && S[Idx] <= '9') {
            if(SetValue == 1) {
                T->Int0[0] *= 10;
                T->Int0[0] += S[Idx]-48; 
            } else {
                T->Int1[0] *= 10;
                T->Int1[0] += S[Idx]-48;
            }
        } else if(S[Idx] == ' ') {
            SetValue++;
            if(SetValue == 3) { MB_ERROR("Too many values assigned for instruction"); }
        }
    }
    
    return T;
}

int MB_Eval(struct MB_Env* E, int Idx) {
    int Int0[2];
    int Int1[2];
    int Var0;
    int Var1;
    
    if(Idx < 0 || Idx > E->TokC) {
        return -1;
    }
    
    Int0[0] = E->TokV[Idx]->Int0[0];
    Int0[1] = E->TokV[Idx]->Int0[1];
    Int1[0] = E->TokV[Idx]->Int1[0];
    Int1[1] = E->TokV[Idx]->Int1[1];
    if(Int0[1] == -1) { Var0 = Int0[0]; }
    else { Var0 = E->Var[Int0[0]][Int0[1]]; }
    if(Int1[1] == -1) { Var1 = Int1[0]; }
    else { Var1 = E->Var[Int1[0]][Int1[1]]; }
    
    if( E->TokV[Idx]->Instruction == LET ||
        E->TokV[Idx]->Instruction == ADD ||
        E->TokV[Idx]->Instruction == SUB ||
        E->TokV[Idx]->Instruction == MUL ||
        E->TokV[Idx]->Instruction == DIV) {
        if(Int0[1] == -1) {
            MB_ERROR("Value assignment to a number literal or unknown"); 
        }
    }
    
    switch(E->TokV[Idx]->Instruction) {
        default: break;
        case LET: E->Var[Int0[0]][Int0[1]] = Var1; break;
        case IF: {
            int Condition = E->TokV[Idx-2]->Instruction;
            if(Condition == AND || Condition == OR || Condition == NOT) { Condition = MB_Eval(E, Idx-2); }
            else { Condition = MB_Eval(E, Idx-1); }
            
            if(Condition == -1) { MB_ERROR("No condition for IF instruction"); }
            if(!Condition) { E->TokIndex = Var1; }
            break;
        }
        case WHILE: {
            int Condition = E->TokV[Idx-2]->Instruction;
            if(Condition == AND || Condition == OR || Condition == NOT) { Condition = MB_Eval(E, Idx-2); }
            else { Condition = MB_Eval(E, Idx-1); }
            
            if(Condition == -1) { MB_ERROR("No condition for WHILE instruction"); }
            if(!Condition) { E->TokIndex = Var1; }
            break;
        }
        case LOOP: E->TokIndex = Var1-1; break;
        case GOTO: E->TokIndex = Var1-1; break;
        case EXIT: printf("Program stopped with code %d", Var0); exit(0);
        case PRINT: printf("OUT: %d\n", Var0); break;
        case SCAN:
            printf("IN: ");
            if(scanf("%d", &E->Var[Int0[0]][Int0[1]]) != 1) {
                MB_ERROR("SCAN was not a number");
            }
            break;
        case ADD: E->Var[Int0[0]][Int0[1]] = Var0 + Var1; break;
        case SUB: E->Var[Int0[0]][Int0[1]] = Var0 - Var1; break;
        case MUL: E->Var[Int0[0]][Int0[1]] = Var0 * Var1; break;
        case DIV: E->Var[Int0[0]][Int0[1]] = Var0 / Var1; break;
        case EQUAL: return Var0 == Var1;
        case MORE: return Var0 > Var1;
        case LESS: return Var0 < Var1;
        case AND: return MB_Eval(E, Idx-1) && MB_Eval(E, Idx+1);
        case OR: return MB_Eval(E, Idx-1) || MB_Eval(E, Idx+1);
        case NOT: return !MB_Eval(E, Idx+1);
    
    }
    return -1;
}

void* MB_New(char* S) {
    struct MB_Env* E = malloc(sizeof(struct MB_Env));
    
    FILE* Script = fopen(S, "r");
    char* Line = malloc(sizeof(char)*128);
    int Idx;
    int SubIdx;
    
    E->TokV = NULL;
    E->TokC = 0;
    E->TokIndex = 0;
    
    for(Idx = 0; Idx < 26; Idx++) {
        for(SubIdx = 0; SubIdx < 10; SubIdx++) {
            E->Var[Idx][SubIdx] = 0;
        }
    }
    
    while(fgets(Line, 128, Script)) {
        if(strlen(Line) >= 3) {
            struct MB_Tok* T = MB_Lex(Line);
            E->TokV = realloc(E->TokV, sizeof(void*)*(E->TokC+1));
            E->TokV[E->TokC] = T;
            E->TokC++;
        }
    }
    
    for(Idx = 0; Idx < E->TokC; Idx++) {
        if(E->TokV[Idx]->Instruction == IF) {
            SubIdx = Idx;
            while(SubIdx < E->TokC) {
                if(E->TokV[SubIdx]->Instruction == END) {
                    E->TokV[Idx]->Int1[0] = SubIdx;
                    break;
                }
                SubIdx++;
            }
            if(E->TokV[Idx]->Int1[0] == 0) { MB_ERROR("No END for IF instruction"); }
        }
        if(E->TokV[Idx]->Instruction == WHILE) {
            SubIdx = Idx;
            while(SubIdx < E->TokC) {
                if(E->TokV[SubIdx]->Instruction == LOOP) {
                    E->TokV[Idx]->Int1[0] = SubIdx;
                    E->TokV[SubIdx]->Int1[0] = Idx;
                    break;
                }
                SubIdx++;
            }
            if(E->TokV[Idx]->Int1[0] == 0) { MB_ERROR("No LOOP for WHILE instruction"); }
        }
        if(E->TokV[Idx]->Instruction == GOTO) {
            SubIdx = 0;
            while(SubIdx < E->TokC) {
                if(E->TokV[SubIdx]->Instruction == POS && E->TokV[Idx]->Int0[0] == E->TokV[SubIdx]->Int0[0]) {
                    E->TokV[Idx]->Int1[0] = SubIdx;
                    break;
                }
                SubIdx++;
            }
            if(E->TokV[Idx]->Int1[0] == 0) { MB_ERROR("No POS for GOTO instruction"); }
        }
    }
    
    free(Line);
    fclose(Script);
    return E;
}

void MB_Run(struct MB_Env* E) {
    for(E->TokIndex = 0; E->TokIndex < E->TokC; E->TokIndex++) {
        MB_Eval(E, E->TokIndex);
    }
}

int main(int ArgC, char* ArgV[]) {
    if(ArgV[1]) {
        struct MB_Env* E = MB_New(ArgV[1]);
        
        printf("Processing %s\n", ArgV[1]);
        MB_Run(E);
        printf("Finished %s\n", ArgV[1]);
    } else {
        MB_ERROR("No file inserted");
    }    
    
    return 0;
}
