#include "lmsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//======================================================
//  Utilities
//======================================================

void lmsm_cap_value(int *val) {
    // Cap the value pointed to by this pointer between 999 and -999
    if (*val > 999) {
        *val = 999;
    } else if (*val < -999) {
        *val = -999;
    }
}

int lmsm_has_two_values_on_stack(lmsm *our_little_machine) {
    // Return 0 if there are not two values on the stack
    return (our_little_machine->stack_pointer >= our_little_machine->return_address_pointer + 2);
}

//======================================================
//  Instruction Implementation
//======================================================

void lmsm_i_jal(lmsm *our_little_machine) {
    // Save the current program counter onto the stack
    our_little_machine->stack[our_little_machine->stack_pointer] = our_little_machine->program_counter;
    our_little_machine->stack_pointer++;
    // Jump to the specified location
    our_little_machine->program_counter = our_little_machine->current_instruction;
}

void lmsm_i_ret(lmsm *our_little_machine) {
    // Return from the subroutine by popping the return address from the stack
    our_little_machine->stack_pointer--;
    our_little_machine->program_counter = our_little_machine->stack[our_little_machine->stack_pointer];
}

void lmsm_i_push(lmsm *our_little_machine) {
    // Push the current accumulator value onto the stack
    our_little_machine->stack_pointer++;
    our_little_machine->stack[our_little_machine->stack_pointer] = our_little_machine->accumulator;
}

void lmsm_i_pop(lmsm *our_little_machine) {
    // Pop a value from the stack and store it in the accumulator
    our_little_machine->accumulator = our_little_machine->stack[our_little_machine->stack_pointer];
    our_little_machine->stack_pointer--;
}

void lmsm_i_dup(lmsm *our_little_machine) {
    // Duplicate the value at the top of the stack
    our_little_machine->stack_pointer++;
    our_little_machine->stack[our_little_machine->stack_pointer] = our_little_machine->stack[our_little_machine->stack_pointer - 1];
}

void lmsm_i_drop(lmsm *our_little_machine) {
    // Remove the value at the top of the stack
    our_little_machine->stack_pointer--;
}

void lmsm_i_swap(lmsm *our_little_machine) {
    // Swap the top two values on the stack
    int temp = our_little_machine->stack[our_little_machine->stack_pointer];
    our_little_machine->stack[our_little_machine->stack_pointer] = our_little_machine->stack[our_little_machine->stack_pointer - 1];
    our_little_machine->stack[our_little_machine->stack_pointer - 1] = temp;
}

void lmsm_i_sadd(lmsm *our_little_machine) {
    // Signed addition: Add the top two values on the stack
    our_little_machine->stack[our_little_machine->stack_pointer - 1] += our_little_machine->stack[our_little_machine->stack_pointer];
    lmsm_cap_value(&our_little_machine->stack[our_little_machine->stack_pointer - 1]);
    our_little_machine->stack_pointer--;
}

void lmsm_i_ssub(lmsm *our_little_machine) {
    // Signed subtraction: Subtract the top value on the stack from the second-top value
    our_little_machine->stack[our_little_machine->stack_pointer - 1] -= our_little_machine->stack[our_little_machine->stack_pointer];
    lmsm_cap_value(&our_little_machine->stack[our_little_machine->stack_pointer - 1]);
    our_little_machine->stack_pointer--;
}

void lmsm_i_smax(lmsm *our_little_machine) {
    // Signed maximum: Replace the top two values with the maximum of the two
    our_little_machine->stack[our_little_machine->stack_pointer - 1] = 
        (our_little_machine->stack[our_little_machine->stack_pointer - 1] > our_little_machine->stack[our_little_machine->stack_pointer]) ? 
        our_little_machine->stack[our_little_machine->stack_pointer - 1] : our_little_machine->stack[our_little_machine->stack_pointer];
    our_little_machine->stack_pointer--;
}

