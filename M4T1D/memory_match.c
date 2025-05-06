#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>     // sleep()

#define BOARD_SIZE   4
#define NUM_CELLS    (BOARD_SIZE * BOARD_SIZE)
#define NUM_PAIRS    (NUM_CELLS / 2)

/* Shuffle pairs 1…NUM_PAIRS into board[] */
void initialize_board(int board[NUM_CELLS]) {
    int pairs[NUM_CELLS];
    for (int i = 0; i < NUM_PAIRS; i++) {
        pairs[2*i]     = i + 1;
        pairs[2*i + 1] = i + 1;
    }
    srand(time(NULL));
    for (int i = NUM_CELLS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = pairs[i];
        pairs[i] = pairs[j];
        pairs[j] = tmp;
    }
    memcpy(board, pairs, sizeof(pairs));
}

/* Print board: revealed values or '*' */
void print_board(const int board[NUM_CELLS], const int revealed[NUM_CELLS]) {
    printf("\nCurrent Board:\n");
    for (int i = 0; i < NUM_CELLS; i++) {
        if (revealed[i])      printf("%2d ", board[i]);
        else                   printf(" * ");
        if ((i + 1) % BOARD_SIZE == 0) printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int board[NUM_CELLS];
    int revealed[NUM_CELLS] = {0};
    int total_matches = 0;
    int game_over     = 0;

    if (rank == 0) {
        initialize_board(board);
        printf("Server: %dx%d board initialized with %d pairs.\n",
               BOARD_SIZE, BOARD_SIZE, NUM_PAIRS);
    }

    while (1) {
        // 1) Server pushes current state to players
        if (rank == 0) {
            for (int p = 1; p < size; p++) {
                MPI_Send(board,    NUM_CELLS, MPI_INT, p,  0, MPI_COMM_WORLD);
                MPI_Send(revealed, NUM_CELLS, MPI_INT, p,  1, MPI_COMM_WORLD);
            }
        }
        // 2) Players receive state (or skip if game already over)
        if (rank > 0) {
            MPI_Recv(board,    NUM_CELLS, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(revealed, NUM_CELLS, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // 3) Server checks if we're done
        if (rank == 0 && total_matches == NUM_PAIRS) {
            game_over = 1;
        }

        // 4) Broadcast new game_over flag so everyone can break if needed
        MPI_Bcast(&game_over, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (game_over) break;

        if (rank == 0) {
            // --- Server: collect moves and update ---
            for (int p = 1; p < size; p++) {
                int pos[2];
                MPI_Recv(pos, 2, MPI_INT, p, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                int a = pos[0], b = pos[1];
                if (a >= 0 && a < NUM_CELLS &&
                    b >= 0 && b < NUM_CELLS &&
                    a != b &&
                    !revealed[a] && !revealed[b] &&
                    board[a] == board[b]) {
                    revealed[a] = revealed[b] = 1;
                    total_matches++;
                    printf("Server: Player %d matched cells %d & %d (value=%d).\n",
                           p, a, b, board[a]);
                } else {
                    printf("Server: Player %d missed on %d & %d.\n", p, a, b);
                }
            }

            // 5) Small pause before next round
            sleep(2);
        }
        else {
            // --- Player: pick two cards and send ---
            print_board(board, revealed);

            int a, b;
            do {
                printf("Player %d, enter two hidden positions (0–%d): ",
                       rank, NUM_CELLS - 1);
                fflush(stdout);
                scanf("%d %d", &a, &b);
            } while (
                a < 0 || a >= NUM_CELLS ||
                b < 0 || b >= NUM_CELLS ||
                a == b ||
                revealed[a] || revealed[b]
            );

            int sel[2] = {a, b};
            MPI_Send(sel, 2, MPI_INT, 0, 2, MPI_COMM_WORLD);
        }

        // 6) After collecting/sending moves, server may have found all pairs.
        //    Broadcast again so that players will exit immediately on next loop.
        MPI_Bcast(&game_over, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (game_over) break;
    }

    if (rank == 0) {
        printf("Server: Final board state:\n");
        print_board(board, revealed);
    }

    MPI_Finalize();
    return 0;
}
