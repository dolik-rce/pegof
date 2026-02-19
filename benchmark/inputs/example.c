// Enum type
enum Color {
    RED = 1,
    GREEN = 2,
    BLUE = 3
};

// Structure type
struct Point {
    int x;
    int y;
    enum Color color;
};

// Union type – can store different data types in same memory area
union Data {
    int i;
    float f;
    char c;
};

// Global variable
int global_counter = 0;

// Function declarations
int multiply(int a, int b);
int sum_array(int *arr, int len);
void update_point(struct Point *p, int dx, int dy, enum Color new_color);
void handle_union(union Data *d, int mode);

// Function definitions
int multiply(int a, int b) {
    int result = 0;
    int i = 0;
    while (i < b) {      // while loop
        result += a;     // assignment and arithmetic
        i++;
    }
    return result;
}

int sum_array(int *arr, int len) {
    int sum = 0;
    int i;
    for (i = 0; i < len; i++) {   // for loop
        sum += arr[i];
    }
    return sum;
}

void update_point(struct Point *p, int dx, int dy, enum Color new_color) {
    if (p) {                     // conditional check
        p->x += dx;
        p->y += dy;
        p->color = new_color;
        global_counter++;        // modify global variable
    }
}

void handle_union(union Data *d, int mode) {
    switch (mode) {              // switch-case conditional
        case 0:
            d->i = 42;           // assign integer
            break;
        case 1:
            d->f = 3.14f;        // assign float
            break;
        default:
            d->c = 'Z';          // assign char
            break;
    }
}

// Main-like function
int main_program() {
    int numbers[5] = {1, 2, 3, 4, 5};
    int *ptr = numbers;
    int sum = sum_array(ptr, 5);
    int product = multiply(3, 4);
    int combined = product + sum;

    struct Point p1 = {0, 0, RED};
    update_point(&p1, 2, 3, BLUE);

    union Data d;
    handle_union(&d, 1); // sets d.f = 3.14

    // Ternary conditional
    int value = (combined > 20) ? combined : 0;

    // do-while countdown example
    int countdown = 3;
    do {
        countdown--;
        global_counter += countdown;
    } while (countdown > 0);

    // return depends on struct and enum usage
    return value + p1.x + p1.y + p1.color + global_counter;
}
