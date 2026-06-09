/*
 * 06_String_StringBuilder.c  —  Java String + StringBuilder in C
 *
 * Build:  gcc -Wall -Wextra -o 06_String_StringBuilder 06_String_StringBuilder.c
 * Run:    ./06_String_StringBuilder
 *
 * ─────────────────────────────────────────────────────────────────────
 * JAVA STRING INTERNALS:
 *
 *   String is IMMUTABLE. Every operation returns a NEW string.
 *   Backed by: private final byte[] value;  (Java 9+, was char[])
 *   Cached: private int hash;               (computed once, cached)
 *
 *   String pool: literals are interned in a pool (like a HashSet<String>)
 *   "hello" == "hello"  →  true (same pool entry)
 *   new String("hello") == "hello"  →  false (new heap object)
 *
 * JAVA STRINGBUILDER INTERNALS:
 *
 *   Mutable. Used when you need to build a string incrementally.
 *   Backed by: char[] value;  with count (length in use) and capacity.
 *   Capacity grows by: (value.length + 1) * 2
 *
 *   Java Source:
 *     abstract class AbstractStringBuilder {
 *         byte[] value;     // backing array
 *         int    count;     // number of chars used
 *     }
 *
 *   append(s): amortized O(1) — just copy to buffer, grow if needed
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ═══════════════════════════════════════════════════════════
 *  IMMUTABLE STRING — mirrors Java's String
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char *value;    /* private final byte[] value */
    int   length;   /* cached length              */
    int   hash;     /* cached hashCode            */
    int   hash_set; /* flag: has hash been computed? */
} JString;

JString *jstr_of(const char *s) {
    JString *str = malloc(sizeof(JString));
    str->length   = (int)strlen(s);
    str->value    = malloc(str->length + 1);
    memcpy(str->value, s, str->length + 1);
    str->hash     = 0;
    str->hash_set = 0;
    return str;
}

void jstr_free(JString *s) {
    if (!s) return;
    free(s->value);
    free(s);
}

/* String.hashCode() — computed once, cached */
int jstr_hashcode(JString *s) {
    if (s->hash_set) return s->hash;
    int h = 0;
    for (int i = 0; i < s->length; i++)
        h = 31 * h + (unsigned char)s->value[i];
    s->hash     = h;
    s->hash_set = 1;
    return h;
}

/* String.equals() — value comparison */
int jstr_equals(const JString *a, const JString *b) {
    if (a == b)             return 1;  /* same pointer = same object */
    if (a->length != b->length) return 0;
    return memcmp(a->value, b->value, a->length) == 0;
}

/* String.compareTo() */
int jstr_compare_to(const JString *a, const JString *b) {
    return strcmp(a->value, b->value);
}

/* String.charAt(i) */
char jstr_char_at(const JString *s, int i) {
    if (i < 0 || i >= s->length) {
        fprintf(stderr, "StringIndexOutOfBoundsException: %d\n", i); exit(1);
    }
    return s->value[i];
}

/* String.substring(begin, end) — returns NEW string (immutable!) */
JString *jstr_substring(const JString *s, int begin, int end) {
    if (begin < 0 || end > s->length || begin > end) {
        fprintf(stderr, "StringIndexOutOfBoundsException\n"); exit(1);
    }
    int len = end - begin;
    JString *sub = malloc(sizeof(JString));
    sub->value   = malloc(len + 1);
    memcpy(sub->value, s->value + begin, len);
    sub->value[len] = '\0';
    sub->length  = len;
    sub->hash_set = 0;
    return sub;
}

/* String + String — returns NEW string */
JString *jstr_concat(const JString *a, const JString *b) {
    JString *result = malloc(sizeof(JString));
    result->length  = a->length + b->length;
    result->value   = malloc(result->length + 1);
    memcpy(result->value,             a->value, a->length);
    memcpy(result->value + a->length, b->value, b->length);
    result->value[result->length] = '\0';
    result->hash_set = 0;
    return result;
}

