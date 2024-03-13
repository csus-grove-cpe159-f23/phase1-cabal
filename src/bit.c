/** //f
 * CPE/CSC 159 Operating System Pragmatics
 * California State University, Sacramento
 *
 * Bit Utilities
 */
//d
/** Ahoy! //f
 * The following code assumes zero indexing
 * The following code assumes 16 bit values
 */
//d
#include "bit.h"
unsigned int bit_count(unsigned int value) {//f
/** //f
 * Counts the number of bits that are set
 * @param value - the integer value to count bits in
 * @return number of bits that are set
 */
//d
    if(value == 0){return 0;}
    if((value & 1) == 1){
        return 1 + bit_count(value >> 1);
    }else{
        return bit_count(value >> 1);
    }
}
//d
unsigned int bit_test(unsigned int value, int bit) {//f
/** //f
 * Checks if the given bit is set
 * @param value - the integer value to test
 * @param bit - which bit to check
 * @return 1 if set, 0 if not set
 */
//d
    return (1 & (value >> bit));
}
//d
unsigned int bit_set(unsigned int value, int bit) {//f
/** //f
 * Sets the specified bit in the given integer value
 * @param value - the integer value to modify
 * @param bit - which bit to set
 */
//d
    return (value | (1 << bit));
}
//d
unsigned int bit_clear(unsigned int value, int bit) {//f
/** //f
 * Clears the specified bit in the given integer value
 * @param value - the integer value to modify
 * @param bit - which bit to clear
 */
//d
    return (value & ~(1 << bit));
}
//d
unsigned int bit_toggle(unsigned int value, int bit) {//f
/** //f
 * Toggles the specified bit in the given integer value
 * @param value - the integer value to modify
 * @param bit - which bit to toggle
 */
//d
    return (value ^ (1 << bit));
}
//d
