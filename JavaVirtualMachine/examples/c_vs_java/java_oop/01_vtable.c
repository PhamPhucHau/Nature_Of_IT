/*
 * 01_vtable.c  —  Java Polymorphism & Virtual Dispatch in C
 *
 * Build:  gcc -Wall -Wextra -o 01_vtable 01_vtable.c
 * Run:    ./01_vtable
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA VIRTUAL DISPATCH WORKS:
 *
 *   Every Java class has a CLASS OBJECT in the Method Area.
 *   That class object contains a VTABLE — an array of function pointers,
 *   one slot per virtual (non-static, non-final) method.
 *
 *   Every Java OBJECT starts with a hidden "object header" that holds:
 *     - Mark Word  (GC info, lock state, hash code)  — 8 bytes
 *     - Class Pointer (klass*)  — points to the vtable   — 8 bytes
 *   Then the actual fields follow.
 *
 *   When you call:
 *     Animal a = new Dog();
 *     a.speak();             // virtual call
 *
 *   The JVM does:
 *     1. Load `a`          → get the Object reference (pointer)
 *     2. Follow class ptr  → get Dog's class object
 *     3. Index vtable      → find speak() slot
 *     4. Call that fn ptr  → Dog.speak() runs
 *
 *   This is why polymorphism works — you never call Animal.speak()
 *   when a is a Dog. The vtable always routes to the right class.
 *
 *   C++ calls this "virtual function table" — same concept.
 *   Java always uses vtable dispatch for non-final instance methods.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  VTABLE  — one per CLASS (shared by all instances)
 *  This is what lives in the JVM's Method Area
 * ═══════════════════════════════════════════════════════════ */

/* Forward-declare the Object type */
typedef struct Object Object;

/* Method signature: every virtual method takes (Object* self, ...) */
typedef void     (*Fn_speak)(Object *self);
typedef void     (*Fn_move) (Object *self);
typedef double   (*Fn_area) (Object *self);
typedef void     (*Fn_toString)(Object *self);

/* VTable for Animal hierarchy */
typedef struct {
    const char  *class_name;   /* "Dog", "Cat", etc. */
    Fn_speak     speak;
    Fn_move      move;
    Fn_toString  toString;
} AnimalVTable;

/* VTable for Shape hierarchy */
typedef struct {
    const char  *class_name;
    Fn_area      area;
    Fn_toString  toString;
} ShapeVTable;

/* ═══════════════════════════════════════════════════════════
 *  OBJECT HEADER  — the hidden 16 bytes every Java object has
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    void       *vtable_ptr;    /* klass* — points to the vtable  */
    int         identity_hash; /* cached System.identityHashCode  */
    int         gc_mark;       /* used by GC during mark phase    */
} ObjectHeader;

/* ═══════════════════════════════════════════════════════════
 *  BASE CLASS: Animal
 *  Java:  abstract class Animal { ... }
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    ObjectHeader header;   /* MUST be first — Java object layout */
    char         name[32];
    int          age;
} Animal;

/* ═══════════════════════════════════════════════════════════
 *  SUBCLASS: Dog  extends Animal
 *  Java:  class Dog extends Animal { int breed; }
 *  C key: Dog starts with the Animal struct (field inheritance)
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Animal base;    /* MUST be first — embeds Animal (= extends) */
    char   breed[32];
} Dog;

/* ═══════════════════════════════════════════════════════════
 *  SUBCLASS: Cat  extends Animal
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Animal base;
    int    indoor;
} Cat;

/* ═══════════════════════════════════════════════════════════
 *  SUBCLASS: Duck  extends Animal
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Animal base;
    int    can_fly;
} Duck;

/* ═══════════════════════════════════════════════════════════
 *  METHOD IMPLEMENTATIONS  — one set per class
 * ═══════════════════════════════════════════════════════════ */

/* Dog methods */
void dog_speak(Object *self) {
    Dog *dog = (Dog *)self;
    printf("  %s (Dog): Woof! Woof!\n", dog->base.name);
}
void dog_move(Object *self) {
    Dog *dog = (Dog *)self;
    printf("  %s (Dog): runs on 4 legs\n", dog->base.name);
}
void dog_toString(Object *self) {
    Dog *dog = (Dog *)self;
    printf("  Dog{name='%s', age=%d, breed='%s'}\n",
           dog->base.name, dog->base.age, dog->breed);
}