/* String.toUpperCase() — returns NEW string */
JString *jstr_to_upper(const JString *s) {
    JString *result = jstr_of(s->value);
    for (int i = 0; i < result->length; i++)
        result->value[i] = (char)toupper((unsigned char)result->value[i]);
    return result;
}

/* String.trim() — returns NEW string */
JString *jstr_trim(const JString *s) {
    int start = 0, end = s->length;
    while (start < end && s->value[start] == ' ') start++;
    while (end > start && s->value[end-1] == ' ') end--;
    return jstr_substring(s, start, end);
}

/* String.contains(sub) */
int jstr_contains(const JString *s, const char *sub) {
    return strstr(s->value, sub) != NULL;
}

/* String.indexOf(sub) */
int jstr_index_of(const JString *s, const char *sub) {
    char *found = strstr(s->value, sub);
    return found ? (int)(found - s->value) : -1;
}

/* String.split(delim) — simple version */
void jstr_split(const JString *s, char delim,
                char ***parts, int *count) {
    *count = 1;
    for (int i = 0; i < s->length; i++)
        if (s->value[i] == delim) (*count)++;

    *parts = malloc(*count * sizeof(char *));
    int part = 0, start = 0;
    for (int i = 0; i <= s->length; i++) {
        if (i == s->length || s->value[i] == delim) {
            int len = i - start;
            (*parts)[part] = malloc(len + 1);
            memcpy((*parts)[part], s->value + start, len);
            (*parts)[part][len] = '\0';
            part++;
            start = i + 1;
        }
    }
}

/* ═══════════════════════════════════════════════════════════
 *  STRINGBUILDER — mutable, O(1) amortized append
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    char *value;    /* byte[] value   — backing buffer     */
    int   count;    /* int   count    — chars used         */
    int   capacity; /* value.length   — buffer size        */
} StringBuilder;

#define SB_DEFAULT_CAPACITY 16

StringBuilder *sb_new(void) {
    StringBuilder *sb = malloc(sizeof(StringBuilder));
    sb->capacity = SB_DEFAULT_CAPACITY;
    sb->value    = malloc(sb->capacity);
    sb->count    = 0;
    printf("[ALLOC] StringBuilder  capacity=%d\n", sb->capacity);
    return sb;
}

StringBuilder *sb_new_with_capacity(int cap) {
    StringBuilder *sb = malloc(sizeof(StringBuilder));
    sb->capacity = cap;
    sb->value    = malloc(sb->capacity);
    sb->count    = 0;
    return sb;
}

void sb_free(StringBuilder *sb) {
    free(sb->value);
    free(sb);
}

/* Grow: (count + append_len + 1) vs (capacity + 1) * 2 — Java formula */
static void sb_ensure_capacity(StringBuilder *sb, int min_capacity) {
    if (min_capacity <= sb->capacity) return;
    int new_cap = (sb->capacity + 1) * 2;  /* Java's exact formula */
    if (new_cap < min_capacity) new_cap = min_capacity;
    sb->value    = realloc(sb->value, new_cap);
    printf("[GROW]  StringBuilder capacity: %d → %d\n", sb->capacity, new_cap);
    sb->capacity = new_cap;
}

/* sb.append(String) — returns sb for chaining */
StringBuilder *sb_append(StringBuilder *sb, const char *s) {
    int len = (int)strlen(s);
    sb_ensure_capacity(sb, sb->count + len + 1);
    memcpy(sb->value + sb->count, s, len);
    sb->count += len;
    sb->value[sb->count] = '\0';
    return sb;  /* enables chaining: sb.append("a").append("b") */
}

/* sb.append(int) */
StringBuilder *sb_append_int(StringBuilder *sb, int n) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", n);
    return sb_append(sb, buf);
}

