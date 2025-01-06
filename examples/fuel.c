/*
Example of instantiating of the WebAssembly module and invoking its exported
function.

You can compile and run this example on Linux with:

   cargo build --release -p wasmtime-c-api
   cc examples/fuel.c \
       -I crates/c-api/include \
       target/release/libwasmtime.a \
       -lpthread -ldl -lm \
       -o fuel
   ./fuel

Note that on Windows and macOS the command will be similar, but you'll need
to tweak the `-lpthread` and such annotations.

You can also build using cmake:

mkdir build && cd build && cmake .. && cmake --build . --target wasmtime-fuel
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <wasm.h>
#include <wasmtime.h>
#include <math.h>

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);

inline struct timespec time_now();
inline long nanodiff(struct timespec start_time, struct timespec end_time);

static inline wasm_functype_t* wasm_functype_new_4_0(
  wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4
) {
  wasm_valtype_t* ps[4] = {p1, p2, p3, p4};
  wasm_valtype_vec_t params, results;
  wasm_valtype_vec_new(&params, 4, ps);
  wasm_valtype_vec_new_empty(&results);
  return wasm_functype_new(&params, &results);
}
static inline wasm_functype_t* wasm_functype_new_5_0(
  wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4, wasm_valtype_t* p5
) {
  wasm_valtype_t* ps[5] = {p1, p2, p3, p4, p5};
  wasm_valtype_vec_t params, results;
  wasm_valtype_vec_new(&params, 5, ps);
  wasm_valtype_vec_new_empty(&results);
  return wasm_functype_new(&params, &results);
}
static inline wasm_functype_t* wasm_functype_new_6_0(
  wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4, wasm_valtype_t* p5, wasm_valtype_t* p6
) {
  wasm_valtype_t* ps[6] = {p1, p2, p3, p4, p5, p6};
  wasm_valtype_vec_t params, results;
  wasm_valtype_vec_new(&params, 6, ps);
  wasm_valtype_vec_new_empty(&results);
  return wasm_functype_new(&params, &results);
}
static inline wasm_functype_t* wasm_functype_new_7_0(
  wasm_valtype_t* p1, wasm_valtype_t* p2, wasm_valtype_t* p3, wasm_valtype_t* p4, wasm_valtype_t* p5, wasm_valtype_t* p6, wasm_valtype_t* p7
) {
  wasm_valtype_t* ps[7] = {p1, p2, p3, p4, p5, p6, p7};
  wasm_valtype_vec_t params, results;
  wasm_valtype_vec_new(&params, 7, ps);
  wasm_valtype_vec_new_empty(&results);
  return wasm_functype_new(&params, &results);
}

static wasm_trap_t* is_transparent_callback(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
	results[0].kind = WASMTIME_I32;
	results[0].of.i32 = 0;
	return NULL;
}

static wasm_trap_t* api_sin_callback(
    void *env,
    wasmtime_caller_t *caller,
    const wasmtime_val_t *args,
    size_t nargs,
    wasmtime_val_t *results,
    size_t nresults
) {
	results[0].kind = WASMTIME_F32;
	results[0].of.f32 = sinf(args[0].of.f32);
	return NULL;
}

static wasm_trap_t* syscall_args_callback(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results, size_t nresults) {
	return NULL;
}

static void call_function(wasmtime_context_t *context, wasmtime_extern_t *fib, int n)
{
	wasmtime_error_t *error = NULL;
	wasm_trap_t *trap = NULL;

	error = wasmtime_context_set_fuel(context, 10000000ULL);
	if (error != NULL)
		exit_with_error("failed to set fuel", error, NULL);

    wasmtime_val_t params[3];
    params[0].kind = WASMTIME_I32;
    params[0].of.i32 = n;
    params[1].kind = WASMTIME_I32;
    params[1].of.i32 = n;
    params[2].kind = WASMTIME_I32;
    params[2].of.i32 = n;
    wasmtime_val_t results[1];
    error = wasmtime_func_call(context, &fib->of.func, params, 3, results, 1, &trap);
    if (error != NULL || trap != NULL) {
      if (trap != NULL) {
        wasmtime_trap_code_t code;
        assert(wasmtime_trap_code(trap, &code));
        assert(code == WASMTIME_TRAP_CODE_OUT_OF_FUEL);
      }
      fprintf(stderr, "Exhausted fuel computing\n");
      exit(1);
    }
}

static void call_function_args(wasmtime_context_t *context, wasmtime_extern_t *func, wasmtime_val_t *params, size_t nargs)
{
	wasmtime_error_t *error = NULL;
	wasm_trap_t *trap = NULL;

	error = wasmtime_context_set_fuel(context, 10000000ULL);
	if (error != NULL)
		exit_with_error("failed to set fuel", error, NULL);

	wasmtime_val_t results[1];
	error = wasmtime_func_call(context, &func->of.func, params, nargs, results, 0, &trap);
	if (error != NULL || trap != NULL) {
	  if (trap != NULL) {
		wasmtime_trap_code_t code;
		assert(wasmtime_trap_code(trap, &code));
		assert(code == WASMTIME_TRAP_CODE_OUT_OF_FUEL);
	  }
	  fprintf(stderr, "Exhausted fuel computing\n");
	  exit(1);
	}
}

int main() {
  wasmtime_error_t *error = NULL;

  wasm_config_t *config = wasm_config_new();
  assert(config != NULL);
  wasmtime_config_consume_fuel_set(config, true);

  // Create an *engine*, which is a compilation context, with our configured
  // options.
  wasm_engine_t *engine = wasm_engine_new_with_config(config);
  assert(engine != NULL);
  wasmtime_store_t *store = wasmtime_store_new(engine, NULL, NULL);
  assert(store != NULL);
  wasmtime_context_t *context = wasmtime_store_context(store);

  // Load our input file to parse it next
  FILE* file = fopen("block.wasm", "r");
  if (!file) {
    printf("> Error loading file!\n");
    return 1;
  }
  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0L, SEEK_SET);
  wasm_byte_vec_t binary;
  wasm_byte_vec_new_uninitialized(&binary, file_size);
  if (fread(binary.data, file_size, 1, file) != 1) {
    printf("> Error loading module!\n");
    return 1;
  }
  fclose(file);

  // Compile.
  printf("Compiling module...\n");
  wasmtime_module_t *module = NULL;
  error = wasmtime_module_new(engine, (uint8_t*) binary.data, binary.size, &module);
  if (!module)
    exit_with_error("failed to compile module", error, NULL);
  wasm_byte_vec_delete(&binary);

  wasm_functype_t *hello_ty = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
  wasmtime_func_t is_transparent;
  wasmtime_func_new(context, hello_ty, is_transparent_callback, NULL, NULL, &is_transparent);

  wasm_functype_t *api_sin_type = wasm_functype_new_1_1(wasm_valtype_new_f32(), wasm_valtype_new_f32());
  wasmtime_func_t api_sin_func;
  wasmtime_func_new(context, api_sin_type, api_sin_callback, NULL, NULL, &api_sin_func);

  wasm_functype_t* call_args[8];
  call_args[0] = wasm_functype_new_0_0();
  call_args[1] = wasm_functype_new_1_0(wasm_valtype_new_i32());
  call_args[2] = wasm_functype_new_2_0(wasm_valtype_new_i32(), wasm_valtype_new_i32());
  call_args[3] = wasm_functype_new_3_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  call_args[4] = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  call_args[5] = wasm_functype_new_5_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  call_args[6] = wasm_functype_new_6_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  call_args[7] = wasm_functype_new_7_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());

  wasmtime_func_t call_functions[8];
  for (int i = 0; i < 8; i++)
  {
	  wasmtime_func_new(context, call_args[i], syscall_args_callback, NULL, NULL, &call_functions[i]);
  }

  wasm_functype_t* syscall_args[8];
  syscall_args[0] = wasm_functype_new_0_0();
  syscall_args[1] = wasm_functype_new_1_0(wasm_valtype_new_i32());
  syscall_args[2] = wasm_functype_new_2_0(wasm_valtype_new_i32(), wasm_valtype_new_i32());
  syscall_args[3] = wasm_functype_new_3_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  syscall_args[4] = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  syscall_args[5] = wasm_functype_new_5_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  syscall_args[6] = wasm_functype_new_6_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
  syscall_args[7] = wasm_functype_new_7_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());

  wasmtime_func_t syscall_functions[8];
  for (int i = 0; i < 8; i++)
  {
	  wasmtime_func_new(context, syscall_args[i], syscall_args_callback, NULL, NULL, &syscall_functions[i]);
  }

  wasmtime_extern_t imports[2 + 8];
  imports[0].kind = WASMTIME_EXTERN_FUNC;
  imports[0].of.func = is_transparent;
  imports[1].kind = WASMTIME_EXTERN_FUNC;
  imports[1].of.func = api_sin_func;
  for (int i = 0; i < 8; i++)
  {
	  imports[2 + i].kind = WASMTIME_EXTERN_FUNC;
	  imports[2 + i].of.func = syscall_functions[i];
  }

  wasm_trap_t *trap = NULL;
  wasmtime_instance_t instance;
  error = wasmtime_instance_new(context, module, &imports[0], 2 + 8, &instance, &trap);
  if (error != NULL || trap != NULL)
    exit_with_error("failed to instantiate", error, trap);

  // Lookup our `fibonacci` export function
  wasmtime_extern_t fib;
  bool ok = wasmtime_instance_export_get(context, &instance, "stdRainbowBlockColor", strlen("stdRainbowBlockColor"), &fib);
  assert(ok);
  assert(fib.kind == WASMTIME_EXTERN_FUNC);

  wasmtime_extern_t fib2;
  ok = wasmtime_instance_export_get(context, &instance, "stdBenchVisibility2", strlen("stdBenchVisibility2"), &fib2);
  assert(ok);

  // Lookup our `call_tester_0-8` export functions
  wasmtime_extern_t call_tester[9];
  for (int i = 0; i < 9; i++)
  {
	  char name[32];
	  sprintf(name, "call_tester_%d", i);
	  ok = wasmtime_instance_export_get(context, &instance, name, strlen(name), &call_tester[i]);
	  assert(ok);
  }

  // Lookup our `syscall_tester_0-7` export functions
  wasmtime_extern_t syscall_tester[8];
  for (int i = 0; i < 8; i++)
  {
	  char name[32];
	  sprintf(name, "syscall_tester_%d", i);
	  ok = wasmtime_instance_export_get(context, &instance, name, strlen(name), &syscall_tester[i]);
	  assert(ok);
  }

  static const uint64_t SAMPLES = 30;
  static const uint64_t TIMES_1 = 2000;
  static const uint64_t TIMES_2 = 1;

//#define CLEAR_CACHE
#ifdef CLEAR_CACHE
  const size_t dataSize = 1024*1024*64ull;
  char *data = (char *)mmap(NULL, dataSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

  printf("Running benchmark with %lu iterations\n", TIMES_1);
  call_function(context, &fib, 8);

  for (int N = 0; N < 8; N++) {
	printf("N = %d\n", N);
  for (size_t i = 0; i < SAMPLES; i++)
  {
	uint64_t total = 0;

	// Call it repeatedly until it fails
	for (int n = 0; n < TIMES_1; n++)
	{
#ifdef CLEAR_CACHE
		for (size_t i = 0; i < dataSize; i += 64)
			data[i] = 1;
#endif
		struct timespec t0 = time_now();
		asm ("" ::: "memory");
		wasmtime_val_t args[8];
		for (int i = 0; i < N; i++)
		{
			args[i].kind = WASMTIME_I32;
			args[i].of.i32 = i;
		}
		call_function_args(context, &call_tester[N], args, N);
		//call_function_args(context, &syscall_tester[N], args, N);
		asm ("" ::: "memory");
		struct timespec t1 = time_now();
		total += nanodiff(t0, t1);
	}

	printf("Nanodiff: %lu\n", total / TIMES_1);
  }
  }

  // Clean up after ourselves at this point
  wasmtime_module_delete(module);
  wasmtime_store_delete(store);
  wasm_engine_delete(engine);
  return 0;
}

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap) {
  fprintf(stderr, "error: %s\n", message);
  wasm_byte_vec_t error_message;
  if (error != NULL) {
    wasmtime_error_message(error, &error_message);
  } else {
    wasm_trap_message(trap, &error_message);
  }
  fprintf(stderr, "%.*s\n", (int)error_message.size, error_message.data);
  wasm_byte_vec_delete(&error_message);
  exit(1);
}

struct timespec time_now()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t;
}
long nanodiff(struct timespec start_time, struct timespec end_time)
{
	return (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + (end_time.tv_nsec - start_time.tv_nsec);
}
