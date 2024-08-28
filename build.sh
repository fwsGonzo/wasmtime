set -e
source "/home/gonzo/github/emsdk/emsdk_env.sh"
emcc -O2 --no-entry -s ERROR_ON_UNDEFINED_SYMBOLS=0 -sSTANDALONE_WASM \
	-s "EXPORTED_RUNTIME_METHODS=['syscall_tester_0', 'syscall_tester_1', 'syscall_tester_2', 'syscall_tester_3', 'syscall_tester_4', 'syscall_tester_5', 'syscall_tester_6', 'syscall_tester_7', \
	'call_tester_0', 'call_tester_1', 'call_tester_2', 'call_tester_3', 'call_tester_4', 'call_tester_5', 'call_tester_6', 'call_tester_7']" \
	-s EXPORT_ALL=1 block.c -o block.wasm
#emcc -O2 --no-entry -s ERROR_ON_UNDEFINED_SYMBOLS=0 -sSTANDALONE_WASM block.c -o block.wat
gcc -O2 -static -pthread \
	examples/fuel.c \
	-I crates/c-api/include \
	-I crates/c-api/wasm-c-api/include \
	target/release/libwasmtime.a \
	-ldl -lm \
	-o fuel
./fuel