/* Cat methods */
void cat_speak(Object *self) {
    Cat *cat = (Cat *)self;
    printf("  %s (Cat): Meow~\n", cat->base.name);
}
void cat_move(Object *self) {
    Cat *cat = (Cat *)self;
    printf("  %s (Cat): sneaks silently\n", cat->base.name);
}
void cat_toString(Object *self) {
    Cat *cat = (Cat *)self;
    printf("  Cat{name='%s', age=%d, indoor=%s}\n",
           cat->base.name, cat->base.age, cat->indoor ? "yes" : "no");
}

/* Duck methods */
void duck_speak(Object *self) {
    Duck *duck = (Duck *)self;
    printf("  %s (Duck): Quack!\n", duck->base.name);
}
void duck_move(Object *self) {
    Duck *duck = (Duck *)self;
    if (duck->can_fly)
        printf("  %s (Duck): flies through the air\n", duck->base.name);
    else
        printf("  %s (Duck): waddles on land\n", duck->base.name);
}
void duck_toString(Object *self) {
    Duck *duck = (Duck *)self;
    printf("  Duck{name='%s', can_fly=%d}\n", duck->base.name, duck->can_fly);
}

/* ═══════════════════════════════════════════════════════════
 *  VTABLES  — one static instance per class (lives in .rodata)
 *  Java: these live in the Method Area (PermGen / Metaspace)
 * ═══════════════════════════════════════════════════════════ */
static AnimalVTable DOG_VTABLE  = { "Dog",  dog_speak,  dog_move,  dog_toString  };
static AnimalVTable CAT_VTABLE  = { "Cat",  cat_speak,  cat_move,  cat_toString  };
static AnimalVTable DUCK_VTABLE = { "Duck", duck_speak, duck_move, duck_toString };

/* ═══════════════════════════════════════════════════════════
 *  CONSTRUCTOR HELPERS  — equivalent of `new Dog(...)`
 * ═══════════════════════════════════════════════════════════ */
Dog *new_Dog(const char *name, int age, const char *breed) {
    Dog *d = calloc(1, sizeof(Dog));
    d->base.header.vtable_ptr = &DOG_VTABLE;  /* set vtable ptr */
    strncpy(d->base.name, name, 31);
    d->base.age = age;
    strncpy(d->breed, breed, 31);
    return d;
}

Cat *new_Cat(const char *name, int age, int indoor) {
    Cat *c = calloc(1, sizeof(Cat));
    c->base.header.vtable_ptr = &CAT_VTABLE;
    strncpy(c->base.name, name, 31);
    c->base.age = age;
    c->indoor = indoor;
    return c;
}

Duck *new_Duck(const char *name, int age, int can_fly) {
    Duck *d = calloc(1, sizeof(Duck));
    d->base.header.vtable_ptr = &DUCK_VTABLE;
    strncpy(d->base.name, name, 31);
    d->base.age = age;
    d->can_fly = can_fly;
    return d;
}

/* ═══════════════════════════════════════════════════════════
 *  VIRTUAL DISPATCH FUNCTIONS
 *  These simulate what the JVM does at every virtual call site
 * ═══════════════════════════════════════════════════════════ */
void virtual_speak(Animal *a) {
    AnimalVTable *vt = (AnimalVTable *)a->header.vtable_ptr;
    printf("  [vtable dispatch → %s.speak()]\n", vt->class_name);
    vt->speak((Object *)a);  /* route to correct implementation */
}

void virtual_move(Animal *a) {
    AnimalVTable *vt = (AnimalVTable *)a->header.vtable_ptr;
    vt->move((Object *)a);
}

void virtual_toString(Animal *a) {
    AnimalVTable *vt = (AnimalVTable *)a->header.vtable_ptr;
    vt->toString((Object *)a);
}

/* ═══════════════════════════════════════════════════════════
 *  SHAPE HIERARCHY — shows how interface-like dispatch works
 * ═══════════════════════════════════════════════════════════ */

typedef struct Shape Shape;

typedef struct {
    const char *class_name;
    double (*area)     (Shape *self);
    double (*perimeter)(Shape *self);
    void   (*describe) (Shape *self);
} ShapeVTableV2;

typedef struct Shape {
    ShapeVTableV2 *vtable;  /* klass ptr */
} Shape;

