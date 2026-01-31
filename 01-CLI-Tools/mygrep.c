/**
 * @file mygrep.c
 * @brief A simplified implementation of the Unix 'grep' utility.
 * Supports case-insensitive search (-i) and custom output files (-o).
 * Demonstrates POSIX argument parsing (getopt), stream processing, and dynamic memory management.
 */

#define _POSIX_C_SOURCE 200809L // Required for getline
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // for getopt
#include <assert.h>

/**
 * Reads a stream line by line and prints lines containing the keyword.
 * * @param input The input file stream (or stdin).
 * @param output The output file stream (or stdout).
 * @param keyword The search term.
 * @param case_insensitive Flag: 1 for case-insensitive search, 0 otherwise.
 */
void process_stream(FILE *input, FILE *output, const char *keyword, int case_insensitive) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // getline automatically reallocates 'line' buffer as needed
    while ((read = getline(&line, &len, input)) != -1) {
        
        // Create a temporary copy for comparison to preserve original line for output
        char *comparison_line = malloc(read + 1);
        if (!comparison_line) {
            perror("Memory allocation failed");
            free(line);
            exit(EXIT_FAILURE);
        }
        strcpy(comparison_line, line);

        // Normalize to lowercase if case-insensitive requested
        if (case_insensitive) {
            for (int i = 0; comparison_line[i]; i++) {
                comparison_line[i] = tolower((unsigned char)comparison_line[i]);
            }
        }

        // Check for keyword occurrence
        if (strstr(comparison_line, keyword) != NULL) {
            fprintf(output, "%s", line);
        }

        free(comparison_line);
    }
    
    free(line); // getline buffer must be freed by caller
}

int main(int argc, char *argv[]) {
    int case_insensitive = 0;
    char *outfile_path = NULL;
    FILE *output = stdout;
    
    int opt;
    // Parse command line arguments using POSIX getopt
    // "i" = flag, "o:" = option requiring an argument
    while ((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
            case 'i':
                case_insensitive = 1;
                break;
            case 'o':
                outfile_path = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", argv[0]);
                return EXIT_FAILURE;
            default:
                abort(); // Should not be reached
        }
    }

    // Prepare output stream
    if (outfile_path != NULL) {
        output = fopen(outfile_path, "w");
        if (output == NULL) {
            fprintf(stderr, "%s: Error opening output file '%s': %s\n", 
                    argv[0], outfile_path, strerror(errno));
            return EXIT_FAILURE;
        }
    }

    // Check if keyword is provided
    if (optind >= argc) {
        fprintf(stderr, "Error: No keyword provided.\n");
        fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", argv[0]);
        if (output != stdout) fclose(output);
        return EXIT_FAILURE;
    }

    // Retrieve keyword and advance index
    char *raw_keyword = argv[optind];
    char *search_keyword = strdup(raw_keyword); // Duplicate to avoid modifying argv
    if (!search_keyword) {
        perror("Memory allocation failed");
        return EXIT_FAILURE;
    }
    optind++;

    if (case_insensitive) {
        for (int i = 0; search_keyword[i]; i++) {
            search_keyword[i] = tolower((unsigned char)search_keyword[i]);
        }
    }

    // Process inputs: either stdin (if no files) or list of files
    if (optind >= argc) {
        process_stream(stdin, output, search_keyword, case_insensitive);
    } else {
        for (int i = optind; i < argc; i++) {
            char *current_path = argv[i];
            FILE *input = fopen(current_path, "r");

            if (input == NULL) {
                // Standard grep behavior: print error to stderr but CONTINUE with next file
                fprintf(stderr, "%s: Error opening input file '%s': %s\n", 
                        argv[0], current_path, strerror(errno));
                continue; 
            }
            
            process_stream(input, output, search_keyword, case_insensitive);
            fclose(input);
        }
    }

    free(search_keyword);
    if (output != stdout) {
        fclose(output);
    }

    return EXIT_SUCCESS;
}
