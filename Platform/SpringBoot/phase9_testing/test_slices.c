/*
 * test_slices.c  —  Spring Boot Test Slices in C
 *
 * Build:  gcc -Wall -Wextra -o test_slices test_slices.c
 * Run:    ./test_slices
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Spring Boot Test Slices: the mechanism that loads ONLY the beans
 *   relevant to one layer, making tests fast and focused.
 *
 *   Key classes:
 *     @SpringBootTest → AnnotationConfigServletWebServerApplicationContext
 *                       loads ALL auto-configurations + component scan
 *
 *     @WebMvcTest    → WebMvcTypeExcludeFilter
 *                       loads: @Controller, MockMvc, security filters
 *                       excludes: @Service, @Repository, @Component
 *
 *     @DataJpaTest   → DataJpaTypeExcludeFilter + AutoConfigureTestDatabase
 *                       loads: @Repository, JPA, DataSource (H2 in-memory)
 *                       excludes: @Controller, @Service
 *
 *   @MockBean: replaces a real bean in the context with a mock.
 *   In Spring, this works by registering a BeanDefinition that returns
 *   a Mockito mock object instead of the real implementation.
 *
 * C APPROACH:
 *   We model each test slice as a "context filter" — a function that
 *   decides which beans to include. We show:
 *     - Which beans are loaded per slice
 *     - How @MockBean injects stub behavior
 *     - How MockMvc simulates HTTP without a real server
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  BEAN LAYER CLASSIFICATION
 *  Each bean belongs to one layer.
 *  Slice annotations filter based on these layers.
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    LAYER_CONTROLLER,    /* @RestController, @Controller */
    LAYER_SERVICE,       /* @Service                     */
    LAYER_REPOSITORY,    /* @Repository                  */
    LAYER_COMPONENT,     /* @Component (generic)         */
    LAYER_CONFIG,        /* @Configuration               */
    LAYER_JPA,           /* JPA infrastructure beans     */
    LAYER_DATASOURCE,    /* DataSource, connection pool  */
    LAYER_SECURITY,      /* Spring Security filters      */
    LAYER_CONVERTER,     /* HttpMessageConverter, Jackson */
    LAYER_MOCK,          /* @MockBean replacements       */
} BeanLayer;

static const char* layer_name(BeanLayer l) {
    static const char* names[] = {
        "CONTROLLER", "SERVICE", "REPOSITORY", "COMPONENT",
        "CONFIG", "JPA", "DATASOURCE", "SECURITY", "CONVERTER", "MOCK"
    };
    return names[(int)l < 10 ? (int)l : 9];
}

/* ═══════════════════════════════════════════════════════════
 *  BEAN DESCRIPTOR — a bean candidate
 * ═══════════════════════════════════════════════════════════ */
#define MAX_BEANS 32

typedef struct {
    const char *name;
    const char *class_name;
    BeanLayer   layer;
    int         is_mock;    /* 1 = @MockBean (always included in test context) */
} BeanDescriptor;

/* ═══════════════════════════════════════════════════════════
 *  TEST SLICE TYPES
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    SLICE_FULL,        /* @SpringBootTest — all beans */
    SLICE_WEB_MVC,     /* @WebMvcTest — controllers only */
    SLICE_DATA_JPA,    /* @DataJpaTest — repositories + JPA only */
    SLICE_JSON,        /* @JsonTest — Jackson serialization only */
} SliceType;

/* ─────────────────────────────────────────────────────────────
 *  Filter: should this bean be included in this slice?
 *  Java: TypeExcludeFilter.match() for each bean class
 * ──────────────────────────────────────────────────────────── */
static int bean_included_in_slice(const BeanDescriptor *b, SliceType slice) {
    if (b->is_mock) return 1;  /* @MockBean always included */

    switch (slice) {
        case SLICE_FULL:
            return 1;  /* everything */

        case SLICE_WEB_MVC:
            /* Includes: controllers, converters, security, config */
            return (b->layer == LAYER_CONTROLLER ||
                    b->layer == LAYER_CONVERTER   ||
                    b->layer == LAYER_SECURITY     ||
                    b->layer == LAYER_CONFIG);

        case SLICE_DATA_JPA:
            /* Includes: repositories, JPA infrastructure, datasource */
            return (b->layer == LAYER_REPOSITORY ||
                    b->layer == LAYER_JPA         ||
                    b->layer == LAYER_DATASOURCE   ||
                    b->layer == LAYER_CONFIG);

        case SLICE_JSON:
            /* Includes: Jackson converters only */
            return (b->layer == LAYER_CONVERTER);

        default:
            return 0;
    }
}

/* ─────────────────────────────────────────────────────────────
 *  MOCK CALL RECORDER — simulates Mockito stub/verify
 * ──────────────────────────────────────────────────────────── */
#define MAX_CALL_RECORDS 32

typedef struct {
    const char *bean_name;
    const char *method_name;
    const char *arg;
    const char *return_val;
} CallRecord;

typedef struct {
    CallRecord records[MAX_CALL_RECORDS];
    int        count;
} MockRegistry;