/* Circle */
typedef struct { Shape base; double radius; } Circle;
double circle_area(Shape *s)      { Circle *c=(Circle*)s; return 3.14159*c->radius*c->radius; }
double circle_peri(Shape *s)      { Circle *c=(Circle*)s; return 2*3.14159*c->radius; }
void   circle_desc(Shape *s)      { Circle *c=(Circle*)s; printf("  Circle(r=%.1f)\n",c->radius); }

/* Rectangle */
typedef struct { Shape base; double w, h; } Rect;
double rect_area(Shape *s)        { Rect *r=(Rect*)s; return r->w*r->h; }
double rect_peri(Shape *s)        { Rect *r=(Rect*)s; return 2*(r->w+r->h); }
void   rect_desc(Shape *s)        { Rect *r=(Rect*)s; printf("  Rect(%.1f×%.1f)\n",r->w,r->h); }

/* Triangle */
typedef struct { Shape base; double a,b,c; } Triangle;
double tri_area(Shape *s) {
    Triangle *t=(Triangle*)s;
    double sp=(t->a+t->b+t->c)/2;
    return __builtin_sqrt(sp*(sp-t->a)*(sp-t->b)*(sp-t->c));
}
double tri_peri(Shape *s) { Triangle *t=(Triangle*)s; return t->a+t->b+t->c; }
void   tri_desc(Shape *s) { Triangle *t=(Triangle*)s; printf("  Triangle(%.0f,%.0f,%.0f)\n",t->a,t->b,t->c); }

static ShapeVTableV2 CIRCLE_VT   = {"Circle",   circle_area, circle_peri, circle_desc};
static ShapeVTableV2 RECT_VT     = {"Rectangle",rect_area,   rect_peri,   rect_desc};
static ShapeVTableV2 TRIANGLE_VT = {"Triangle", tri_area,    tri_peri,    tri_desc};

Circle   *new_Circle(double r)       { Circle   *c=calloc(1,sizeof(Circle));   c->base.vtable=&CIRCLE_VT;   c->radius=r;     return c; }
Rect     *new_Rect(double w,double h){ Rect      *r=calloc(1,sizeof(Rect));     r->base.vtable=&RECT_VT;     r->w=w; r->h=h; return r; }
Triangle *new_Tri(double a,double b,double c){ Triangle *t=calloc(1,sizeof(Triangle)); t->base.vtable=&TRIANGLE_VT; t->a=a;t->b=b;t->c=c; return t; }

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_polymorphism(void) {
    printf("━━━ 1. Polymorphism via vtable ━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  Animal[] animals = {new Dog(), new Cat(), new Duck()};\n");
    printf("       for(Animal a : animals) a.speak();\n\n");

    /* Upcast — same as Java: Animal a = new Dog(); */
    Animal *animals[3];
    animals[0] = (Animal *)new_Dog("Rex",   3, "Labrador");
    animals[1] = (Animal *)new_Cat("Whiskers", 5, 1);
    animals[2] = (Animal *)new_Duck("Donald", 2, 1);

    printf("Calling speak() on each — JVM routes via vtable:\n");
    for (int i = 0; i < 3; i++)
        virtual_speak(animals[i]);

    printf("\nCalling move():\n");
    for (int i = 0; i < 3; i++)
        virtual_move(animals[i]);

    printf("\nCalling toString():\n");
    for (int i = 0; i < 3; i++)
        virtual_toString(animals[i]);

    /* Free — note: free(animals[i]) works because Dog* == Animal* (same address) */
    for (int i = 0; i < 3; i++) free(animals[i]);
    printf("\n");
}

