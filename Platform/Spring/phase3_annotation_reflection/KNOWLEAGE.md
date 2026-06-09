# 🔍 Phase 3: Annotation Scanner & Reflection Engine

> **Spring Concept**: Classpath scanning, `@ComponentScan`, `ReflectionUtils`
> **C Equivalent**: Walking struct metadata tables + function pointer dispatch

---

## What Does Spring Scan?

When Spring starts, it scans the classpath for classes annotated with:

```text
@Component          ← generic bean
@Service            ← business layer bean
@Repository         ← data access bean
@Controller         ← web layer bean
@RestController     ← web + @ResponseBody
@Configuration      ← config class with @Bean methods
```

This is triggered by:
```java
@SpringBootApplication    // includes @ComponentScan
@ComponentScan("com.example")  // explicit package
```

---

## How Classpath Scanning Works Internally

```text
ComponentScanAnnotationParser
  → ClassPathBeanDefinitionScanner.doScan()
    → findCandidateComponents(basePackage)
      → ClassPathScanningCandidateComponentProvider
        1. Convert package name → file path
           "com.example" → "com/example/"
        2. Find all .class files in that path
        3. For each class:
           a. Load class bytes (ClassLoader)
           b. Read annotation metadata (ASM bytecode reader)
           c. Check if annotated with @Component or @Component meta-annotated
           d. If yes → create BeanDefinition
           e. Register in BeanDefinitionRegistry
```

Note: Spring uses ASM to read annotations WITHOUT loading the class into JVM.
This is much faster than using `Class.forName()` and avoids side effects.

---

## How Reflection Works in Spring

Spring uses Java Reflection extensively for:

| Operation | Java API | Used By |
|-----------|----------|---------|
| Invoke controller method | `Method.invoke(obj, args)` | DispatcherServlet |
| Read field value | `Field.get(obj)` | JSON serializers |
| Write field value | `Field.set(obj, value)` | @Autowired injection |
| Get annotations | `Method.getAnnotation(...)` | Handler mapping |
| Create instance | `Constructor.newInstance(args)` | Bean creation |
| Get all methods | `Class.getDeclaredMethods()` | AOP proxy building |

---

## Proxy Factory — JDK Proxy vs CGLIB

Spring uses two proxy mechanisms:

### JDK Dynamic Proxy
```text
Used when: target class implements at least one interface
Creates:   a new class implementing the same interfaces
           with an InvocationHandler that intercepts every call

Java:
  Proxy.newProxyInstance(
      classLoader,
      new Class[]{UserService.class},
      (proxy, method, args) -> {
          // before advice
          Object result = method.invoke(target, args);
          // after advice
          return result;
      }
  );
```

### CGLIB Proxy
```text
Used when: target class does NOT implement an interface
Creates:   a subclass of the target class using bytecode generation
           overrides all methods to add interceptor chain

Spring Boot default since 5.x: uses CGLIB even with interfaces
(configurable via spring.aop.proxy-target-class=false)
```

---

## C Files in This Phase

| File                   | What It Simulates                          |
|------------------------|--------------------------------------------|
| `annotation_scanner.c` | Classpath scanning: find annotated classes |
| `reflection_engine.c`  | Method/field invoke via function pointers  |
| `proxy_factory.c`      | JDK/CGLIB proxy simulation with wrappers   |

---

## Build and Run

```bash
gcc -Wall -Wextra -o annotation_scanner annotation_scanner.c && ./annotation_scanner
gcc -Wall -Wextra -o reflection_engine reflection_engine.c && ./reflection_engine
gcc -Wall -Wextra -o proxy_factory proxy_factory.c && ./proxy_factory
```

---

## Key Insight

```text
Java Reflection:
    Class.getDeclaredMethods()
    → returns Method[] objects
    → each Method has: name, parameter types, return type, annotations

C equivalent:
    A static table of MethodDescriptor structs:
    {
        .name       = "getUsers",
        .fn_ptr     = (void*)user_controller_get_users,
        .annotation = ANNOTATION_GET_MAPPING,
        .path       = "/users"
    }

Spring builds these tables at startup.
Everything after that is table lookup.
```
