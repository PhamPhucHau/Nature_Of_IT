# 🔐 Phase 7: Spring Security

> **Spring Concept**: `SecurityFilterChain`, `AuthenticationManager`, `JWT`
> **C Equivalent**: Linked list of filter functions applied to every request

---

## Spring Security Architecture

```text
HTTP Request
  ↓
DelegatingFilterProxy
  ↓
SecurityFilterChain (list of filters in ORDER)
  ├── 1. SecurityContextPersistenceFilter  ← load/save SecurityContext
  ├── 2. CorsFilter                         ← CORS headers
  ├── 3. CsrfFilter                         ← CSRF token check
  ├── 4. LogoutFilter                       ← handle /logout
  ├── 5. UsernamePasswordAuthFilter         ← handle /login form
  ├── 6. JwtAuthenticationFilter (custom)  ← validate JWT token
  ├── 7. ExceptionTranslationFilter         ← 401/403 handling
  └── 8. FilterSecurityInterceptor          ← access control check
  ↓
DispatcherServlet → @Controller
```

Each filter can:
- **Pass through**: call `chain.doFilter(request, response)` to continue
- **Abort**: write error response directly and return (without calling next filter)

---

## Authentication Flow

```text
POST /login  { username, password }
  ↓
UsernamePasswordAuthenticationFilter
  ↓
AuthenticationManager.authenticate(token)
  ↓
UserDetailsService.loadUserByUsername(username)
  ↓ (load from DB)
BCryptPasswordEncoder.matches(rawPwd, encodedPwd)
  ↓ (if matches)
Authentication object stored in SecurityContext
  ↓
JWT token generated and returned to client
```

---

## JWT Validation Flow (per request)

```text
Request header: Authorization: Bearer eyJhbGciOiJIUzI1NiJ9...
  ↓
JwtAuthenticationFilter.doFilterInternal()
  ↓
Extract token from header
  ↓
JwtService.validateToken(token)
  ├── Decode header + payload
  ├── Verify signature (HMAC-SHA256 with secret key)
  ├── Check expiration (exp claim)
  └── Check not-before (nbf claim)
  ↓
JwtService.extractUsername(token)
  ↓
UserDetailsService.loadUserByUsername(username)
  ↓
UsernamePasswordAuthenticationToken created
  ↓
SecurityContextHolder.setAuthentication(token)
  ↓
Continue filter chain (request is now authenticated)
```

---

## JWT Structure

```text
eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJ1c2VyMSIsInJvbGVzIjpbIlVTRVIiXSwiZXhwIjoxNzA5MDAwMDAwfQ.SIG

Header (Base64):
  { "alg": "HS256", "typ": "JWT" }

Payload (Base64):
  {
    "sub": "user1",            ← subject (username)
    "roles": ["USER", "ADMIN"],← authorities
    "iat": 1709000000,         ← issued at
    "exp": 1709086400          ← expiration (24h later)
  }

Signature:
  HMAC-SHA256(base64(header) + "." + base64(payload), secretKey)
```

---

## @PreAuthorize vs @Secured

```java
// Method-level security
@PreAuthorize("hasRole('ADMIN')")           // SpEL expression
@PreAuthorize("hasAuthority('USER_READ')")  // specific authority
@PreAuthorize("#userId == authentication.principal.id")  // custom check

@Secured("ROLE_ADMIN")                      // simpler, no SpEL

@PostAuthorize("returnObject.owner == authentication.name")  // after method
```

---

## C Files in This Phase

| File                        | What It Simulates                          |
|-----------------------------|--------------------------------------------|
| `security_filter_chain.c`   | Full filter chain with JWT validation      |
| `authentication_manager.c`  | Login flow + token generation              |
| `jwt_validator.c`           | JWT structure + signature validation       |

---

## Build and Run

```bash
gcc -Wall -Wextra -o security_filter_chain security_filter_chain.c && ./security_filter_chain
gcc -Wall -Wextra -o jwt_validator jwt_validator.c && ./jwt_validator
```

---

## Key Insight

```text
Spring Security is just a chain of filters.
EVERY filter gets to see EVERY request.
EVERY filter can ABORT the chain.

SecurityContextHolder is a ThreadLocal<SecurityContext>.
It stores the current authenticated user for the duration of the request.

@PreAuthorize is an AOP advice that:
  1. Reads SecurityContextHolder.getAuthentication()
  2. Evaluates the SpEL expression
  3. Throws AccessDeniedException if denied
  4. Proceeds normally if allowed
```
