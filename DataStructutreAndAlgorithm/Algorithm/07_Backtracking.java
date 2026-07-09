/*
================================================================================
BACKTRACKING ALGORITHM - JAVA VERSION
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
- Network configuration optimization
- Service dependency resolution

COMPLEXITY:
- Time: O(N^N) worst case, O(N!) for permutations
- Space: O(N) for recursion stack

KEY INSIGHT:
Backtracking is exhaustive search with intelligent pruning.
================================================================================
*/

import java.util.*;

public class Backtracking {
    
    /*
    N-QUEENS PROBLEM: Place N queens without attacking each other
    Network context: Port/address assignment without conflicts
    */
    static class QueensSolver {
        int[] board;
        int n;
        List<int[]> solutions;
        
        QueensSolver(int n) {
            this.n = n;
            this.board = new int[n];
            this.solutions = new ArrayList<>();
        }
        
        boolean isSafe(int row, int col) {
            // Check columns
            for (int i = 0; i < row; i++) {
                if (board[i] == col) {
                    return false;
                }
            }
            
            // Check diagonals
            for (int i = 0; i < row; i++) {
                if (Math.abs(board[i] - col) == Math.abs(i - row)) {
                    return false;
                }
            }
            
            return true;
        }
        
        void solve(int row) {
            if (row == n) {
                // All queens placed - found a solution
                solutions.add(board.clone());
                return;
            }
            
            // Try placing queen in each column
            for (int col = 0; col < n; col++) {
                if (isSafe(row, col)) {
                    board[row] = col;  // Place queen
                    solve(row + 1);    // Recurse to next row
                    // Implicit backtrack when function returns
                }
            }
        }
    }
    
    static void queensDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: N-QUEENS PROBLEM                                   ║");
        System.out.println("║ (Network context: Non-conflicting port/address assignment)    ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int n = 4;
        System.out.println("\nSolving " + n + "-Queens problem using Backtracking:");
        System.out.println("Rules: No two queens can attack each other");
        System.out.println("       (same row, column, or diagonal)\n");
        
        QueensSolver solver = new QueensSolver(n);
        solver.solve(0);
        
        System.out.println("Total solutions: " + solver.solutions.size());
        
        System.out.println("\nFirst few solutions (Q = Queen, . = Empty):");
        
        int limit = Math.min(3, solver.solutions.size());
        for (int sol = 0; sol < limit; sol++) {
            System.out.println("\nSolution " + (sol + 1) + ":");
            int[] solution = solver.solutions.get(sol);
            
            for (int row = 0; row < n; row++) {
                for (int col = 0; col < n; col++) {
                    if (solution[row] == col) {
                        System.out.print(" Q");
                    } else {
                        System.out.print(" .");
                    }
                }
                System.out.println();
            }
        }
        
        System.out.println("\nNETWORK ANALOGY:");
        System.out.println("- Queens = Services/processes");
        System.out.println("- Board positions = Network nodes");
        System.out.println("- Constraints = No resource conflicts");
        System.out.println("- Solution = Valid configuration");
    }
    
    /*
    SUDOKU SOLVER: Fill 9x9 grid with digits 1-9
    Network context: Configuration constraint satisfaction
    */
    static class SudokuSolver {
        int[][] board;
        int attempts;
        
