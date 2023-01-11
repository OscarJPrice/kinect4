#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define BOARD_SIZE_Y 5
#define BOARD_SIZE_X 7
#define IN_BETWEEN_INCLUSIVE(a, b, c) (a <= b && b <= c)
#define IN_BETWEEN_NON_INCLUSIVE(a, b, c) (a <= b && b < c)

typedef enum {
    false, 
    true
} bool;

typedef enum {
    none,
    red,
    yellow
} side;

typedef side board[BOARD_SIZE_Y][BOARD_SIZE_X];

char* to_string(side side) {
    assert(IN_BETWEEN_INCLUSIVE(none, side, yellow));

    switch (side) {
        case red:
            return "\x1b[31;1mO\x1b[0m ";
        case yellow:
            return "\x1b[33;1mO\x1b[0m ";
        case none:
            return "  ";
    }
}


void initialize_board(board board) {
    for (int y = 0; y < BOARD_SIZE_Y; y++) {
        for (int x = 0; x < BOARD_SIZE_X; x++) {
            board[y][x] = none;
        }
    }
}

void output_board(side board[BOARD_SIZE_Y][BOARD_SIZE_X]) {
    for (int y = BOARD_SIZE_Y-1; y >= 0; y--) {
        for (int x = 0; x < BOARD_SIZE_X; x++) {
            printf("|%s", to_string(board[y][x]));
        }
        printf("|\n");
    }
    for (int x = 0; x < BOARD_SIZE_X; x++) {
        printf(" %i ", x+1);
    }
    printf(" \n");

}


typedef enum {
    ok,
    index_out_of_bounds, 
    illegal_overwrite
} result;

result place_by_col(board board, side side, int col) {

    if(!IN_BETWEEN_INCLUSIVE(1, col, BOARD_SIZE_X)) {
        return index_out_of_bounds;
    }

    for (int i = 0; i < BOARD_SIZE_Y; i++) {
        if (board[i][col-1] == none) {
            board[i][col-1] = side;
            return ok;
        }
    }
    return illegal_overwrite;
}

void clear() {
    printf("\x1B[2J\x1B[1;1H");
}

typedef struct {
    int x;
    int y;
} i_v2;

side poll_arm(board board, i_v2 origin, i_v2 direction) {
    
    i_v2 cursor = origin;

    for (int i = 0; i < 4; i++) {
        if (board[cursor.y][cursor.x] != board[origin.y][origin.x]) {
            return none;
        }
        cursor.x+=direction.x;
        cursor.y+=direction.y;
        if(!IN_BETWEEN_NON_INCLUSIVE(0, cursor.x, BOARD_SIZE_X) ||
            !IN_BETWEEN_NON_INCLUSIVE(0, cursor.y, BOARD_SIZE_Y)) {
            return none;
        }
    }
    return board[origin.y][origin.x];
}

side poll_arms(board board, i_v2 origin) {
    return 
    poll_arm(board, origin, (i_v2){.x = 1, .y = 0} ) |
    poll_arm(board, origin, (i_v2){.x = 1, .y = 1} ) |
    poll_arm(board, origin, (i_v2){.x = 0, .y = 1} ) |
    poll_arm(board, origin, (i_v2){.x = -1, .y = 1} ) |
    poll_arm(board, origin, (i_v2){.x = -1, .y = 0} ) |
    poll_arm(board, origin, (i_v2){.x = -1, .y = -1} ) |
    poll_arm(board, origin, (i_v2){.x = 0, .y = -1} ) |
    poll_arm(board, origin, (i_v2){.x = 1, .y = -1} );
}

side eval(board board) {
    i_v2 tried[BOARD_SIZE_X*BOARD_SIZE_Y];
    for (int y = 0; y < BOARD_SIZE_Y; y++) {
        for (int x = 0; x < BOARD_SIZE_X; x++) {

            if (board[y][x] != none) {
                side verdict = poll_arms(board, (i_v2){.x = x, .y = y});
                if (verdict != none) {
                    return verdict;
                }
            }
        }
    }
    return none;
}

bool is_scal_int(char str[30]) {
    for (int c = 0; c < 30 && str[c] != '\0'; c++) {
        if (!isdigit(str[c]) && str[c] != '\n') {
            return false;
        }
    }
    return true;
}

int get_safe_number() {
    char buffer[30];
    fgets(buffer, 29, stdin);

    if (!is_scal_int(buffer)) {
        return -1;
    }
    return atoi(buffer);
}


int main() {
    board board;
    initialize_board(board);
    int turn = 1;
    clear();
    output_board(board);

    while(true) {
        printf("Your turn %s : ", turn ? "yellow" : "red");

        int buffer = get_safe_number();
        if (buffer == 99) return 0;

        if (buffer == -1) {
            clear();
            printf("Enter a number you monglet. \n");
            output_board(board);
            continue;
        };

        clear();
        switch(place_by_col(board, turn ? yellow : red, buffer) ){
            case ok:                
                turn ^= 1;
                side verdict = eval(board);
                if (verdict != none) {
                    printf("%s wins!", verdict == yellow ? "Yellow" : "Red");
                    return 0;
                }
                break;
            case illegal_overwrite:
                printf("Column %i is full!\n", buffer);
                break;
            case index_out_of_bounds:
                printf("Column %i does not exist!\n", buffer);
                break;
        };
        output_board(board);
        
    }

    output_board(board);
    return 0;
}