/* sb.append(char) */
StringBuilder *sb_append_char(StringBuilder *sb, char c) {
    sb_ensure_capacity(sb, sb->count + 2);
    sb->value[sb->count++] = c;
    sb->value[sb->count]   = '\0';
    return sb;
}

/* sb.insert(offset, s) */
void sb_insert(StringBuilder *sb, int offset, const char *s) {
    int len = (int)strlen(s);
    sb_ensure_capacity(sb, sb->count + len + 1);
    memmove(sb->value + offset + len,
            sb->value + offset,
            sb->count - offset + 1);
    memcpy(sb->value + offset, s, len);
    sb->count += len;
}

/* sb.delete(start, end) */
void sb_delete(StringBuilder *sb, int start, int end) {
    if (start < 0 || end > sb->count || start > end) {
        fprintf(stderr, "StringIndexOutOfBoundsException\n"); exit(1);
    }
    memmove(sb->value + start,
            sb->value + end,
            sb->count - end + 1);
    sb->count -= (end - start);
}

/* sb.reverse() — modifies in place */
void sb_reverse(StringBuilder *sb) {
    for (int i = 0, j = sb->count - 1; i < j; i++, j--) {
        char t = sb->value[i];
        sb->value[i] = sb->value[j];
        sb->value[j] = t;
    }
}

/* sb.setCharAt(i, c) */
void sb_set_char_at(StringBuilder *sb, int i, char c) {
    if (i < 0 || i >= sb->count) {
        fprintf(stderr, "StringIndexOutOfBoundsException\n"); exit(1);
    }
    sb->value[i] = c;
}

/* sb.toString() — creates immutable JString from StringBuilder */
JString *sb_to_string(const StringBuilder *sb) {
    return jstr_of(sb->value);
}

/* sb.length() */
int sb_length(const StringBuilder *sb) { return sb->count; }

