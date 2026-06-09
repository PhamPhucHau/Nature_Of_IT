# 🚀 Spring Boot Full Startup Sequence

> Read this file ONCE before starting Phase 1.
> It is the map. The phases are the territory.

---

## The One-Line Summary

```text
SpringApplication.run() = prepare environment → bootstrap context → refresh context → start server → notify listeners → ready
```

---

## Annotated Source Trace

Every line below corresponds to real Spring Boot source code.

```text
╔══════════════════════════════════════════════════════════════════════╗
║  public static void main(String[] args) {                           ║
║      SpringApplication.run(MyApp.class, args);                      ║
║  }                                                                   ║
╚══════════════════════════════════════════════════════════════════════╝

SpringApplication constructor:
  ├─ this.primarySources = Set.of(MyApp.class)
  ├─ this.webApplicationType = WebApplicationType.deduceFromClasspath()
  │     → if spring-webmvc on classpath: SERVLET
  │     → if spring-webflux on classpath: REACTIVE
  │     → otherwise: NONE
  ├─ this.bootstrapRegistryInitializers = loadFactories(BootstrapRegistryInitializer)
  ├─ this.initializers = loadFactories(ApplicationContextInitializer)
  │     [reads from META-INF/spring.factories]
  ├─ this.listeners = loadFactories(ApplicationListener)
  │     [reads from META-INF/spring.factories]
  └─ this.mainApplicationClass = deduceMainApplicationClass()
        [walks stack trace to find main()]

run(args):
  ├─ 01. bootstrapContext = createBootstrapContext()
  │
  ├─ 02. headless mode: java.awt.headless = true
  │
  ├─ 03. springApplicationRunListeners = getRunListeners(args)
  │       [loads SpringApplicationRunListener from spring.factories]
  │       [default: EventPublishingRunListener]
  │
  ├─ 04. listeners.starting(bootstrapContext, mainApplicationClass)
  │       → publishes: ApplicationStartingEvent
  │         (fired BEFORE environment is prepared — very early)
  │
  ├─ 05. applicationArguments = new DefaultApplicationArguments(args)
  │
  ├─ 06. environment = prepareEnvironment(listeners, bootstrapContext, appArgs)
  │       ├─ create StandardServletEnvironment
  │       ├─ configure property sources:
  │       │    commandLineArgs → systemProperties → systemEnvironment
  │       ├─ bind spring.profiles.active
  │       ├─ attach ConfigDataEnvironmentPostProcessor
  │       │    → loads application.yml / application.properties
  │       │    → loads application-{profile}.yml
  │       └─ publishes: ApplicationEnvironmentPreparedEvent
  │            (your beans can listen here to inspect environment)
  │
  ├─ 07. configureIgnoreBeanInfo(environment)
  │
  ├─ 08. printBanner(environment)
  │       → reads spring.banner.location or default ASCII art
  │       → prints "Spring Boot" + version to console
  │
  ├─ 09. context = createApplicationContext()
  │       → if SERVLET: AnnotationConfigServletWebServerApplicationContext
  │       → if REACTIVE: AnnotationConfigReactiveWebServerApplicationContext
  │       → if NONE: AnnotationConfigApplicationContext
  │
  ├─ 10. context.setApplicationStartup(applicationStartup)
  │         [tracks startup steps for /actuator/startup]
  │
  ├─ 11. prepareContext(bootstrapContext, context, environment, listeners, appArgs, banner)
  │       ├─ context.setEnvironment(environment)
  │       ├─ postProcessApplicationContext(context)
  │       │    → registers BeanNameGenerator, ResourceLoader, ConversionService
  │       ├─ applyInitializers(context)
  │       │    → runs all ApplicationContextInitializer.initialize()
  │       ├─ listeners.contextPrepared(context)
  │       │    → publishes: ApplicationContextInitializedEvent
  │       ├─ bootstrapContext.close(context)
  │       ├─ load(context, sources)
  │       │    → registers MyApp.class as @Configuration source
  │       └─ listeners.contextLoaded(context)
  │            → publishes: ApplicationPreparedEvent
  │
  ├─ 12. refreshContext(context)   ◄━━━━━━━━━━━━━━━ THE BIG STEP
  │       → AbstractApplicationContext.refresh()   (see Spring/ folder)
  │       │
  │       During refresh():
  │       ├─ prepareRefresh()
  │       ├─ obtainFreshBeanFactory()
  │       │    → registers all BeanDefinitions from @SpringBootApplication class
  │       │
  │       ├─ prepareBeanFactory(beanFactory)
  │       │
  │       ├─ postProcessBeanFactory(beanFactory)
  │       │    → for web: registers ServletContext-aware beans
  │       │
  │       ├─ invokeBeanFactoryPostProcessors(beanFactory)   ◄── KEY STEP
  │       │    → ConfigurationClassPostProcessor runs:
  │       │         1. Parses @SpringBootApplication class
  │       │         2. Processes @ComponentScan → registers YOUR beans
  │       │         3. Processes @EnableAutoConfiguration:
  │       │              a. Loads AutoConfiguration.imports (150+ classes)
  │       │              b. For each class, evaluates @Conditional annotations
  │       │              c. Passing classes are registered as @Configuration
  │       │         4. Processes all @Configuration classes' @Bean methods
  │       │
  │       ├─ registerBeanPostProcessors(beanFactory)
  │       │    → AutowiredAnnotationBeanPostProcessor (handles @Autowired)
  │       │    → CommonAnnotationBeanPostProcessor (handles @PostConstruct)
  │       │
  │       ├─ initMessageSource()
  │       ├─ initApplicationEventMulticaster()
  │       │
  │       ├─ onRefresh()   ◄── EMBEDDED SERVER STARTS HERE
  │       │    → ServletWebServerApplicationContext.createWebServer()
  │       │         → TomcatServletWebServerFactory.getWebServer()
  │       │              → new Tomcat()
  │       │              → tomcat.getConnector().setPort(8080)
  │       │              → addContext(contextPath, docBase)
  │       │              → registers DispatcherServletRegistrationBean
  │       │         → webServer.start()  ← Tomcat now listening on 8080
  │       │
  │       ├─ registerListeners()
  │       ├─ finishBeanFactoryInitialization(beanFactory)
  │       │    → instantiates ALL non-lazy singleton beans
  │       │    → runs @PostConstruct on each
  │       │
  │       └─ finishRefresh()
  │            → publishes: ContextRefreshedEvent
  │            → webServer.start() (if deferred)
  │            → publishes: ServletWebServerInitializedEvent
  │
  ├─ 13. afterRefresh(context, applicationArguments)
  │       [hook for subclasses, currently empty in Boot]
  │
  ├─ 14. Duration timeTakenToStartup = ...
  │       started = true
  │       listeners.started(context, timeTakenToStartup)
  │       → publishes: ApplicationStartedEvent
  │
  ├─ 15. callRunners(context, applicationArguments)
  │       → runs all ApplicationRunner beans (in @Order)
  │       → runs all CommandLineRunner beans (in @Order)
  │         [use these for: data initialization, cache warmup, etc.]
  │
  └─ 16. listeners.ready(context, timeTakenToReady)
          → publishes: ApplicationReadyEvent
          → logs: "Started MyApp in 2.341 seconds (process running for 2.876)"
```

