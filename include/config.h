/**
 * @file config.h
 * @brief Configuration parsing and validation
 * 
 * Handles command-line argument parsing and configuration validation
 * for all tasks (1-4). Automatically detects which task based on parameters.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

/**
 * @brief Parse command-line arguments
 * 
 * Parses all supported command-line arguments and populates
 * the simulator configuration structure.
 * 
 * Supported arguments:
 * - Single-level cache (Tasks 1-3):
 *   -S size      Cache size in bytes
 *   -B blocksize Block size in bytes (default: 16)
 *   -A assoc     Associativity (1/2/3/4, default: 2=fully-assoc)
 * 
 * - Multi-level cache (Task 4):
 *   -S1 size     L1 cache size
 *   -B1 blocksize L1 block size
 *   -A1 assoc    L1 associativity
 *   -S2 size     L2 cache size
 *   -B2 blocksize L2 block size
 *   -A2 assoc    L2 associativity
 * 
 * - TLB:
 *   -T entries   TLB entries
 *   -L assoc     TLB associativity
 * 
 * - Other:
 *   -t tracefile Trace file path
 *   -v           Verbose mode
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return Pointer to populated configuration, or NULL on error
 */
sim_config_t* parse_arguments(int argc, char *argv[]);

/**
 * @brief Validate configuration
 * 
 * Checks that the configuration is valid:
 * - Cache size > 0 and multiple of 4
 * - Block size >= 4 and multiple of 4
 * - Valid associativity codes (1-4)
 * - For set-associative: cache size divisible by (block_size * ways)
 * - For multi-level: L2 >= L1 size, compatible block sizes
 * - TLB entries >= 2 and power of 2
 * - Trace file exists
 * 
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
bool validate_config(const sim_config_t *config);

/**
 * @brief Detect task number from configuration
 * 
 * Task 1: Default settings (fully-assoc, block_size=16)
 * Task 2: Custom block size specified (-B)
 * Task 3: Custom associativity specified (-A)
 * Task 4: Multi-level cache (-S2 specified)
 * 
 * @param config Configuration to analyze
 * @return Task number (1, 2, 3, or 4)
 */
int detect_task(const sim_config_t *config);

/**
 * @brief Free configuration resources
 * 
 * @param config Configuration to free
 */
void free_config(sim_config_t *config);

/**
 * @brief Print configuration summary
 * 
 * Useful for debugging and verification.
 * 
 * @param config Configuration to print
 */
void print_config(const sim_config_t *config);

#endif /* CONFIG_H */

