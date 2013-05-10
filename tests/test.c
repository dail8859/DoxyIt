// --- Test function names ---

void ()	// fail

void f ()

void func_1(void)

// --- Test return values ---

int func()

const char *func()

const char* func()

const char ** func()

// --- Test parameter values ---

int func(int x)

float func(...)

int printf( const char * format , ... )

float func(char *p, char** str, double x)

// --- Test multiline functions ---

const char * 
func(
	int one,
	double two,
	char three)

// --- Test indentation

    const char* func()
	
	float func(char *p, char** str, double x)