void demo_vtable_layout(void) {
    printf("━━━ 2. VTable Memory Layout ━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Each CLASS has ONE vtable in memory (like Metaspace in JVM)\n\n");

    printf("DOG_VTABLE  @ %p\n", (void *)&DOG_VTABLE);
    printf("  .class_name = \"%s\"\n", DOG_VTABLE.class_name);
    printf("  .speak      = %p\n", (void *)DOG_VTABLE.speak);
    printf("  .move       = %p\n", (void *)DOG_VTABLE.move);
    printf("\n");

    printf("CAT_VTABLE  @ %p\n", (void *)&CAT_VTABLE);
    printf("  .speak      = %p  (different from Dog!)\n", (void *)CAT_VTABLE.speak);
    printf("\n");

    printf("Dog INSTANCE layout (sizeof=%zu):\n", sizeof(Dog));
    printf("  [0..15]  ObjectHeader (vtable_ptr + hash + gc_mark)\n");
    printf("  [16..47] name[32]\n");
    printf("  [48..51] age (int)\n");
    printf("  [52..83] breed[32]\n\n");

    Dog *d = new_Dog("Buddy", 2, "Poodle");
    printf("Dog instance @ %p\n", (void *)d);
    printf("  vtable_ptr  @ offset 0 = %p → \"%s\"\n",
           d->base.header.vtable_ptr,
           ((AnimalVTable*)d->base.header.vtable_ptr)->class_name);
    printf("  name        = \"%s\"\n", d->base.name);

    /* The magic: ANY pointer to Dog also points to the vtable */
    Animal *a = (Animal *)d;
    printf("\nUpcast Animal* @ %p (SAME address as Dog*)\n", (void *)a);
    AnimalVTable *vt = (AnimalVTable *)a->header.vtable_ptr;
    printf("  vtable_ptr still = %p → class=%s\n",
           (void *)vt, vt->class_name);
    printf("  virtual_speak() still calls Dog.speak:\n");
    virtual_speak(a);

    free(d);
    printf("\n");
}

void demo_shape_polymorphism(void) {
    printf("━━━ 3. Shape Polymorphism (interface-like) ━━━━━━━━━━━━\n");
    printf("Java:  List<Shape> shapes = List.of(new Circle(5), new Rectangle(4,6));\n");
    printf("       shapes.forEach(s -> System.out.printf(\"area=%%f%%n\", s.area()));\n\n");

    Shape *shapes[3];
    shapes[0] = (Shape *)new_Circle(5);
    shapes[1] = (Shape *)new_Rect(4, 6);
    shapes[2] = (Shape *)new_Tri(3, 4, 5);

    double total = 0;
    for (int i = 0; i < 3; i++) {
        shapes[i]->vtable->describe(shapes[i]);
        double a = shapes[i]->vtable->area(shapes[i]);
        double p = shapes[i]->vtable->perimeter(shapes[i]);
        printf("    area=%.2f  perimeter=%.2f\n", a, p);
        total += a;
    }
    printf("  Total area: %.2f\n", total);

    for (int i = 0; i < 3; i++) free(shapes[i]);
    printf("\n");
}

void demo_instanceof(void) {
    printf("━━━ 4. instanceof check (via vtable class_name) ━━━━━━━\n");
    printf("Java:  if (animal instanceof Dog d) { ... }\n\n");

    Animal *animals[4];
    animals[0] = (Animal *)new_Dog("Rex", 3, "Lab");
    animals[1] = (Animal *)new_Cat("Luna", 2, 1);
    animals[2] = (Animal *)new_Dog("Max", 5, "German Shepherd");
    animals[3] = (Animal *)new_Duck("Daffy", 4, 0);

    printf("instanceof Dog:\n");
    for (int i = 0; i < 4; i++) {
        AnimalVTable *vt = (AnimalVTable *)animals[i]->header.vtable_ptr;
        int is_dog = (strcmp(vt->class_name, "Dog") == 0);
        printf("  animals[%d] (%s): instanceof Dog = %s\n",
               i, vt->class_name, is_dog ? "true" : "false");
        if (is_dog) {
            Dog *d = (Dog *)animals[i]; /* safe downcast */
            printf("    → Dog.breed = \"%s\"\n", d->breed);
        }
    }

    for (int i = 0; i < 4; i++) free(animals[i]);
    printf("\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  01_vtable.c — Java Polymorphism / vtable in C      ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_polymorphism();
    demo_vtable_layout();
    demo_shape_polymorphism();
    demo_instanceof();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Every Java object has a hidden vtable pointer (klass*)\n");
    printf("  Virtual calls: obj→klass→vtable[slot]()  — 2 pointer chases\n");
    printf("  Static/final calls: direct call  — 0 pointer chases (faster)\n");
    printf("  Subclass embeds superclass at offset 0 — upcast is free (no cost)\n");
    printf("  instanceof: compare klass pointers or walk class hierarchy\n");
    printf("  C++ does the EXACT same thing — Java vtable IS C++ vtable\n");
    return 0;
}
