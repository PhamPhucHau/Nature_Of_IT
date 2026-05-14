# MiniSpringBoot Implementation Guide

## Overview

This guide walks through building a mini Spring Boot framework from scratch using Java Core.

---

## Project Structure

```
MiniSpringBoot/
├── src/
│   ├── core/
│   │   ├── IoCContainer.java
│   │   ├── BeanFactory.java
│   │   ├── ReflectionEngine.java
│   │   └── AnnotationScanner.java
│   ├── web/
│   │   ├── HttpServer.java
│   │   ├── Router.java
│   │   ├── Dispatcher.java
│   │   └── ControllerEngine.java
│   ├── json/
│   │   └── JsonSerializer.java
│   ├── security/
│   │   ├── JwtUtil.java
│   │   └── AuthFilter.java
│   ├── config/
│   │   └── PropertiesLoader.java
│   └── boot/
│       └── MiniSpringApplication.java
├── examples/
│   ├── User.java
│   ├── UserController.java
│   └── UserService.java
└── test/
    ├── test_core/
    ├── test_web/
    └── test_integration/
```

---

## Phase 1: HTTP Server

### HttpServer.java

```java
package minispringboot.web;

import java.io.*;
import java.net.*;

public class HttpServer {
    private int port;
    private Router router;
    
    public HttpServer(int port, Router router) {
        this.port = port;
        this.router = router;
    }
    
    public void start() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server started on port " + port);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                new Thread(() -> handleClient(clientSocket)).start();
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
    
    private void handleClient(Socket clientSocket) {
        try (
            BufferedReader in = new BufferedReader(
                new InputStreamReader(clientSocket.getInputStream())
            );
            PrintWriter out = new PrintWriter(
                new OutputStreamWriter(clientSocket.getOutputStream())
            )
        ) {
            // Parse HTTP request
            String requestLine = in.readLine();
            if (requestLine == null) return;
            
            String[] parts = requestLine.split(" ");
            String method = parts[0];
            String path = parts[1];
            
            // Skip headers
            String line;
            while ((line = in.readLine()) != null && !line.isEmpty()) {
                // Process headers
            }
            
            // Find route and invoke controller
            String response = router.dispatch(method, path);
            
            // Send response
            out.println("HTTP/1.1 200 OK");
            out.println("Content-Type: text/plain");
            out.println("Content-Length: " + response.length());
            out.println();
            out.print(response);
            out.flush();
            
        } catch (IOException e) {
            System.err.println("Client handling error: " + e.getMessage());
        } finally {
            try {
                clientSocket.close();
            } catch (IOException e) {
                // Ignore
            }
        }
    }
}
```

---

## Phase 2: Router

### Router.java

```java
package minispringboot.web;

import java.util.*;

public class Router {
    private Map<String, Map<String, ControllerMethod>> routes;
    
    public Router() {
        routes = new HashMap<>();
    }
    
    public void register(String method, String path, ControllerMethod methodInfo) {
        routes.computeIfAbsent(path, k -> new HashMap<>())
              .put(method.toUpperCase(), methodInfo);
    }
    
    public String dispatch(String method, String path) {
        Map<String, ControllerMethod> pathRoutes = routes.get(path);
        if (pathRoutes == null) {
            return "404 Not Found";
        }
        
        ControllerMethod controllerMethod = pathRoutes.get(method.toUpperCase());
        if (controllerMethod == null) {
            return "405 Method Not Allowed";
        }
        
        try {
            return controllerMethod.invoke();
        } catch (Exception e) {
            return "500 Internal Server Error: " + e.getMessage();
        }
    }
}
```

### ControllerMethod.java

```java
package minispringboot.web;

import java.lang.reflect.Method;

public class ControllerMethod {
    private Object controller;
    private Method method;
    
    public ControllerMethod(Object controller, Method method) {
        this.controller = controller;
        this.method = method;
    }
    
    public String invoke() throws Exception {
        Object result = method.invoke(controller);
        return result != null ? result.toString() : "";
    }
}
```

---

## Phase 3: Annotations

### Controller.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Controller {
}
```

### RequestMapping.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface RequestMapping {
    String value() default "";
}
```

### GetMapping.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface GetMapping {
    String value();
}
```

### PostMapping.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface PostMapping {
    String value();
}
```

### Autowired.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface Autowired {
}
```

---

## Phase 4: IoC Container

### IoCContainer.java

```java
package minispringboot.core;

import minispringboot.core.annotations.*;

import java.lang.reflect.*;
import java.util.*;

public class IoCContainer {
    private Map<Class<?>, Object> beans;
    private List<Class<?>> scanPackages;
    
    public IoCContainer() {
        this.beans = new HashMap<>();
        this.scanPackages = new ArrayList<>();
    }
    
