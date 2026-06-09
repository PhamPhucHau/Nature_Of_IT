/*
 * 05_exception.c  —  Java try/catch/finally in C with setjmp/longjmp
 *
 * Build:  gcc -Wall -Wextra -o 05_exception 05_exception.c
 * Run:    ./05_exception
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA EXCEPTIONS WORK:
 *
 *   The JVM uses a per-method EXCEPTION TABLE — a list of entries:
 *     { start_pc, end_pc, handler_pc, catch_type }
 *   When an exception is thrown, the JVM walks the call stack
 *   looking for a handler. This is called "stack unwinding".
 *
 *   In C, the equivalent is setjmp/longjmp:
 *     - setjmp(env)  saves the CPU registers + stack pointer (= try {})
 *     - longjmp(env) restores them and jumps back (= throw)
 *
 *   Java's exception hierarchy:
 *     Throwable
 *     ├── Error (OutOfMemoryError, StackOverflowError)
 *     └── Exception
 *         ├── RuntimeException (unchecked)
 *         │   ├── NullPointerException
 *         │   ├── ArrayIndexOutOfBoundsException
 *         │   ├── ArithmeticException
 *         │   └── ClassCastException
 *         └── IOException (checked — must declare throws)
 *
 *   finally block: runs ALWAYS — whether exception thrown or not.
 *   The JVM implements this by duplicating the finally code into
 *   both the normal exit path and every exception handler.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* ═══════════════════════════════════════════════════════════
 *  EXCEPTION TYPES  — mirrors Java's exception hierarchy
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    EX_NONE                    = 0,
    EX_NULL_POINTER            = 1,
    EX_ARRAY_INDEX_OUT_OF_BOUNDS = 2,
    EX_ARITHMETIC              = 3,
    EX_CLASS_CAST              = 4,
    EX_STACK_OVERFLOW          = 5,
    EX_OUT_OF_MEMORY           = 6,
    EX_ILLEGAL_ARGUMENT        = 7,
    EX_IO                      = 8,
    EX_RUNTIME                 = 99,
} ExceptionType;

/* The "Throwable" object */
typedef struct {
    ExceptionType type;
    char          message[256];
    char          class_name[64]; /* e.g. "NullPointerException" */
} Exception;

/* ═══════════════════════════════════════════════════════════
 *  EXCEPTION STACK  — simulates JVM's exception handler stack
 *  Each try-block pushes a handler; throw pops and jumps to it
 * ═══════════════════════════════════════════════════════════ */
#define MAX_EXCEPTION_STACK 32

typedef struct {
    jmp_buf   env;          /* setjmp saves CPU state here */
    Exception pending;      /* the thrown exception        */
    int       has_pending;  /* is there an active exception? */
} ExceptionFrame;

static ExceptionFrame ex_stack[MAX_EXCEPTION_STACK];
static int ex_stack_top = 0;

static Exception current_exception;
static int exception_thrown = 0;

/* ═══════════════════════════════════════════════════════════
 *  MACROS  — make the API look like Java syntax
 * ═══════════════════════════════════════════════════════════ */

/*
 * TRY — saves CPU state with setjmp.
 * setjmp returns 0 on first call (= entering try block).
 * setjmp returns non-zero after longjmp (= exception was thrown).
 */
#define TRY \
    do { \
        jmp_buf _jbuf; \
        exception_thrown = 0; \
        if (setjmp(_jbuf) == 0) { \
            ex_stack[ex_stack_top].env[0] = _jbuf[0]; \
            ex_stack_top++;

/* CATCH(type) — check if thrown exception matches */
#define CATCH(ex_type) \
            ex_stack_top--; \
        } else { \
            ex_stack_top--; \
            if (current_exception.type == (ex_type))

/* CATCH_ALL — catches any exception */
#define CATCH_ALL \
            ex_stack_top--; \
        } else {

/* FINALLY_BEGIN — runs whether exception or not */
#define FINALLY \
            ex_stack_top--; \
        } \
    } while(0); \
    {

#define FINALLY_END \
    }

/* END_TRY — closes try/catch block */
#define END_TRY \
            ex_stack_top--; \
        } \
    } while(0)

/* ═══════════════════════════════════════════════════════════
 *  THROW  — mirrors Java's `throw new XxxException("message")`
 * ═══════════════════════════════════════════════════════════ */
