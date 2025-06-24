# THE C History 
ISOC: 
- 1990 aka C90 
- 1995 aka C95 
- 1999 aka C99 
- 2011 aka C11 
- 2018 aka C18 
- 2023 aka C23
# THE necessity of MISRA C 
The ISO C Standard leaves some behaviors flexible to allow performance and portability, but this can lead to unsafe or non-portable code.

- **Types of Undefined or Flexible Behavior in C:**
    - Undefined Behavior: Anything can happen (e.g., buffer overflows).
    - Unspecified Behavior: Result varies, not documented (e.g., argument evaluation order).
    - Implementation-defined Behavior: Compiler chooses, but must document (e.g., size of int, right shift of signed integers).

# MISRA: 
MISRA (Motor Industry Software Reliability Association) provides guidelines for writing safe, reliable, and portable C code, especially for safety-critical systems like:
- Automotive
- Aerospace
- Medical
- Industrial automation
#### MISRA time line 
| Year  | Standard                  | Notes                                      |
|-------|---------------------------|--------------------------------------------|
| 1994  | MISRA Guidelines          | General vehicle software safety           |
| 1998  | MISRA C:1998              | First C-specific automotive guideline     |
| 2004  | MISRA C:2004              | For critical systems, compatible with C90 |
| 2012  | MISRA C:2012              | Compatible with C99, most widely adopted  |
| 2023  | MISRA C:2012 + Amendments | Includes updates and clarifications       |
#### Goals of MSIRA 
- Define a safe subset of the C language
- Eliminate dangerous constructs
- Encourage clear, maintainable, and testable code


# Some Guidelines 
1. **Always use braces for control blocks:**
```C
// Good
if (x > 0)
{
    do_something();
}

// Bad
if (x > 0)
    do_something();
 ```
2. **Avoid implict conversions and casts:** 
```C
uint8_t a = 255; 
int b = a  ; // BAD: MISRA violation ( implicit cast from unsigned to signed ) 
```
3. **Do not use memory allocation:** 
Dynamic allocation is unpredicatable and can gragment memeory. 
4. **No recursion:** 
Recursion can lead to stack overflows and is hard to test in safety critical
code 
5. **Use `uintx_t`/`intx_t` from `<stdint.h>` for fixed-width types 
6. **Do not use `goto`
7. **avoid mixed-type arithmetics** 
```c
uint16_t a = 100 ;
int32_t b = -5; 
uint32_t res = a +b ; // Wrong unsigned + signed 
int32_t res = (int32_t) a + b  // correct way  
```
8. **Usage of Switch case:**
when using a `switch/case` you need to have a `deafult` case and each case must
end with `break` 
```C
switch (x)
{
    case 1:
        do_stuff();
        break; // ✅
    case 2:
        do_other();
        break;
    default:
        break;
}
```
9. **Do not use printf, scanf, or other std I/O**
10. **the usage of static:**
the standard requires `static` variables to be initialized to zero (unless to excpliciltly initize them to otehrwise) 
11. **Usage of if - else if - else structure:**
Always use braces {} even for single-line blocks.<br>
Avoid deep nesting if possible (helps readability and testability).<br> 
```c
if (state == INIT)
{
    initialize();
}
else if (state == RUN)
{
    run_logic();
}
else
{
    shutdown();
}
```
 > not Recommended 
 ```c
 if (state == INIT)
    initialize(); // No braces – violates MISRA Rule 15.6
```
11. **For loops usage:**
- Loop control variables must not overflow or wrap.<br>
- Loop bounds should be clearly defined and deterministic.<br>
- Avoid complex conditions and side-effects inside the loop header.<br>
- break and continue are allowed in MISRA C:2012, but use them sparingly and document them.<br>
- Do not modify the loop variable inside the loop body.<br> 
```c
uint8_t i;

for (i = 0U; i < 10U; ++i)
{
    do_task(i);
}
// ... 
for (i = 0; i++ < 10;) // Complex/ambiguous header – not compliant
{
    if (i == 5)
        break; // should be documented
}
```
12. **Interrupt Service Routine rules** 
- ✅ DOs in an ISR

