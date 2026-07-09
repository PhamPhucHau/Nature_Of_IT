/*
================================================================================
BACKTRACKING ALGORITHM
================================================================================

STORY:
------
A person enters a maze.

He walks straight.

He hits a dead end.

He goes back to the last junction.

He tries a different path.

He hits another dead end.

He backs up again, and tries yet another path.

Eventually, he finds the exit.

That's Backtracking: Try, fail, undo, and try something else.

NETWORK APPLICATION:
- Puzzle solving (Sudoku-like configurations)
- Network path finding with constraints
- Constraint satisfaction in routing
- Configuration exploration
- Backtracking search in distributed systems
- N-Queens problem analogy: Port assignment with no-conflict constraints

COMPLEXITY:
- Time: O(N^N) worst case, O(N!) for permutations
- Space: O(N) for recursion stack
- Heavily depends on pruning effectiveness

KEY INSIGHT:
Backtracking is exhaustive search with intelligent pruning.
Find all solutions that satisfy constraints, or fail fast.
Important for constraint satisfaction and exploration problems.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 10
#define MAX_SOLUTIONS 100

// N-Queens Problem
typedef struct {
    int board[MAX_N];      // board[i] = column of queen in row i
    int solution_count;
    int solutions[MAX_SOLUTIONS][MAX_N];  // Store all solutions
} QueensState;

int is_safe(int board[], int row, int col) {
    // Check if queen can be placed at (row, col)
    
    // Check columns
    for (int i = 0; i < row; i++) {
        if (board[i] == col) {
            return 0;
        }
    }
    
    // Check diagonals
    for (int i = 0; i < row; i++) {
        if (abs(board[i] - col) == abs(i - row)) {
            return 0;
        }
    }
    
    return 1;
}

void solve_queens(int board[], int row, int n, QueensState* state) {
    // Base case: all queens placed
    if (row == n) {
        if (state->solution_count < MAX_SOLUTIONS) {
            memcpy(state->solutions[state->solution_count], board, n * sizeof(int));
        }
        state->solution_count++;
        return;
    }
    
    // Try placing queen in each column
    for (int col = 0; col < n; col++) {
        if (is_safe(board, row, col)) {
            board[row] = col;  // Place queen
            solve_queens(board, row + 1, n, state);  // Recurse
            // No need to explicitly remove queen - it will be overwritten
        }
    }
}

/*
N-QUEENS SOLVER: Find all valid queen placements
Network context: Port/address assignment without conflicts
*/
void queens_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: N-QUEENS PROBLEM                                   ║\n");
    printf("║ (Network context: Non-conflicting port/address assignment)    ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int n = 4;
    printf("\nSolving %d-Queens problem using Backtracking:\n", n);
    printf("Rules: No two queens can attack each other\n");
    printf("       (same row, column, or diagonal)\n");
    
    QueensState state;
    state.solution_count = 0;
    int board[MAX_N] = {0};
    
    solve_queens(board, 0, n, &state);
    
    printf("\nTotal solutions: %d\n", state.solution_count);
    
    printf("\nFirst few solutions (column positions for each row):\n");
    int limit = (state.solution_count > 5) ? 5 : state.solution_count;
    
    for (int sol = 0; sol < limit; sol++) {
        printf("\nSolution %d:\n", sol + 1);
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (state.solutions[sol][row] == col) {
                    printf(" Q");
                } else {
                    printf(" .");
                }
            }
            printf("\n");
        }
    }
    
    printf("\nNETWORK ANALOGY:\n");
    printf("- Queens = Services/processes\n");
    printf("- Board positions = Network nodes\n");
    printf("- Constraints = No resource conflicts\n");
    printf("- Solution = Valid configuration\n");
}

// Sudoku Solver
typedef struct {
    int board[9][9];
    int attempts;
} SudokuState;