void lmsm_i_smin(lmsm *our_little_machine) {
    // Signed minimum: Replace the top two values with the minimum of the two
    our_little_machine->stack[our_little_machine->stack_pointer - 1] = 
        (our_little_machine->stack[our_little_machine->stack_pointer - 1] < our_little_machine->stack[our_little_machine->stack_pointer]) ? 
        our_little_machine->stack[our_little_machine->stack_pointer - 1] : our_little_machine->stack[our_little_machine->stack_pointer];
    our_little_machine->stack_pointer--;
}

void lmsm_i_smul(lmsm *our_little_machine) {
    // Signed multiplication: Multiply the top two values on the stack
    our_little_machine->stack[our_little_machine->stack_pointer - 1] *= our_little_machine->stack[our_little_machine->stack_pointer];
    lmsm_cap_value(&our_little_machine->stack[our_little_machine->stack_pointer - 1]);
    our_little_machine->stack_pointer--;
}

void lmsm_i_sdiv(lmsm *our_little_machine) {
    // Signed division: Divide the second-top value on the stack by the top value
    if (our_little_machine->stack[our_little_machine->stack_pointer] != 0) {
        our_little_machine->stack[our_little_machine->stack_pointer - 1] /= our_little_machine->stack[our_little_machine->stack_pointer];
        lmsm_cap_value(&our_little_machine->stack[our_little_machine->stack_pointer - 1]);
        our_little_machine->stack_pointer--;
    } else {
        // Handle division by zero error
        our_little_machine->error_code = ERROR_DIVISION_BY_ZERO;
        our_little_machine->status = STATUS_HALTED;
    }
}

void lmsm_i_out(lmsm *our_little_machine) {
    // Append the current accumulator to the output buffer in the LMSM
    snprintf(our_little_machine->output_buffer, sizeof(our_little_machine->output_buffer), "%d", our_little_machine->accumulator);
}

void lmsm_i_inp(lmsm *our_little_machine) {
    // Read a value from the command line and store it as an int in the accumulator
    printf("Enter a value: ");
    scanf("%d", &our_little_machine->accumulator);
}

void lmsm_i_load(lmsm *our_little_machine, int location) {
    // Load a value from memory into the accumulator
    if (location >= 0 && location <= TOP_OF_MEMORY) {
        our_little_machine->accumulator = our_little_machine->memory[location];
    } else {
        // Handle out-of-bounds memory access error
        our_little_machine->error_code = ERROR_OUT_OF_BOUNDS;
        our_little_machine->status = STATUS_HALTED;
    }
}

void lmsm_i_add(lmsm *our_little_machine, int location) {
    // Add a value from memory to the accumulator
    if (location >= 0 && location <= TOP_OF_MEMORY) {
        our_little_machine->accumulator += our_little_machine->memory[location];
        lmsm_cap_value(&our_little_machine->accumulator);
    } else {
        // Handle out-of-bounds memory access error
        our_little_machine->error_code = ERROR_OUT_OF_BOUNDS;
        our_little_machine->status = STATUS_HALTED;
    }
}

void lmsm_i_sub(lmsm *our_little_machine, int location) {
    // Subtract a value from memory from the accumulator
    if (location >= 0 && location <= TOP_OF_MEMORY) {
        our_little_machine->accumulator -= our_little_machine->memory[location];
        lmsm_cap_value(&our_little_machine->accumulator);
    } else {
        // Handle out-of-bounds memory access error
        our_little_machine->error_code = ERROR_OUT_OF_BOUNDS;
        our_little_machine->status = STATUS_HALTED;
    }
}

void lmsm_i_load_immediate(lmsm *our_little_machine, int value) {
    // Load an immediate value into the accumulator
    our_little_machine->accumulator = value;
}

void lmsm_i_store(lmsm *our_little_machine, int location) {
    // Store the accumulator value into memory
    if (location >= 0 && location <= TOP_OF_MEMORY) {
        our_little_machine->memory[location] = our_little_machine->accumulator;
    } else {
        // Handle out-of-bounds memory access error
        our_little_machine->error_code = ERROR_OUT_OF_BOUNDS;
        our_little_machine->status = STATUS_HALTED;
    }
}