static const char *exception_name(ExceptionType t) {
    switch(t) {
        case EX_NULL_POINTER:             return "NullPointerException";
        case EX_ARRAY_INDEX_OUT_OF_BOUNDS:return "ArrayIndexOutOfBoundsException";
        case EX_ARITHMETIC:               return "ArithmeticException";
        case EX_CLASS_CAST:               return "ClassCastException";
        case EX_STACK_OVERFLOW:           return "StackOverflowError";
        case EX_OUT_OF_MEMORY:            return "OutOfMemoryError";
        case EX_ILLEGAL_ARGUMENT:         return "IllegalArgumentException";
        case EX_IO:                       return "IOException";
        default:                          return "RuntimeException";
    }
}

void throw_exception(ExceptionType type, const char *message) {
    if (ex_stack_top == 0) {
        /* No handler — JVM prints stack trace and exits */
        fprintf(stderr, "Exception in thread \"main\" %s: %s\n",
                exception_name(type), message);
        fprintf(stderr, "\tat <C function>\n");
        exit(1);
    }
    /* Set current exception — like JVM's pending exception slot */
    current_exception.type = type;
    strncpy(current_exception.message, message, 255);
    strncpy(current_exception.class_name, exception_name(type), 63);
    exception_thrown = 1;

    /* longjmp = unwind the stack to the nearest try block */
    longjmp(ex_stack[ex_stack_top - 1].env, 1);
}

/* Convenience throw functions — like `new NullPointerException()` */
#define throw_NPE(msg)          throw_exception(EX_NULL_POINTER, msg)
#define throw_AIOOBE(msg)       throw_exception(EX_ARRAY_INDEX_OUT_OF_BOUNDS, msg)
#define throw_ArithEx(msg)      throw_exception(EX_ARITHMETIC, msg)
#define throw_IllegalArg(msg)   throw_exception(EX_ILLEGAL_ARGUMENT, msg)
#define throw_IOException(msg)  throw_exception(EX_IO, msg)

