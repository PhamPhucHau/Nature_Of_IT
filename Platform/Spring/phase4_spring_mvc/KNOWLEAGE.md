# 🌐 Phase 4: Spring MVC — DispatcherServlet

> **Spring Concept**: `DispatcherServlet`, `HandlerMapping`, `HandlerAdapter`
> **C Equivalent**: A routing table of (HTTP method, path) → function pointers

---

## The DispatcherServlet Pattern

Spring MVC uses the **Front Controller** design pattern:

```text
ALL requests → ONE servlet → DispatcherServlet → routes to @Controller
```

This is the opposite of old Java web apps where every URL had its own servlet.

---

## Full Request Processing Flow

```text
Browser: GET /api/users?page=1 HTTP/1.1

  ↓ TCP Socket (kernel)
  ↓ Embedded Tomcat (parses HTTP bytes → HttpServletRequest)
  ↓ FilterChain (security filters, logging filters, CORS filter)

DispatcherServlet.doDispatch(request, response)
  ↓
  ├─ 1. HandlerMapping.getHandler(request)
  │      → finds: UserController.getUsers() method
  │      → returns: HandlerExecutionChain(handler + interceptors)
  │
  ├─ 2. HandlerAdapter.handle(request, response, handler)
  │      → reads @RequestParam, @PathVariable, @RequestBody
  │      → deserializes JSON body if @RequestBody present
  │      → invokes UserController.getUsers(page=1) via Reflection
  │      → returns: ModelAndView or @ResponseBody object
  │
  ├─ 3. HandlerInterceptor.postHandle()  ← AOP interceptors
  │
  ├─ 4. HttpMessageConverter.write()
  │      → serializes return value to JSON bytes
  │      → writes Content-Type: application/json header
  │
  └─ 5. HTTP/1.1 200 OK \r\n...
```

---

## @RequestMapping Resolution

```java
@RestController
@RequestMapping("/api")
public class UserController {

    @GetMapping("/users")           // GET /api/users
    public List<User> getUsers(
        @RequestParam int page) { }

    @GetMapping("/users/{id}")      // GET /api/users/123
    public User getUserById(
        @PathVariable Long id) { }

    @PostMapping("/users")          // POST /api/users
    public User createUser(
        @RequestBody CreateUserRequest req) { }

    @DeleteMapping("/users/{id}")   // DELETE /api/users/123
    public void deleteUser(
        @PathVariable Long id) { }
}
```

Spring builds a route table at startup:
```text
GET  /api/users          → UserController::getUsers(page)
GET  /api/users/{id}     → UserController::getUserById(id)
POST /api/users          → UserController::createUser(body)
DELETE /api/users/{id}   → UserController::deleteUser(id)
```

---

## HandlerInterceptor (pre/post processing)

```java
public class LoggingInterceptor implements HandlerInterceptor {
    boolean preHandle(request, response, handler) {
        log("→ " + request.getMethod() + " " + request.getPath());
        return true;   // return false to abort the request
    }
    void postHandle(request, response, handler, modelAndView) {
        log("← Response ready");
    }
    void afterCompletion(request, response, handler, ex) {
        log("Request complete");
    }
}
```

---

## C Files in This Phase

| File                    | What It Simulates                          |
|-------------------------|--------------------------------------------|
| `dispatcher_servlet.c`  | Full request processing pipeline           |
| `handler_mapping.c`     | Route table: URL pattern → controller method |
| `handler_adapter.c`     | Parameter extraction + method invocation   |
| `view_resolver.c`       | Return value → JSON serialization          |

---

## Build and Run

```bash
gcc -Wall -Wextra -o dispatcher_servlet dispatcher_servlet.c && ./dispatcher_servlet
gcc -Wall -Wextra -o handler_mapping handler_mapping.c && ./handler_mapping
```

---

## Key Insight

```text
The DispatcherServlet is Spring MVC's brain.
It doesn't DO any business logic.
It only ROUTES, DELEGATES, and ASSEMBLES.

Every component is pluggable:
  - HandlerMapping  → find the right handler
  - HandlerAdapter  → invoke it
  - MessageConverter→ serialize the result

This is the Strategy + Chain of Responsibility pattern.
```
