/*
 * 01_lambda_closure.c  —  Java Lambda & Closure in C
 *
 * Build:  gcc -Wall -Wextra -o 01_lambda_closure 01_lambda_closure.c
 * Run:    ./01_lambda_closure
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA LAMBDA WORKS:
 *
 *  A Java lambda is compiled to a private static method + an instance
 *  of a synthetic class that implements the functional interface.
 *
 *  Lambda:   x -> x * 2
 *  Compiles to:
 *    private static Integer lambda$0(Integer x) { return x * 2; }
 *    // + an invokedynamic call that creates a Function instance
 *
 *  In C, this is just a FUNCTION POINTER:
 *    int (*double_it)(int) = function_that_doubles;
 *
 *  CLOSURE — capturing variables from outer scope:
 *    Java:   int factor = 3;
 *            Function<Integer,Integer> multiply = x -> x * factor;
 *    C:      A closure is a struct { fn_ptr + captured_env }
 *
 *  Java captures variables by VALUE (effectively final).
 *  The JVM creates a closure object that holds:
 *    - pointer to the lambda method
 *    - copies of all captured variables
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  PART 1: Function Pointers = Simple Lambdas (no capture)
 *
 *  Java: Function<Integer,Integer> f = x -> x * 2;
 *  C:    int (*f)(int) = lambda_double;
 * ═══════════════════════════════════════════════════════════ */

/* These are the lambda bodies — static methods in Java's compiled bytecode */
int lambda_double(int x)    { return x * 2; }
int lambda_square(int x)    { return x * x; }
int lambda_negate(int x)    { return -x; }
int lambda_add_ten(int x)   { return x + 10; }

/* Predicate<Integer> */
int pred_is_even(int x)    { return x % 2 == 0; }
int pred_is_positive(int x){ return x > 0; }
int pred_greater_5(int x)  { return x > 5; }

/* Consumer<Integer> */
void consumer_print(int x)        { printf("  → %d\n", x); }
void consumer_print_squared(int x){ printf("  → %d² = %d\n", x, x*x); }

/* Supplier<Integer> */
int supplier_random(void) { return rand() % 100; }
int supplier_zero(void)   { return 0; }

/* BiFunction<Integer,Integer,Integer> */
int bifn_add(int a, int b)  { return a + b; }
int bifn_max(int a, int b)  { return a > b ? a : b; }
int bifn_mul(int a, int b)  { return a * b; }

/* ═══════════════════════════════════════════════════════════
 *  PART 2: Closure — function pointer + captured environment
 *
 *  Java: int factor = 3;
 *        Function<Integer,Integer> f = x -> x * factor;
 *
 *  C:    struct { int factor; } env;
 *        int (*fn)(struct Env*, int);
 * ═══════════════════════════════════════════════════════════ */

/* Generic closure type — mimics Java's lambda with captures */
typedef struct {
    /* The function — takes "self" (captured env) + argument */
    int (*fn)(void *env, int x);
    void *env;   /* heap-allocated captured variables */
} Closure;

/* Execute a closure — like calling a lambda */
int closure_apply(Closure *c, int x) {
    return c->fn(c->env, x);
}

/* A predicate closure */
typedef struct {
    int (*fn)(void *env, int x);
    void *env;
} ClosurePred;

int closure_test(ClosurePred *c, int x) {
    return c->fn(c->env, x);
}

/* ── Closure: multiply by a captured factor ── */
typedef struct { int factor; } MultiplyEnv;

int multiply_fn(void *env, int x) {
    MultiplyEnv *e = (MultiplyEnv *)env;
    return x * e->factor;
}

Closure *make_multiplier(int factor) {
    Closure *c = malloc(sizeof(Closure));
    MultiplyEnv *env = malloc(sizeof(MultiplyEnv));
    env->factor = factor;
    c->fn  = multiply_fn;
    c->env = env;
    return c;
}

/* ── Closure: add a captured offset ── */
typedef struct { int offset; } AddEnv;

int add_fn(void *env, int x) {
    AddEnv *e = (AddEnv *)env;
    return x + e->offset;
}

Closure *make_adder(int offset) {
    Closure *c = malloc(sizeof(Closure));
    AddEnv *env = malloc(sizeof(AddEnv));
    env->offset = offset;
    c->fn  = add_fn;
    c->env = env;
    return c;
}

/* ── Closure: range check ── */
typedef struct { int low; int high; } RangeEnv;

int range_fn(void *env, int x) {
    RangeEnv *e = (RangeEnv *)env;
    return x >= e->low && x <= e->high;
}