        SudokuSolver(int[][] initial) {
            this.board = new int[9][9];
            this.attempts = 0;
            
            // Copy initial state
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    this.board[i][j] = initial[i][j];
                }
            }
        }
        
        boolean isValid(int row, int col, int num) {
            // Check row
            for (int j = 0; j < 9; j++) {
                if (board[row][j] == num) return false;
            }
            
            // Check column
            for (int i = 0; i < 9; i++) {
                if (board[i][col] == num) return false;
            }
            
            // Check 3x3 box
            int boxRow = (row / 3) * 3;
            int boxCol = (col / 3) * 3;
            for (int i = boxRow; i < boxRow + 3; i++) {
                for (int j = boxCol; j < boxCol + 3; j++) {
                    if (board[i][j] == num) return false;
                }
            }
            
            return true;
        }
        
        boolean solve() {
            // Find empty cell
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (board[i][j] == 0) {
                        // Try each number 1-9
                        for (int num = 1; num <= 9; num++) {
                            attempts++;
                            
                            if (isValid(i, j, num)) {
                                board[i][j] = num;
                                
                                if (solve()) {
                                    return true;  // Solution found
                                }
                                
                                board[i][j] = 0;  // Backtrack
                            }
                        }
                        return false;  // No valid number found
                    }
                }
            }
            
            return true;  // All cells filled
        }
        
        void print() {
            for (int i = 0; i < 9; i++) {
                if (i % 3 == 0 && i != 0) System.out.println("------+-------+------");
                for (int j = 0; j < 9; j++) {
                    if (j % 3 == 0 && j != 0) System.out.print("| ");
                    System.out.print(board[i][j] + " ");
                }
                System.out.println();
            }
        }
    }
    
    static void sudokuDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: SUDOKU SOLVER                                      ║");
        System.out.println("║ (Network context: Configuration constraint satisfaction)      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[][] board = {
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
        
        System.out.println("\nSolving Sudoku puzzle using Backtracking:");
        System.out.println("Initial state:");
        SudokuSolver solver = new SudokuSolver(board);
        solver.print();
        
        solver.solve();
        
        System.out.println("\nSolved in " + solver.attempts + " attempts:");
        solver.print();
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Cells = Network configuration parameters");
        System.out.println("- Numbers = Valid values for parameters");
        System.out.println("- Constraints = Dependency rules");
        System.out.println("- Backtracking = Configuration search");
    }
    
    /*
    PERMUTATIONS: Generate all permutations of an array
    */
    static class PermutationGenerator {
        int[] arr;
        List<int[]> perms;
        
        PermutationGenerator(int[] arr) {
            this.arr = arr.clone();
            this.perms = new ArrayList<>();
        }
        
        void generate(int start) {
            if (start == arr.length - 1) {
                // Found a permutation
                perms.add(arr.clone());
                return;
            }
            
            for (int i = start; i < arr.length; i++) {
                // Swap
                swap(start, i);
                // Recurse
                generate(start + 1);
                // Backtrack (swap back)
                swap(start, i);
            }
        }
        
        void swap(int i, int j) {
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    
    static void permutationDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: PERMUTATIONS (Backtracking application)            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[] arr = {1, 2, 3, 4};
        PermutationGenerator gen = new PermutationGenerator(arr);
        
        System.out.println("\nGenerating permutations of {1, 2, 3, 4}:");
        gen.generate(0);
        
        System.out.println("Total permutations: " + gen.perms.size() + " (4! = 24)\n");
        
        System.out.println("First 8 permutations:");
        for (int i = 0; i < Math.min(8, gen.perms.size()); i++) {
            System.out.print("{");
            int[] perm = gen.perms.get(i);
            for (int j = 0; j < perm.length; j++) {
                System.out.print(perm[j]);
                if (j < perm.length - 1) System.out.print(", ");
            }
            System.out.println("}");
        }
    }
    
    /*
    SUBSET SUM: Find all subsets that sum to target
    Network context: Bandwidth allocation combinations
    */
    static class SubsetSumSolver {
        List<List<Integer>> solutions;
        
        SubsetSumSolver() {
            this.solutions = new ArrayList<>();
        }
        
        void solve(int[] arr, int index, int target, int sum, List<Integer> current) {
            if (sum == target) {
                // Found a solution
                solutions.add(new ArrayList<>(current));
                return;
            }
            
            if (index >= arr.length || sum > target) {
                return;  // Pruning
            }
            
            // Include arr[index]
            current.add(arr[index]);
            solve(arr, index + 1, target, sum + arr[index], current);
            current.remove(current.size() - 1);  // Backtrack
            
            // Exclude arr[index]
            solve(arr, index + 1, target, sum, current);
        }
    }
    
    static void subsetSumDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 4: SUBSET SUM PROBLEM                                 ║");
        System.out.println("║ (Network context: Bandwidth allocation combinations)          ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[] arr = {10, 20, 15, 5};
        int target = 25;
        
        System.out.print("\nFind all subsets of {");
        for (int i = 0; i < arr.length; i++) {
            System.out.print(arr[i]);
            if (i < arr.length - 1) System.out.print(", ");
        }
        System.out.println("} that sum to " + target + ":\n");
        
        SubsetSumSolver solver = new SubsetSumSolver();
        solver.solve(arr, 0, target, 0, new ArrayList<>());
        
        System.out.println("Total solutions: " + solver.solutions.size() + "\n");
        
        for (int i = 0; i < solver.solutions.size(); i++) {
            System.out.print("Solution " + (i + 1) + ": {");
            List<Integer> sol = solver.solutions.get(i);
            for (int j = 0; j < sol.size(); j++) {
                System.out.print(sol.get(j));
                if (j < sol.size() - 1) System.out.print(", ");
            }
            System.out.println("}");
        }
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Array elements = Available bandwidth chunks");
        System.out.println("- Target sum = Required bandwidth");
        System.out.println("- Solutions = Valid bandwidth combinations");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nN-Queens:");
        System.out.println("  Time: O(N!) worst case, much better with pruning");
        System.out.println("  Space: O(N) for recursion");
        System.out.println("  Pruning: Very effective (90%+ reductions)\n");
        
        System.out.println("Sudoku:");
        System.out.println("  Time: O(9^(empty cells))");
        System.out.println("  Space: O(empty cells) for recursion");
        System.out.println("  Pruning: Very effective with constraint checking\n");
        
        System.out.println("Permutations:");
        System.out.println("  Time: O(N!)");
        System.out.println("  Space: O(N!)");
        System.out.println("  No pruning possible - must generate all\n");
        
        System.out.println("Subset Sum:");
        System.out.println("  Time: O(2^N) worst case");
        System.out.println("  Space: O(N) for recursion");
        System.out.println("  Pruning: Current sum > target");
    }
    
    static void keyConcepts() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY CONCEPTS OF BACKTRACKING                                  ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. PRUNING:");
        System.out.println("   - Avoid exploring invalid branches early");
        System.out.println("   - Example: Sum already > target in subset sum");
        System.out.println("   - Can reduce complexity exponentially\n");
        
        System.out.println("2. CONSTRAINT SATISFACTION:");
        System.out.println("   - Check constraints incrementally");
        System.out.println("   - Fail fast if constraint violated");
        System.out.println("   - Essential for N-Queens\n");
        
        System.out.println("3. CHOICE AND UNDO:");
        System.out.println("   - Make a choice");
        System.out.println("   - Recursively solve");
        System.out.println("   - If fails, undo choice and try next\n");
        
        System.out.println("4. EXHAUSTIVE SEARCH:");
        System.out.println("   - Explores all valid possibilities");
        System.out.println("   - Guarantees finding all solutions");
        System.out.println("   - Expensive but thorough");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ BACKTRACKING ALGORITHM - JAVA COMPREHENSIVE DEMONSTRATION     ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nBacktracking Strategy: Try, fail, undo, retry");
        System.out.println("Best for: Constraint satisfaction, finding all solutions");
        System.out.println("Weakness: Exponential worst-case complexity\n");
        
        queensDemo();
        sudokuDemo();
        permutationDemo();
        subsetSumDemo();
        complexityAnalysis();
        keyConcepts();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Backtracking: Exhaustive search with undo");
        System.out.println("2. Essential for constraint satisfaction");
        System.out.println("3. Pruning is critical for performance");
        System.out.println("4. Finds ALL solutions, not just one");
        System.out.println("5. Used in puzzle solving and exploration");
    }
}