---

## Events Timeline

```text
Time →

ApplicationStartingEvent            (before ANYTHING is ready)
ApplicationEnvironmentPreparedEvent (environment loaded, no beans yet)
ApplicationContextInitializedEvent  (context created, no beans yet)
ApplicationPreparedEvent            (beans defined, not yet created)
ContextRefreshedEvent               (all beans created, server started)
ApplicationStartedEvent             (runners not yet run)
ApplicationReadyEvent               (fully ready to serve traffic)

ApplicationFailedEvent              (published if ANYTHING above fails)
```

---

## Key C Files Mapping

| Spring Boot Step | C File |
|-----------------|--------|
| Startup lifecycle | `phase1_spring_application/spring_application.c` |
| Auto-config evaluation | `phase2_auto_configuration/conditional_engine.c` |
| Property binding | `phase3_config_properties/property_binder.c` |
| Embedded server | `phase4_embedded_server/embedded_tomcat.c` |
| Event publishing | `phase1_spring_application/startup_events.c` |
| Shutdown | `phase8_graceful_shutdown/shutdown_hook.c` |

---

## The 3 Most Important Lines in Spring Boot

```java
// Line 1: everything starts here
SpringApplication.run(MyApp.class, args);

// Line 2: the file that makes auto-config work
// META-INF/spring/org.springframework.boot.autoconfigure.AutoConfiguration.imports

// Line 3: the annotation that ties it all together
@SpringBootApplication
```

If you understand these three fully, you understand Spring Boot.
The C files in this folder build each piece from scratch.