void lmsm_i_halt(lmsm *our_little_machine) {
    // Set the machine status to HALTED
    our_little_machine->status = STATUS_HALTED;
}

void lmsm_i_branch_unconditional(lmsm *our_little_machine, int location) {
    // Unconditional branch to the specified location
    our_little_machine->program_counter = location;
}

void lmsm_i_branch_if_zero(lmsm *our_little_machine, int location) {
    // Branch to the specified location if the accumulator is zero
    if (our_little_machine->accumulator == 0) {
        our_little_machine->program_counter = location;
    }
}

void lmsm_i_branch_if_positive(lmsm *our_little_machine, int location) {
    // Branch to the specified location if the accumulator is positive
    if (our_little_machine->accumulator > 0) {
        our_little_machine->program_counter = location;
    }
}

void lmsm_step(lmsm *our_little_machine) {
    // If the machine is not halted, we need to read the instruction in the memory slot
    // pointed to by the program counter, bump the program counter, then execute
    // the instruction
    if (our_little_machine->status != STATUS_HALTED) {
        int next_instruction = our_little_machine->memory[our_little_machine->program_counter];
        our_little_machine->program_counter++;
        our_little_machine->current_instruction = next_instruction;
        int instruction = our_little_machine->current_instruction;
        lmsm_exec_instruction(our_little_machine, instruction);
    }
}

//======================================================
//  LMSM Implementation
//======================================================

void lmsm_exec_instruction(lmsm *our_little_machine, int instruction) {
    // Dispatch the rest of the instruction set and implement the instructions above

    switch (instruction) {
        case 0:
            lmsm_i_halt(our_little_machine);
            break;
        case 100 ... 199:
            lmsm_i_add(our_little_machine, instruction - 100);
            break;
        // Handle other instructions similarly
        default:
            // Unknown instruction
            our_little_machine->error_code = ERROR_UNKNOWN_INSTRUCTION;
            our_little_machine->status = STATUS_HALTED;
            break;
    }
}

void lmsm_load(lmsm *our_little_machine, int *program, int length) {
    // Load a program into the machine's memory
    for (int i = 0; i < length; ++i) {
        if (i <= TOP_OF_MEMORY) {
            our_little_machine->memory[i] = program[i];
        } else {
            // Handle out-of-bounds memory access error
            our_little_machine->error_code = ERROR_OUT_OF_BOUNDS;
            our_little_machine->status = STATUS_HALTED;
        }
    }
}

void lmsm_init(lmsm *the_machine) {
    // Initialize the LMSM
    the_machine->accumulator = 0;
    the_machine->status = STATUS_READY;
    the_machine->error_code = ERROR_NONE;
    the_machine->program_counter = 0;
    the_machine->current_instruction = 0;
    the_machine->stack_pointer = TOP_OF_MEMORY + 1;
    the_machine->return_address_pointer = TOP_OF_MEMORY - 100;
    memset(the_machine->output_buffer, 0, sizeof(char) * 1000);
    memset(the_machine->memory, 0, sizeof(int) * TOP_OF_MEMORY + 1);
}

void lmsm_reset(lmsm *our_little_machine) {
    // Reset the machine to its initial state
    lmsm_init(our_little_machine);
}

void lmsm_run(lmsm *our_little_machine) {
    // Run the machine's program
    our_little_machine->status = STATUS_RUNNING;
    while (our_little_machine->status != STATUS_HALTED) {
        lmsm_step(our_little_machine);
    }
}

lmsm *lmsm_create() {
    // Create a new LMSM and initialize it
    lmsm *the_machine = malloc(sizeof(lmsm));
    lmsm_init(the_machine);
    return the_machine;
}

void lmsm_delete(lmsm *the_machine) {
    // Free the memory used by the LMSM
    free(the_machine);
}