ClosurePred *make_range_check(int low, int high) {
    ClosurePred *c = malloc(sizeof(ClosurePred));
    RangeEnv *env = malloc(sizeof(RangeEnv));
    env->low = low; env->high = high;
    c->fn  = range_fn;
    c->env = env;
    return c;
}

void closure_free(Closure *c) { free(c->env); free(c); }
void closure_pred_free(ClosurePred *c) { free(c->env); free(c); }

/* ═══════════════════════════════════════════════════════════
 *  PART 3: Function Composition  —  andThen, compose
 *
 *  Java: Function<Integer,Integer> f = x -> x*2;
 *        Function<Integer,Integer> g = x -> x+1;
 *        Function<Integer,Integer> h = f.andThen(g); // g(f(x))
 * ═══════════════════════════════════════════════════════════ */

typedef int (*IntFn)(int);

/* compose two functions: g(f(x)) */
typedef struct { IntFn f; IntFn g; } ComposedEnv;

int composed_fn(void *env, int x) {
    ComposedEnv *e = (ComposedEnv *)env;
    return e->g(e->f(x));
}

Closure *compose(IntFn f, IntFn g) {
    Closure *c = malloc(sizeof(Closure));
    ComposedEnv *env = malloc(sizeof(ComposedEnv));
    env->f = f; env->g = g;
    c->fn  = composed_fn;
    c->env = env;
    return c;
}

/* ═══════════════════════════════════════════════════════════
 *  PART 4: Higher-order functions — map, filter, reduce
 * ═══════════════════════════════════════════════════════════ */

/* map: apply fn to every element, return new array */
int *map(const int *arr, int len, IntFn fn) {
    int *result = malloc(len * sizeof(int));
    for (int i = 0; i < len; i++)
        result[i] = fn(arr[i]);
    return result;
}

/* filter: keep elements where pred returns true */
int *filter(const int *arr, int len, int (*pred)(int), int *out_len) {
    int *temp = malloc(len * sizeof(int));
    *out_len = 0;
    for (int i = 0; i < len; i++)
        if (pred(arr[i])) temp[(*out_len)++] = arr[i];
    int *result = malloc(*out_len * sizeof(int));
    memcpy(result, temp, *out_len * sizeof(int));
    free(temp);
    return result;
}

/* reduce: fold all elements with binary fn and identity */
int reduce(const int *arr, int len, int identity, int (*fn)(int,int)) {
    int acc = identity;
    for (int i = 0; i < len; i++)
        acc = fn(acc, arr[i]);
    return acc;
}

/* forEach */
void foreach(const int *arr, int len, void (*consumer)(int)) {
    for (int i = 0; i < len; i++) consumer(arr[i]);
}