int is_valid(int board[9][9], int row, int col, int num) {
    // Check row
    for (int j = 0; j < 9; j++) {
        if (board[row][j] == num) return 0;
    }
    
    // Check column
    for (int i = 0; i < 9; i++) {
        if (board[i][col] == num) return 0;
    }
    
    // Check 3x3 box
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;
    for (int i = box_row; i < box_row + 3; i++) {
        for (int j = box_col; j < box_col + 3; j++) {
            if (board[i][j] == num) return 0;
        }
    }
    
    return 1;
}

int solve_sudoku(int board[9][9], SudokuState* state) {
    state->attempts++;
    
    // Find empty cell
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0) {
                // Try each number 1-9
                for (int num = 1; num <= 9; num++) {
                    if (is_valid(board, i, j, num)) {
                        board[i][j] = num;
                        
                        if (solve_sudoku(board, state)) {
                            return 1;  // Solution found
                        }
                        
                        board[i][j] = 0;  // Backtrack
                    }
                }
                return 0;  // No valid number found
            }
        }
    }
    
    return 1;  // All cells filled (solution found)
}

void sudoku_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: SUDOKU SOLVER                                      ║\n");
    printf("║ (Network context: Configuration constraint satisfaction)      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    SudokuState state;
    state.attempts = 0;
    
    // Sudoku puzzle (0 = empty)
    int board[9][9] = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };
    
    printf("\nSolving Sudoku puzzle using Backtracking:\n");
    printf("Initial state:\n");
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0 && i != 0) printf("------+-------+------\n");
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j != 0) printf("| ");
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
    
    solve_sudoku(board, &state);
    
    printf("\nSolved in %d attempts:\n", state.attempts);
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0 && i != 0) printf("------+-------+------\n");
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j != 0) printf("| ");
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
    
    printf("\nNETWORK CONTEXT:\n");
    printf("- Cells = Network configuration parameters\n");
    printf("- Numbers = Valid values for parameters\n");
    printf("- Constraints = Dependency rules\n");
    printf("- Backtracking = Configuration search\n");
}

// Permutations
typedef struct {
    int perms[100][MAX_N];
    int count;
} Permutations;

void generate_permutations(int arr[], int n, int start, Permutations* perms) {
    if (start == n - 1) {
        if (perms->count < 100) {
            memcpy(perms->perms[perms->count], arr, n * sizeof(int));
        }
        perms->count++;
        return;
    }
    
    for (int i = start; i < n; i++) {
        // Swap
        int temp = arr[start];
        arr[start] = arr[i];
        arr[i] = temp;
        
        // Recurse
        generate_permutations(arr, n, start + 1, perms);
        
        // Backtrack (swap back)
        temp = arr[start];
        arr[start] = arr[i];
        arr[i] = temp;
    }
}

void permutation_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 3: PERMUTATIONS (Backtracking application)            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int n = 4;
    int arr[] = {1, 2, 3, 4};
    Permutations perms;
    perms.count = 0;
    
    printf("\nGenerating permutations of {1, 2, 3, 4}:\n");
    
    generate_permutations(arr, n, 0, &perms);
    
    printf("Total permutations: %d (%d! = 24)\n", perms.count, n);
    
    printf("\nFirst 8 permutations:\n");
    for (int i = 0; i < 8 && i < perms.count; i++) {
        printf("{");
        for (int j = 0; j < n; j++) {
            printf("%d", perms.perms[i][j]);
            if (j < n - 1) printf(", ");
        }
        printf("}\n");
    }
}

// Subset Sum
typedef struct {
    int solutions[100][10];
    int solution_sizes[100];
    int count;
} SubsetSolutions;

void find_subsets(int arr[], int n, int index, int current[], int size, 
                  int target, int current_sum, SubsetSolutions* sols) {
    if (current_sum == target) {
        if (sols->count < 100) {
            memcpy(sols->solutions[sols->count], current, size * sizeof(int));
            sols->solution_sizes[sols->count] = size;
        }
        sols->count++;
        return;
    }
    
    if (index >= n || current_sum > target) {
        return;  // Pruning
    }
    
    // Include arr[index]
    current[size] = arr[index];
    find_subsets(arr, n, index + 1, current, size + 1, target, 
                current_sum + arr[index], sols);
    
    // Exclude arr[index]
    find_subsets(arr, n, index + 1, current, size, target, current_sum, sols);
}