static void mock_stub(MockRegistry *mr, const char *bean,
                       const char *method, const char *arg, const char *ret) {
    if (mr->count >= MAX_CALL_RECORDS) return;
    mr->records[mr->count].bean_name   = bean;
    mr->records[mr->count].method_name = method;
    mr->records[mr->count].arg        = arg;
    mr->records[mr->count].return_val  = ret;
    mr->count++;
}

static const char* mock_invoke(const MockRegistry *mr, const char *bean,
                                 const char *method, const char *arg) {
    for (int i = 0; i < mr->count; i++) {
        if (strcmp(mr->records[i].bean_name,   bean)   == 0 &&
            strcmp(mr->records[i].method_name, method) == 0 &&
            (mr->records[i].arg == NULL ||
             strcmp(mr->records[i].arg, arg) == 0)) {
            return mr->records[i].return_val;
        }
    }
    return "(no stub — returns null)";
}

/* ─────────────────────────────────────────────────────────────
 *  MOCKMVC — simulates HTTP request/response without a real server
 *  Java: MockMvc.perform(get("/api/users")).andExpect(status().isOk())
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    const char *method;
    const char *path;
    const char *body;
    const char *auth_header;
    int         expected_status;
} MockMvcRequest;

typedef struct {
    int         status;
    const char *body;
    const char *content_type;
    int         passed;
} MockMvcResult;

/* ─────────────────────────────────────────────────────────────
 *  Print test context beans
 * ──────────────────────────────────────────────────────────── */
static void print_context(const BeanDescriptor *all_beans, int n,
                            SliceType slice, const char *slice_name) {
    printf("  Context type: %s\n", slice_name);
    printf("  %-35s %-12s %s\n", "Bean", "Layer", "Included");
    printf("  %-35s %-12s %s\n",
           "───────────────────────────────────",
           "────────────",
           "────────");

    int included = 0, excluded = 0;
    for (int i = 0; i < n; i++) {
        int incl = bean_included_in_slice(&all_beans[i], slice);
        printf("  %-35s %-12s %s\n",
               all_beans[i].name,
               layer_name(all_beans[i].layer),
               incl ? (all_beans[i].is_mock ? "✅ MOCK" : "✅ REAL") : "⏭  skip");
        if (incl) included++; else excluded++;
    }
    printf("\n  Total: %d beans (included=%d, excluded=%d)\n\n",
           n, included, excluded);
}

/* ═══════════════════════════════════════════════════════════
 *  ALL APPLICATION BEANS (the full list a real Spring Boot app has)
 * ═══════════════════════════════════════════════════════════ */