/* Print exception like Java's e.getMessage(), e.getClass().getName() */
void print_exception(const char *prefix) {
    printf("%s %s: %s\n", prefix,
           current_exception.class_name,
           current_exception.message);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO FUNCTIONS
 * ═══════════════════════════════════════════════════════════ */

/* Simple divide — throws ArithmeticException on divide by zero */
int safe_divide(int a, int b) {
    if (b == 0)
        throw_ArithEx("/ by zero");
    return a / b;
}

/* Array access — throws ArrayIndexOutOfBoundsException */
int array_get(int *arr, int len, int idx) {
    if (idx < 0 || idx >= len)
        throw_AIOOBE("Index out of bounds");
    return arr[idx];
}

/* Null check — throws NullPointerException */
void process_string(const char *s) {
    if (s == NULL)
        throw_NPE("Cannot process null string");
    printf("  Processing: \"%s\"\n", s);
}

void demo_basic_trycatch(void) {
    printf("━━━ 1. Basic try/catch ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  try { int r = 10/0; }\n");
    printf("       catch(ArithmeticException e) { ... }\n\n");

    /* Manually implement try/catch using setjmp */
    jmp_buf env;
    exception_thrown = 0;

    if (setjmp(env) == 0) {
        /* This is the try block */
        ex_stack_top++;
        ex_stack[ex_stack_top-1].env[0] = env[0];

        printf("  Inside try block...\n");
        int result = safe_divide(10, 0);  /* throws! */
        printf("  This line never runs: %d\n", result);

        ex_stack_top--;
    } else {
        /* This is the catch block */
        ex_stack_top--;
        if (current_exception.type == EX_ARITHMETIC) {
            print_exception("  Caught ArithmeticException:");
        }
    }
    printf("\n");
}

void demo_multiple_catch(void) {
    printf("━━━ 2. Multiple catch blocks ━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  try { ... }\n");
    printf("       catch(NullPointerException e) { ... }\n");
    printf("       catch(ArithmeticException e) { ... }\n\n");

    const char *inputs[] = {"hello", NULL, "world"};
    for (int i = 0; i < 3; i++) {
        jmp_buf env;
        exception_thrown = 0;

        if (setjmp(env) == 0) {
            ex_stack_top++;
            ex_stack[ex_stack_top-1].env[0] = env[0];

            printf("  Try with inputs[%d]:\n", i);
            process_string(inputs[i]);

            ex_stack_top--;
        } else {
            ex_stack_top--;
            if (current_exception.type == EX_NULL_POINTER) {
                print_exception("    Caught NullPointerException:");
            } else if (current_exception.type == EX_ARITHMETIC) {
                print_exception("    Caught ArithmeticException:");
            } else {
                print_exception("    Caught Exception:");
            }
        }
    }
    printf("\n");
}

void demo_finally(void) {
    printf("━━━ 3. finally block — always runs ━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  try { ... } catch(E e) { ... } finally { cleanup(); }\n\n");

    /* Simulating:
     *   try {
     *       open_resource();
     *       do_work();       // might throw
     *   } catch (Exception e) {
     *       handle(e);
     *   } finally {
     *       close_resource(); // ALWAYS runs
     *   }
     */

    int resource_opened = 0;
    int exception_in_try = 1;  /* flip to test both paths */

    for (int test = 0; test < 2; test++) {
        printf("  Test %d (%s):\n", test+1,
               test == 0 ? "exception thrown" : "normal exit");
        jmp_buf env;
        exception_thrown = 0;
        resource_opened = 0;

        if (setjmp(env) == 0) {
            ex_stack_top++;
            ex_stack[ex_stack_top-1].env[0] = env[0];

            printf("    [try] Opening resource\n");
            resource_opened = 1;

            if (test == 0)
                throw_IOException("File not found");

            printf("    [try] Normal work done\n");
            ex_stack_top--;
        } else {
            ex_stack_top--;
            print_exception("    [catch]");
        }

        /* FINALLY — always executes (JVM duplicates this code) */
        if (resource_opened) {
            printf("    [finally] Closing resource (resource_opened=%d)\n",
                   resource_opened);
            resource_opened = 0;
        }
        printf("\n");
    }
}

void demo_exception_propagation(void) {
    printf("━━━ 4. Exception propagation (stack unwinding) ━━━━━━━━\n");
    printf("Java:  method3() throws → method2() doesn't catch → method1() catches\n\n");

    void method3(void);
    void method2(void);

    jmp_buf env;
    exception_thrown = 0;

    if (setjmp(env) == 0) {
        ex_stack_top++;
        ex_stack[ex_stack_top-1].env[0] = env[0];

        printf("  method1: entering try block\n");
        printf("  method1: calling method2...\n");

        /* method2 calls method3 which throws — propagates up */
        {
            printf("    method2: calling method3...\n");
            {
                printf("      method3: about to throw\n");
                throw_NPE("Null reference in method3");
                /* method3 never reaches here */
            }
            /* method2 never reaches here */
            printf("    method2: this never runs\n");
        }
        /* method1 never reaches here either */
        printf("  method1: this never runs\n");
        ex_stack_top--;
    } else {
        ex_stack_top--;
        printf("  method1: caught exception that propagated through method2 and method3:\n");
        print_exception("    ");
    }
    printf("\n");
}

void demo_exception_chaining(void) {
    printf("━━━ 5. Uncaught exception — JVM terminates ━━━━━━━━━━━━\n");
    printf("Java:  // No try/catch → JVM prints stack trace and exits\n");
    printf("  (We simulate this — in real code, throw with no handler calls exit)\n");
    printf("  int x = Integer.parseInt(\"abc\");  → NumberFormatException\n");
    printf("  Since ex_stack_top==0, throw_exception() calls exit(1)\n\n");

    printf("  [Skipping actual execution to not terminate the program]\n");
    printf("  In real usage: throw_exception with empty stack → prints + exit\n\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  05_exception.c — Java try/catch/finally in C       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_basic_trycatch();
    demo_multiple_catch();
    demo_finally();
    demo_exception_propagation();
    demo_exception_chaining();

    printf("━━━ Key Concepts ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  setjmp(env)  = try {          — saves CPU state\n");
    printf("  longjmp(env) = throw          — restores state, jumps\n");
    printf("  setjmp returns 0  = entering try block\n");
    printf("  setjmp returns !0 = exception was thrown (longjmp called)\n");
    printf("\n");
    printf("  JVM exception table per method:\n");
    printf("    { from_pc, to_pc, handler_pc, catch_type }\n");
    printf("  JVM walks the stack on throw, finds matching handler\n");
    printf("  finally: JVM duplicates the code into every exit path\n");
    printf("  Checked exceptions: Java enforces at compile time\n");
    printf("  Unchecked (RuntimeException): no enforcement\n");
    return 0;
}