void sb_print_debug(const StringBuilder *sb) {
    printf("  StringBuilder: value='%s'  count=%d  capacity=%d\n",
           sb->value, sb->count, sb->capacity);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_string_immutability(void) {
    printf("━━━ 1. String Immutability ━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    JString *a = jstr_of("Hello");
    JString *b = jstr_of("World");

    /* + operator creates a NEW string — a and b are unchanged */
    JString *c = jstr_concat(a, b);
    JString *d = jstr_to_upper(a);
    JString *e = jstr_substring(a, 1, 4);

    printf("  a = \"%s\"\n", a->value);
    printf("  b = \"%s\"\n", b->value);
    printf("  a + b = \"%s\"  (NEW string, a unchanged)\n", c->value);
    printf("  toUpperCase(a) = \"%s\"  (NEW string)\n", d->value);
    printf("  a.substring(1,4) = \"%s\"\n", e->value);
    printf("  a.hashCode() = %d  (cached)\n", jstr_hashcode(a));
    printf("  a.hashCode() = %d  (cached again — same value, no recompute)\n",
           jstr_hashcode(a));

    jstr_free(a); jstr_free(b); jstr_free(c); jstr_free(d); jstr_free(e);
    printf("\n");
}

void demo_string_concatenation_pain(void) {
    printf("━━━ 2. String + in loop — why it's slow ━━━━━━━━━━━━━━\n");
    printf("  Java:  String s = \"\";\n");
    printf("         for(int i=0;i<1000;i++) s = s + i;\n");
    printf("  → Creates 1000 TEMPORARY String objects!\n");
    printf("  Each s = s + i does:\n");
    printf("    1. Allocate new String of length(s) + length(i)\n");
    printf("    2. Copy s into new string\n");
    printf("    3. Copy i into new string\n");
    printf("    4. Old s becomes garbage (GC must collect it)\n");
    printf("  Total work: O(n²) for n concatenations\n\n");

    printf("  Solution: use StringBuilder — O(n) total\n");
    printf("  Java:  StringBuilder sb = new StringBuilder();\n");
    printf("         for(int i=0;i<1000;i++) sb.append(i);\n");
    printf("         String result = sb.toString();\n");
    printf("  → Only 1 buffer, grows as needed, O(1) per append\n\n");
}

void demo_stringbuilder(void) {
    printf("━━━ 3. StringBuilder — mutable buffer ━━━━━━━━━━━━━━━━\n");

    StringBuilder *sb = sb_new();

    /* Chained appends — like Java's sb.append(...).append(...) */
    sb_append(sb, "Hello");
    sb_append_char(sb, ',');
    sb_append(sb, " ");
    sb_append(sb, "World");
    sb_append_char(sb, '!');
    sb_append_int(sb, 42);
    sb_print_debug(sb);

    /* insert */
    sb_insert(sb, 5, " Beautiful");
    printf("  After insert at 5: '%s'\n", sb->value);

    /* delete */
    sb_delete(sb, 5, 15);
    printf("  After delete(5,15): '%s'\n", sb->value);

    /* reverse */
    StringBuilder *rev = sb_new();
    sb_append(rev, "Hello");
    sb_reverse(rev);
    printf("  reverse(\"Hello\") = '%s'\n", rev->value);
    sb_free(rev);

    /* Build string from loop — efficient O(n) */
    printf("\n  Building string with loop (O(n) vs O(n²) for + operator):\n");
    StringBuilder *loop_sb = sb_new_with_capacity(50);
    for (int i = 1; i <= 10; i++) {
        sb_append_int(loop_sb, i);
        if (i < 10) sb_append_char(loop_sb, ',');
    }
    JString *result = sb_to_string(loop_sb);
    printf("  Result: '%s'\n", result->value);
    sb_free(loop_sb);
    jstr_free(result);

    /* Watch capacity grow */
    printf("\n  Capacity growth demo (start capacity=%d):\n",
           SB_DEFAULT_CAPACITY);
    StringBuilder *growing = sb_new();
    sb_print_debug(growing);
    for (int i = 0; i < 5; i++) {
        sb_append(growing, "hello world! ");
    }
    sb_print_debug(growing);
    sb_free(growing);

    sb_free(sb);
    printf("\n");
}

void demo_string_operations(void) {
    printf("━━━ 4. String operations ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    JString *s = jstr_of("  Hello, Java World!  ");
    printf("  original:     '%s'\n", s->value);

    JString *trimmed = jstr_trim(s);
    printf("  trim():       '%s'\n", trimmed->value);

    JString *upper   = jstr_to_upper(trimmed);
    printf("  toUpperCase(): '%s'\n", upper->value);

    printf("  contains('Java'): %d\n", jstr_contains(s, "Java"));
    printf("  indexOf('Java'):  %d\n", jstr_index_of(s, "Java"));
    printf("  charAt(8):        '%c'\n", jstr_char_at(trimmed, 7));

    /* split */
    char **parts; int count;
    jstr_split(trimmed, ' ', &parts, &count);
    printf("  split(' '):\n");
    for (int i = 0; i < count; i++) {
        printf("    [%d] '%s'\n", i, parts[i]);
        free(parts[i]);
    }
    free(parts);

    jstr_free(s); jstr_free(trimmed); jstr_free(upper);
    printf("\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  06_String_StringBuilder.c — String internals in C  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_string_immutability();
    demo_string_concatenation_pain();
    demo_stringbuilder();
    demo_string_operations();

    printf("━━━ Key Insights ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  String:        immutable, cached hash, pool-interned literals\n");
    printf("  StringBuilder: mutable buffer, O(1) append, grows ×2\n");
    printf("  Always use StringBuilder in loops — never String + in loop!\n");
    printf("  String.format()  internally uses StringBuilder\n");
    printf("  Compiler optimizes literal concat: \"a\"+\"b\" → \"ab\" at compile time\n");

    return 0;
}