/* print array helper */
void print_arr(const int *arr, int len) {
    printf("[");
    for (int i = 0; i < len; i++) {
        printf("%d", arr[i]);
        if (i < len-1) printf(", ");
    }
    printf("]\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_function_pointers(void) {
    printf("━━━ 1. Function Pointers = Lambdas (no capture) ━━━━━━━\n");
    printf("Java:  Function<Integer,Integer> f = x -> x * 2;\n\n");

    /* Store lambdas in variables — same as Java's Function<T,R> */
    IntFn f = lambda_double;
    IntFn g = lambda_square;
    IntFn h = lambda_negate;

    printf("  f = x -> x*2:    f(5) = %d\n", f(5));
    printf("  g = x -> x*x:    g(5) = %d\n", g(5));
    printf("  h = x -> -x:     h(5) = %d\n", h(5));

    /* Pass as argument — higher-order function */
    int nums[] = {1, 2, 3, 4, 5};
    printf("\n  map(nums, x->x*2): ");
    int *doubled = map(nums, 5, lambda_double);
    print_arr(doubled, 5);
    free(doubled);

    printf("  map(nums, x->x*x): ");
    int *squared = map(nums, 5, lambda_square);
    print_arr(squared, 5);
    free(squared);
    printf("\n");
}

void demo_closures(void) {
    printf("━━━ 2. Closures — capturing outer variables ━━━━━━━━━━━\n");
    printf("Java:  int factor = 3;\n");
    printf("       Function<Integer,Integer> triple = x -> x * factor;\n\n");

    Closure *triple  = make_multiplier(3);
    Closure *times5  = make_multiplier(5);
    Closure *plus100 = make_adder(100);

    printf("  triple = x -> x*3:   triple(7) = %d\n",   closure_apply(triple, 7));
    printf("  times5 = x -> x*5:   times5(7) = %d\n",   closure_apply(times5, 7));
    printf("  plus100 = x -> x+100: plus100(7) = %d\n", closure_apply(plus100, 7));

    /* Closures are independent — different captured environments */
    printf("\n  triple(10)=%d  times5(10)=%d  plus100(10)=%d\n",
           closure_apply(triple, 10),
           closure_apply(times5, 10),
           closure_apply(plus100, 10));

    /* Range predicate closure */
    ClosurePred *in_range = make_range_check(5, 15);
    printf("\n  in_range = x -> x>=5 && x<=15:\n");
    int tests[] = {3, 7, 15, 20};
    for (int i = 0; i < 4; i++)
        printf("  in_range(%2d) = %s\n", tests[i],
               closure_test(in_range, tests[i]) ? "true" : "false");

    closure_free(triple);
    closure_free(times5);
    closure_free(plus100);
    closure_pred_free(in_range);
    printf("\n");
}

void demo_composition(void) {
    printf("━━━ 3. Function Composition — andThen / compose ━━━━━━━\n");
    printf("Java:  f.andThen(g)  =  x -> g(f(x))\n\n");

    /* f: x -> x*2, g: x -> x+10 */
    /* h = f.andThen(g) = x -> (x*2)+10 */
    Closure *h = compose(lambda_double, lambda_add_ten);

    printf("  f = x -> x*2\n");
    printf("  g = x -> x+10\n");
    printf("  h = f.andThen(g) = x -> (x*2)+10\n");
    printf("  h(5) = %d  (expected %d)\n", closure_apply(h, 5), 5*2+10);
    printf("  h(3) = %d  (expected %d)\n", closure_apply(h, 3), 3*2+10);

    closure_free(h);
    printf("\n");
}

void demo_higher_order(void) {
    printf("━━━ 4. Higher-Order Functions: filter/map/reduce ━━━━━━\n");
    printf("Java:  List.of(1..10).stream()\n");
    printf("           .filter(n -> n%%2==0)\n");
    printf("           .map(n -> n*n)\n");
    printf("           .reduce(0, Integer::sum)\n\n");

    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int n = 10;

    printf("  original: "); print_arr(nums, n);

    /* filter: keep evens */
    int even_len;
    int *evens = filter(nums, n, pred_is_even, &even_len);
    printf("  .filter(even): "); print_arr(evens, even_len);

    /* map: square each */
    int *squared = map(evens, even_len, lambda_square);
    printf("  .map(square):  "); print_arr(squared, even_len);

    /* reduce: sum */
    int total = reduce(squared, even_len, 0, bifn_add);
    printf("  .reduce(sum):  %d\n", total);

    /* forEach */
    printf("  .forEach(print):\n");
    foreach(squared, even_len, consumer_print);

    free(evens);
    free(squared);
    printf("\n");
}

void demo_predicate_composition(void) {
    printf("━━━ 5. Predicate Composition — and / or / negate ━━━━━━\n");
    printf("Java:  Predicate<Integer> p = isEven.and(isPositive);\n\n");

    int nums[] = {-4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6};
    int n = 11;

    /* even AND positive */
    printf("  even AND positive:\n  ");
    for (int i = 0; i < n; i++)
        if (pred_is_even(nums[i]) && pred_is_positive(nums[i]))
            printf("%d ", nums[i]);
    printf("\n");

    /* even OR greater_5 */
    printf("  even OR > 5:\n  ");
    for (int i = 0; i < n; i++)
        if (pred_is_even(nums[i]) || pred_greater_5(nums[i]))
            printf("%d ", nums[i]);
    printf("\n");

    /* NOT even = odd */
    printf("  NOT even (odd):\n  ");
    for (int i = 0; i < n; i++)
        if (!pred_is_even(nums[i]))
            printf("%d ", nums[i]);
    printf("\n\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  01_lambda_closure.c — Java Lambda in C             ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_function_pointers();
    demo_closures();
    demo_composition();
    demo_higher_order();
    demo_predicate_composition();

    printf("━━━ Key Insight ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Lambda (no capture) = plain function pointer\n");
    printf("  Lambda (with capture) = {fn_ptr, captured_env} struct\n");
    printf("  Java creates a hidden class that holds captured vars\n");
    printf("  That hidden class has one method: the lambda body\n");
    printf("  invokedynamic instruction bootstraps this at first call\n");
    printf("  Effectively final: captured vars can't change after capture\n");
    return 0;
}