void subset_sum_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 4: SUBSET SUM PROBLEM                                 ║\n");
    printf("║ (Network context: Bandwidth allocation combinations)          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int arr[] = {10, 20, 15, 5};
    int n = 4;
    int target = 25;
    
    printf("\nFind all subsets of {");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("} that sum to %d:\n\n", target);
    
    SubsetSolutions sols;
    sols.count = 0;
    int current[10];
    
    find_subsets(arr, n, 0, current, 0, target, 0, &sols);
    
    printf("Total solutions: %d\n\n", sols.count);
    
    for (int i = 0; i < sols.count; i++) {
        printf("Solution %d: {", i + 1);
        for (int j = 0; j < sols.solution_sizes[i]; j++) {
            printf("%d", sols.solutions[i][j]);
            if (j < sols.solution_sizes[i] - 1) printf(", ");
        }
        printf("}\n");
    }
    
    printf("\nNETWORK CONTEXT:\n");
    printf("- Array elements = Available bandwidth chunks\n");
    printf("- Target sum = Required bandwidth\n");
    printf("- Solutions = Valid bandwidth combinations\n");
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nN-Queens:\n");
    printf("  Time: O(N!) worst case, much better with pruning\n");
    printf("  Space: O(N) for recursion\n");
    printf("  Pruning: Very effective (90%+ reductions)\n\n");
    
    printf("Sudoku:\n");
    printf("  Time: O(9^(empty cells))\n");
    printf("  Space: O(empty cells) for recursion\n");
    printf("  Pruning: Very effective with constraint checking\n\n");
    
    printf("Permutations:\n");
    printf("  Time: O(N!)\n");
    printf("  Space: O(N!)\n");
    printf("  No pruning possible - must generate all\n\n");
    
    printf("Subset Sum:\n");
    printf("  Time: O(2^N) worst case\n");
    printf("  Space: O(N) for recursion\n");
    printf("  Pruning: Current sum > target\n");
}

void key_concepts() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ KEY CONCEPTS OF BACKTRACKING                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. PRUNING:\n");
    printf("   - Avoid exploring invalid branches early\n");
    printf("   - Example: Sum already > target in subset sum\n");
    printf("   - Can reduce complexity exponentially\n\n");
    
    printf("2. CONSTRAINT SATISFACTION:\n");
    printf("   - Check constraints incrementally\n");
    printf("   - Fail fast if constraint violated\n");
    printf("   - Essential for N-Queens\n\n");
    
    printf("3. CHOICE AND UNDO:\n");
    printf("   - Make a choice\n");
    printf("   - Recursively solve\n");
    printf("   - If fails, undo choice and try next\n\n");
    
    printf("4. EXHAUSTIVE SEARCH:\n");
    printf("   - Explores all valid possibilities\n");
    printf("   - Guarantees finding all solutions\n");
    printf("   - Expensive but thorough\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ BACKTRACKING ALGORITHM - COMPREHENSIVE DEMONSTRATION          ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBacktracking Strategy: Try, fail, undo, retry\n");
    printf("Best for: Constraint satisfaction, finding all solutions\n");
    printf("Weakness: Exponential worst-case complexity\n\n");
    
    queens_demo();
    sudoku_demo();
    permutation_demo();
    subset_sum_demo();
    complexity_analysis();
    key_concepts();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Backtracking: Exhaustive search with undo\n");
    printf("2. Essential for constraint satisfaction\n");
    printf("3. Pruning is critical for performance\n");
    printf("4. Finds ALL solutions, not just one\n");
    printf("5. Used in puzzle solving and exploration\n");
    
    return 0;
}