| Rule               |Description                                                                 |
|--------------------|-----------------------------------------------------------------------------|
| 🧠 **Keep it short**  | Do the minimal required action only (e.g., set a flag, read data).          |
| 📤 **Set flags or enqueue** | Communicate with the main loop via a flag, queue, or buffer.               |
| 🚫 **Use `volatile`** | Shared variables between ISR and main must be `volatile`.                   |
| 🔒 **Be safe**       | Disable interrupts (if needed) only for short durations.                    |
| 📵 **Minimal calls** | Call only reentrant and safe functions.                                     |
| 🧪 **Use memory-safe access** | Watch out for global data or shared buffers.                              |

---

- ❌ **What to AVOID in an ISR**: 
    
    - **No blocking operations** (e.g., waiting in a loop).  
    - **No dynamic memory** (`malloc`, `free`).  
    - **Avoid calling non-reentrant functions** (e.g., `printf`, `sprintf`).  
    - **No long computations or looping**.  
    - **Don’t modify shared variables without proper synchronization**.  
    
- Example ISR Structure (MISRA-safe)
```c
#include <stdint.h>
#include <stdbool.h>

volatile bool data_ready = false;
volatile uint16_t sensor_value;

void ADC_IRQHandler(void) 
{
    sensor_value = ADC_Read();  // Read ADC result
    data_ready = true;         // Set flag for main loop
}
// ... 
// Main loop Handling 
void main(void) 
{
    while (1) 
    {
        if (data_ready == true) 
        {
            data_ready = false;
            process_data(sensor_value);
        }
    }
}
```
13. **MISRA C Naming Rules (and Best Practices)**

| Rule            | Description | Example / Note |
|-----------------|-------------|----------------|
| **Rule 5.1**    | Identifiers must be **unique in the first 31 characters** and **case-insensitive in external linkage** (avoids collisions on older platforms). | `DataLogger` vs `Data_Logger` (OK if first 31 chars differ) |
| **Rules 5.2–5.4** | Avoid names that differ **only in case** or are **too similar**. | ❌ `DataBuffer` vs `databuffer`<br>✅ `DataBuffer` vs `RawDataBuffer` |
| **Rules 5.6–5.7** | Don’t **reuse names** across different scopes/namespaces (e.g., typedef, variable, function). | ❌ `typedef uint8_t width;` + `uint16_t width;` |
| **Avoid reserved names** | Never use identifiers starting with `_` or `__` (reserved for compiler/stdlib). | ❌ `__counter`, `_temp`<br>✅ `counter`, `tempValue` |
| **Avoid very short names** | Use **meaningful names** (exception: trivial loop counters like `i`, `j`). | ❌ `s`, `x`<br>✅ `speed_kph`, `position_x` |
| **Use consistent schemes** | Apply **snake_case** or **CamelCase** consistently project-wide. | ✅ `get_sensor_data()` (snake_case)<br>✅ `getSensorData()` (CamelCase) |
---

- Some naming convensions 

🔹 Variables

Descriptive and lowercase with underscores:

`engine_temperature, brake_pressure, sensor_value`

Prefix boolean variables with is_, has_, or flag_:

`is_ready, has_error, flag_received`

🔹 Constants (usually #define or const)

All uppercase with underscores:

`MAX_SPEED, TIMEOUT_MS, BUFFER_SIZE`

🔹 Functions

Verb-based, descriptive names:

`init_motor(), calculate_speed(), read_sensor_data()`

🔹 Types (typedef, structs, enums)

Capitalized with a suffix:

`MotorState_t, SensorData_t, ConfigParams_t`

14.  **Functions Must Have Only One Exit Point:**
- ❌ Bad Example (multiple exit points):
```c
int process_data(int x)
{
    if (x < 0)
        return -1;  // ❌ early exit

    if (x > 100)
        return -2;  // ❌ another early exit

    return 0;       // ❌ multiple returns
}
```
- ✅ Good Example (single return):
```c
int process_data(int x)
{
    int status = 0;

    if (x < 0)
    {
        status = -1;
    }
    else if (x > 100)
    {
        status = -2;
    }

    return status;  // ✅ single exit point
}
```
