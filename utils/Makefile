CC = gcc
CFLAGS = -Wall -O3

TARGETS = op_tables disassembler

all: $(TARGETS)

clean: 
	rm -f *.o disassembler cpu_op.c cpu_op.h cpu_op_table.h debug_op_table.h

disassembler: disassembler.o
disassembler.o: disassembler.c debug_op_table.h

op_tables:
	@echo -n "generating: "
	
	@echo -n "cpu_op_table.h"
	@cat cpu_op_table.h.header > cpu_op_table.h
	@cat data/list | ./gen_op_table.pl struct >> cpu_op_table.h
	@echo "" >> cpu_op_table.h
	@echo "// 0xcb prefixed opcodes" >> cpu_op_table.h
	@echo "" >> cpu_op_table.h
	@cat data/list_cb | ./gen_op_table.pl struct cb >> cpu_op_table.h
	@cat cpu_op_table.h.footer >> cpu_op_table.h
	
	@echo -n " cpu_op.h"
	@cat cpu_op.h.header > cpu_op.h
	@cat data/list | ./gen_op_table.pl header >> cpu_op.h
	@echo "" >> cpu_op.h
	@echo "// 0xcb prefixed opcodes" >> cpu_op.h
	@echo "" >> cpu_op.h
	@cat data/list_cb | ./gen_op_table.pl header cb >> cpu_op.h
	@cat cpu_op.h.footer >> cpu_op.h
	
	@echo -n " cpu_op.c"
	@cat data/list | ./gen_op_table.pl func > cpu_op.c
	@echo "" >> cpu_op.c
	@echo "// 0xcb prefixed opcodes" >> cpu_op.c
	@echo "" >> cpu_op.c
	@cat data/list_cb | ./gen_op_table.pl func cb >> cpu_op.c
	
	@echo " debug_op_table.h"
	@cat debug_op_table.h.header > debug_op_table.h
	@cat data/list | ./gen_dasm_op_table.pl >> debug_op_table.h
	@echo "" >> debug_op_table.h
	@echo "// 0xcb prefixed opcodes" >> debug_op_table.h
	@echo "" >> debug_op_table.h
	@cat data/list_cb | ./gen_dasm_op_table.pl cb >> debug_op_table.h
	@cat debug_op_table.h.footer >> debug_op_table.h