    public void addScanPackage(String packageName) {
        try {
            Class<?>[] classes = findClasses(packageName);
            for (Class<?> clazz : classes) {
                if (clazz.isAnnotationPresent(Controller.class) || 
                    clazz.isAnnotationPresent(Service.class)) {
                    Object instance = createBean(clazz);
                    beans.put(clazz, instance);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private Class<?>[] findClasses(String packageName) throws Exception {
        // Simplified class finding - in production use reflection libraries
        return new Class<?>[0];
    }
    
    private Object createBean(Class<?> clazz) throws Exception {
        Object instance = clazz.getDeclaredConstructor().newInstance();
        injectDependencies(instance);
        return instance;
    }
    
    private void injectDependencies(Object instance) throws Exception {
        Class<?> clazz = instance.getClass();
        
        // Find fields with @Autowired
        for (Field field : clazz.getDeclaredFields()) {
            if (field.isAnnotationPresent(Autowired.class)) {
                field.setAccessible(true);
                Object dependency = beans.get(field.getType());
                if (dependency != null) {
                    field.set(instance, dependency);
                }
            }
        }
    }
    
    public <T> T getBean(Class<T> clazz) {
        return (T) beans.get(clazz);
    }
    
    public void scanPackages(String... packages) {
        for (String pkg : packages) {
            addScanPackage(pkg);
        }
    }
}
```

### Service.java

```java
package minispringboot.core.annotations;

import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Service {
}
```

---

## Phase 5: Annotation Scanner

### AnnotationScanner.java

```java
package minispringboot.core;

import minispringboot.core.annotations.*;

import java.lang.reflect.*;
import java.util.*;

public class AnnotationScanner {
    
    public static List<ControllerMapping> scanControllers(Class<?> clazz) {
        List<ControllerMapping> mappings = new ArrayList<>();
        
        if (!clazz.isAnnotationPresent(Controller.class)) {
            return mappings;
        }
        
        RequestMapping classMapping = clazz.getAnnotation(RequestMapping.class);
        String basePath = classMapping != null ? classMapping.value() : "";
        
        for (Method method : clazz.getDeclaredMethods()) {
            ControllerMapping mapping = new ControllerMapping();
            mapping.setClassPath(basePath);
            mapping.setMethod(method);
            
            if (method.isAnnotationPresent(GetMapping.class)) {
                GetMapping getMapping = method.getAnnotation(GetMapping.class);
                mapping.setPath(basePath + getMapping.value());
                mapping.setHttpMethod("GET");
            } else if (method.isAnnotationPresent(PostMapping.class)) {
                PostMapping postMapping = method.getAnnotation(PostMapping.class);
                mapping.setPath(basePath + postMapping.value());
                mapping.setHttpMethod("POST");
            }
            
            if (mapping.getPath() != null) {
                mappings.add(mapping);
            }
        }
        
        return mappings;
    }
}
```

### ControllerMapping.java

```java
package minispringboot.core;

import java.lang.reflect.Method;

public class ControllerMapping {
    private String classPath;
    private String path;
    private String httpMethod;
    private Method method;
    
    // Getters and setters
    public String getClassPath() { return classPath; }
    public void setClassPath(String classPath) { this.classPath = classPath; }
    
    public String getPath() { return path; }
    public void setPath(String path) { this.path = path; }
    
    public String getHttpMethod() { return httpMethod; }
    public void setHttpMethod(String httpMethod) { this.httpMethod = httpMethod; }
    
    public Method getMethod() { return method; }
    public void setMethod(Method method) { this.method = method; }
}
```

---

## Phase 6: JSON Serializer

### JsonSerializer.java

```java
package minispringboot.json;

import java.lang.reflect.*;
import java.util.*;

public class JsonSerializer {
    
    public String serialize(Object obj) throws Exception {
        if (obj == null) return "null";
        
        Class<?> clazz = obj.getClass();
        
        // Handle primitives and strings
        if (isPrimitiveOrString(clazz)) {
            return "\"" + obj.toString() + "\"";
        }
        
        // Handle collections
        if (obj instanceof Collection) {
            return serializeCollection((Collection<?>) obj);
        }
        
        if (obj instanceof Map) {
            return serializeMap((Map<?, ?>) obj);
        }
        
        // Handle objects
        return serializeObject(obj);
    }
    
    private boolean isPrimitiveOrString(Class<?> clazz) {
        return clazz.isPrimitive() || 
               clazz == String.class ||
               clazz == Integer.class ||
               clazz == Long.class ||
               clazz == Double.class ||
               clazz == Boolean.class;
    }
    
    private String serializeCollection(Collection<?> collection) throws Exception {
        StringBuilder sb = new StringBuilder("[");
        boolean first = true;
        
        for (Object item : collection) {
            if (!first) sb.append(",");
            sb.append(serialize(item));
            first = false;
        }
        
        sb.append("]");
        return sb.toString();
    }
    
    private String serializeMap(Map<?, ?> map) throws Exception {
        StringBuilder sb = new StringBuilder("{");
        boolean first = true;
        
        for (Map.Entry<?, ?> entry : map.entrySet()) {
            if (!first) sb.append(",");
            sb.append("\"").append(entry.getKey()).append("\":");
            sb.append(serialize(entry.getValue()));
            first = false;
        }
        
        sb.append("}");
        return sb.toString();
    }
    
    private String serializeObject(Object obj) throws Exception {
        StringBuilder sb = new StringBuilder("{");
        boolean first = true;
        
        for (Field field : obj.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            Object value = field.get(obj);
            
            if (!first) sb.append(",");
            sb.append("\"").append(field.getName()).append("\":");
            sb.append(serialize(value));
            first = false;
        }
        
        sb.append("}");
        return sb.toString();
    }
}
```

---

## Phase 7: Dispatcher

### Dispatcher.java

```java
package minispringboot.web;

import minispringboot.core.*;
import minispringboot.core.annotations.*;

import java.util.*;

public class Dispatcher {
    private IoCContainer container;
    private Router router;
    
    public Dispatcher(IoCContainer container) {
        this.container = container;
        this.router = new Router();
    }
    
    public void initialize() throws Exception {
        // Scan for controllers
        scanControllers();
    }
    
    private void scanControllers() throws Exception {
        // In a real implementation, scan all classes in packages
        // For now, manually register known controllers
        
        // Example: Register UserController
        // container.scanPackages("com.example");
        
        // After scanning, register routes
        // registerRoutes();
    }
    
    public void registerRoutes() throws Exception {
        // This would be called after IoC container is populated
        // For now, manual registration example:
        
        // UserController userController = container.getBean(UserController.class);
        // Method getUsersMethod = UserController.class.getMethod("getUsers");
        // router.register("GET", "/api/users", new ControllerMethod(userController, getUsersMethod));
    }
}
```

---

## Phase 8: Main Application

### MiniSpringApplication.java

```java
package minispringboot.boot;

import minispringboot.core.IoCContainer;
import minispringboot.web.HttpServer;
import minispringboot.web.Router;

public class MiniSpringApplication {
    
    public static void run(Class<?> mainClass, String... args) {
        try {
            // Parse port from args
            int port = 8080;
            for (String arg : args) {
                if (arg.startsWith("--server.port=")) {
                    port = Integer.parseInt(arg.substring("--server.port=".length()));
                }
            }
            
            // Create IoC container
            IoCContainer container = new IoCContainer();
            
            // Scan packages
            container.scanPackages("com.example");
            
            // Create router
            Router router = new Router();
            
            // Create and start server
            HttpServer server = new HttpServer(port, router);
            server.start();
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static void run(Class<?> mainClass, int port) {
        run(mainClass, "--server.port=" + port);
    }
}
```

---

## Phase 9: Example Application

### User.java

```java
package com.example.model;

public class User {
    private Long id;
    private String name;
    private String email;
    
    public User() {}
    
    public User(Long id, String name, String email) {
        this.id = id;
        this.name = name;
        this.email = email;
    }
    
    // Getters and setters
    public Long getId() { return id; }
    public void setId(Long id) { this.id = id; }
    
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }
    
    public String getEmail() { return email; }
    public void setEmail(String email) { this.email = email; }
}
```

### UserService.java

```java
package com.example.service;

import minispringboot.core.annotations.Service;
import com.example.model.User;

import java.util.*;

@Service
public class UserService {
    private Map<Long, User> users = new HashMap<>();
    
    public UserService() {
        users.put(1L, new User(1L, "John Doe", "john@example.com"));
        users.put(2L, new User(2L, "Jane Smith", "jane@example.com"));
    }
    
    public User getUser(Long id) {
        return users.get(id);
    }
    
    public List<User> getAllUsers() {
        return new ArrayList<>(users.values());
    }
}
```

### UserController.java

```java
package com.example.controller;

import minispringboot.core.annotations.*;
import minispringboot.json.JsonSerializer;
import com.example.model.User;
import com.example.service.UserService;

import java.util.*;

@Controller
@RequestMapping("/api/users")
public class UserController {
    
    @Autowired
    private UserService userService;
    
    @GetMapping("/{id}")
    public User getUser(Long id) {
        return userService.getUser(id);
    }
    
    @GetMapping
    public List<User> getAllUsers() {
        return userService.getAllUsers();
    }
}
```

### Main.java

```java
package com.example;

import minispringboot.boot.MiniSpringApplication;

public class Main {
    public static void main(String[] args) {
        MiniSpringApplication.run(Main.class, 8080);
    }
}
```

---

## Build and Run

### Build
```bash
javac -d out src/**/*.java
```

### Run
```bash
java -cp out com.example.Main
```

### Test
```bash
curl http://localhost:8080/api/users
curl http://localhost:8080/api/users/1
```

---

## Next Steps

1. Add more annotations (@PutMapping, @DeleteMapping, etc.)
2. Implement request body parsing
3. Add JSON deserialization
4. Implement middleware/filter system
5. Add security features
6. Implement auto-configuration
7. Add embedded server support