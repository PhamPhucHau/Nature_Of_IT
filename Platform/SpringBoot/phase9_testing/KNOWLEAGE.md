# 🧪 Phase 9: Spring Boot Testing

> **Spring Boot Concept**: `@SpringBootTest`, `@MockBean`, `@DataJpaTest`, `@WebMvcTest`
> **C Equivalent**: Test harness that creates a partial context with stubs replacing real dependencies

---

## Testing Pyramid in Spring Boot

```text
        ╔══════╗
        ║  E2E ║  @SpringBootTest(webEnvironment=RANDOM_PORT)
        ╠══════╣  Full context + real HTTP + real DB
        ║ Integ║  @SpringBootTest
        ╠══════╣  Full context, no HTTP (TestRestTemplate)
        ║Slice ║  @WebMvcTest / @DataJpaTest / @ServiceTest
        ╠══════╣  Partial context, mock dependencies
        ║ Unit ║  Plain JUnit + Mockito (no Spring context at all)
        ╚══════╝

More context = slower test. Less context = faster test.
Use the SMALLEST context that covers what you're testing.
```

---

## @SpringBootTest — Full Application Context

```java
@SpringBootTest
@Transactional  // rolls back after each test
class UserServiceIntegrationTest {

    @Autowired
    private UserService userService;   // real bean from context

    @MockBean                          // replaces real bean with Mockito mock
    private EmailService emailService;

    @Test
    void shouldCreateUser() {
        User user = userService.createUser(new CreateUserRequest("Alice", "alice@example.com"));

        assertThat(user.getId()).isNotNull();
        // emailService.send() was called (mocked, not real email sent)
        verify(emailService).send(any(EmailMessage.class));
    }
}
```

---

## Test Slices — Partial Contexts

Each slice loads ONLY the beans relevant to one layer:

### @WebMvcTest — Controller layer only
```java
@WebMvcTest(UserController.class)
class UserControllerTest {

    @Autowired
    private MockMvc mockMvc;           // simulates HTTP requests

    @MockBean
    private UserService userService;   // service is mocked

    @Test
    void shouldReturnUserList() throws Exception {
        given(userService.findAll()).willReturn(List.of(new User(1L, "Alice")));

        mockMvc.perform(get("/api/users"))
               .andExpect(status().isOk())
               .andExpect(jsonPath("$[0].name").value("Alice"));
    }
}
```

### @DataJpaTest — JPA/Repository layer only
```java
@DataJpaTest   // loads: JPA, H2, @Repository, @Entity — NOT @Service, @Controller
class UserRepositoryTest {

    @Autowired
    private UserRepository userRepository;   // real JPA, in-memory H2

    @Test
    void shouldFindByEmail() {
        userRepository.save(new User("Alice", "alice@example.com"));

        Optional<User> found = userRepository.findByEmail("alice@example.com");

        assertThat(found).isPresent();
        assertThat(found.get().getName()).isEqualTo("Alice");
    }
}
```

### @RestClientTest — HTTP client testing
```java
@RestClientTest(PaymentClient.class)
class PaymentClientTest {

    @Autowired MockRestServiceServer server;
    @Autowired PaymentClient client;

    @Test
    void shouldCallPaymentGateway() {
        server.expect(requestTo("/charge"))
              .andRespond(withSuccess(...));

        client.charge(100.00, "tok_visa");
        server.verify();
    }
}
```

---

## @MockBean vs @Mock

```text
@Mock (Mockito only, no Spring):
  Creates a Mockito mock. Does NOT replace bean in Spring context.
  Use in pure unit tests (no @SpringBootTest).

@MockBean (Spring Boot):
  Creates a Mockito mock AND replaces the bean in the Spring context.
  Other beans that @Autowire the mocked type get this mock instead.
  Context is re-created per test class (caching if same config).

@SpyBean (Spring Boot):
  Like @MockBean but wraps the REAL bean.
  Real methods are called unless stubbed.
  Use when you need real behavior except for specific methods.
```

---

## Test Slices Comparison

| Annotation | Loads | Does NOT load | Use for |
|------------|-------|---------------|---------|
| `@SpringBootTest` | Full context | — | Integration tests |
| `@WebMvcTest` | @Controller, @ControllerAdvice, Filters, @JsonComponent | @Service, @Repository | Controller tests |
| `@DataJpaTest` | @Repository, JPA, DataSource | @Service, @Controller | Repository tests |
| `@DataRedisTest` | Spring Data Redis | everything else | Redis tests |
| `@RestClientTest` | RestTemplate, MockServer | everything else | HTTP client tests |
| `@JsonTest` | Jackson, serialization | everything else | JSON mapping tests |

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `spring_boot_test.c` | Context creation + injection for test |
| `mock_bean.c` | @MockBean replacement mechanism |
| `test_slices.c` | Partial context loading per slice type |

---

## Build and Run

```bash
gcc -Wall -Wextra -o spring_boot_test spring_boot_test.c && ./spring_boot_test
gcc -Wall -Wextra -o test_slices test_slices.c && ./test_slices
```

---

## Key Insight

```text
@SpringBootTest = full Spring context in a test JVM.
It's slow (seconds) because it starts the entire application.

Test slices = partial context with only the layer you're testing.
They're fast (milliseconds) because they load 10 beans instead of 100.

@MockBean = "replace this bean in the context with a Mockito mock".
This is the bridge between Spring's DI and Mockito's stub/verify.

Rule:
  Unit test:        no Spring context (plain Java + Mockito)
  Slice test:       @WebMvcTest / @DataJpaTest (fast, targeted)
  Integration test: @SpringBootTest (slow, but tests real wiring)
```