static const BeanDescriptor ALL_BEANS[] = {
    /* Controllers */
    { "userController",        "UserController",              LAYER_CONTROLLER, 0 },
    { "orderController",       "OrderController",             LAYER_CONTROLLER, 0 },
    { "authController",        "AuthController",              LAYER_CONTROLLER, 0 },

    /* Services */
    { "userService",           "UserServiceImpl",             LAYER_SERVICE,    0 },
    { "orderService",          "OrderServiceImpl",            LAYER_SERVICE,    0 },
    { "emailService",          "SmtpEmailService",            LAYER_SERVICE,    0 },
    { "paymentService",        "StripePaymentService",        LAYER_SERVICE,    0 },

    /* Repositories */
    { "userRepository",        "UserRepository",              LAYER_REPOSITORY, 0 },
    { "orderRepository",       "OrderRepository",             LAYER_REPOSITORY, 0 },

    /* JPA infrastructure */
    { "entityManagerFactory",  "LocalContainerEntityManagerFactory", LAYER_JPA, 0 },
    { "transactionManager",    "JpaTransactionManager",       LAYER_JPA,        0 },

    /* DataSource */
    { "dataSource",            "HikariDataSource",            LAYER_DATASOURCE, 0 },

    /* Converters */
    { "jacksonObjectMapper",   "ObjectMapper",                LAYER_CONVERTER,  0 },
    { "mappingJackson2HttpConverter", "MappingJackson2HttpMessageConverter", LAYER_CONVERTER, 0 },

    /* Security */
    { "springSecurityFilterChain", "DefaultSecurityFilterChain", LAYER_SECURITY, 0 },
    { "jwtAuthenticationFilter",   "JwtAuthenticationFilter",    LAYER_SECURITY, 0 },

    /* Config */
    { "webMvcConfigurer",      "WebMvcAutoConfiguration",     LAYER_CONFIG,     0 },
    { "requestMappingHandlerMapping", "RequestMappingHandlerMapping", LAYER_CONFIG, 0 },
};
static int total_beans = (int)(sizeof(ALL_BEANS) / sizeof(ALL_BEANS[0]));

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_web_mvc_test(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: @WebMvcTest(UserController.class)\n");
    printf("Only controller layer + mocked services\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Add @MockBean: UserService (mocked, not real) */
    BeanDescriptor beans_with_mocks[MAX_BEANS];
    memcpy(beans_with_mocks, ALL_BEANS, sizeof(ALL_BEANS));
    int n = total_beans;

    /* Override UserService with mock */
    for (int i = 0; i < n; i++) {
        if (strcmp(beans_with_mocks[i].name, "userService") == 0) {
            beans_with_mocks[i].is_mock = 1;  /* @MockBean */
            break;
        }
    }

    print_context(beans_with_mocks, n, SLICE_WEB_MVC, "@WebMvcTest");

    /* Setup mock stubs */
    MockRegistry mocks = {0};
    mock_stub(&mocks, "userService", "findAll", NULL,
              "[{\"id\":1,\"name\":\"Alice\"},{\"id\":2,\"name\":\"Bob\"}]");
    mock_stub(&mocks, "userService", "findById", "42",
              "{\"id\":42,\"name\":\"Alice\"}");
    mock_stub(&mocks, "userService", "findById", "999",
              NULL);  /* not found → 404 */

    /* Simulate MockMvc tests */
    printf("  ── Test: GET /api/users ──────────────────────────────\n");
    const char *result = mock_invoke(&mocks, "userService", "findAll", NULL);
    printf("  mockMvc.perform(get(\"/api/users\"))\n");
    printf("    userService.findAll() returns: %s\n", result);
    printf("    andExpect(status().isOk())         → ✅ PASS (200)\n");
    printf("    andExpect(jsonPath(\"$[0].name\").value(\"Alice\")) → ✅ PASS\n\n");

    printf("  ── Test: GET /api/users/42 ───────────────────────────\n");
    result = mock_invoke(&mocks, "userService", "findById", "42");
    printf("  mockMvc.perform(get(\"/api/users/42\"))\n");
    printf("    userService.findById(42) returns: %s\n", result);
    printf("    andExpect(status().isOk())         → ✅ PASS (200)\n\n");

    printf("  ── Test: GET /api/users/999 (not found) ─────────────\n");
    result = mock_invoke(&mocks, "userService", "findById", "999");
    printf("  mockMvc.perform(get(\"/api/users/999\"))\n");
    printf("    userService.findById(999) returns: %s\n",
           result ? result : "null");
    printf("    andExpect(status().isNotFound())   → ✅ PASS (404)\n\n");
}

void demo_data_jpa_test(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: @DataJpaTest\n");
    printf("Only repository layer + in-memory H2 database\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    print_context(ALL_BEANS, total_beans, SLICE_DATA_JPA, "@DataJpaTest");

    printf("  ── H2 In-Memory Database (AutoConfigureTestDatabase) ───\n");
    printf("  Original datasource: jdbc:mysql://prod-host:3306/mydb\n");
    printf("  REPLACED with:       jdbc:h2:mem:testdb (H2 in-memory)\n");
    printf("  spring.jpa.hibernate.ddl-auto = create-drop (auto schema)\n\n");

    printf("  ── Test: UserRepository.findByEmail() ──────────────────\n");
    printf("  // Arrange\n");
    printf("  userRepository.save(new User(\"Alice\", \"alice@example.com\"));\n\n");
    printf("  // Act\n");
    printf("  Optional<User> found = userRepository.findByEmail(\"alice@example.com\");\n\n");
    printf("  // Assert\n");
    printf("  assertThat(found).isPresent();                → ✅ PASS\n");
    printf("  assertThat(found.get().getName()).isEqualTo(\"Alice\"); → ✅ PASS\n\n");

    printf("  Note: @DataJpaTest wraps each test in @Transactional\n");
    printf("        → rollback after each test = clean DB state\n\n");
}

void demo_full_spring_boot_test(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: @SpringBootTest — Full Context\n");
    printf("All beans + @MockBean for external services\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Mock only the external payment service */
    BeanDescriptor beans_with_mocks[MAX_BEANS];
    memcpy(beans_with_mocks, ALL_BEANS, sizeof(ALL_BEANS));
    int n = total_beans;

    for (int i = 0; i < n; i++) {
        if (strcmp(beans_with_mocks[i].name, "paymentService") == 0 ||
            strcmp(beans_with_mocks[i].name, "emailService")   == 0) {
            beans_with_mocks[i].is_mock = 1;
        }
    }

    print_context(beans_with_mocks, n, SLICE_FULL, "@SpringBootTest");

    printf("  Context startup time: ~3-8 seconds (full context is expensive)\n");
    printf("  Best practice: reuse context across test classes (@DirtiesContext sparingly)\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  test_slices.c — Spring Boot Test Slices in C       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_web_mvc_test();
    demo_data_jpa_test();
    demo_full_spring_boot_test();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  @WebMvcTest  = controller beans only + MockMvc  (fast: ~500ms)\n");
    printf("  @DataJpaTest = repository + JPA + H2             (fast: ~1s)\n");
    printf("  @SpringBootTest = full context                    (slow: ~5s)\n");
    printf("  @MockBean replaces bean in Spring context with Mockito mock\n");
    printf("  @DataJpaTest replaces real DB with H2 in-memory automatically\n");
    printf("  @Transactional in tests = rollback after each test\n");
    return 0;
}
