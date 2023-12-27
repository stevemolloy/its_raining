#include <assert.h>
#include <errno.h>
#include <gcrypt.h>

#include "raylib.h"
#include "memo.h"

#define INITIAL_QA_COUNT 64

#define GCRY_CIPHER GCRY_CIPHER_AES256
#define GCRY_MODE GCRY_CIPHER_MODE_CBC

#define KEY_LENGTH 32 // 256 bits
#define IV_LENGTH 16  // 128 bits

void handle_error(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

int *CodepointRemoveDuplicates(int *codepoints, int codepointCount, int *codepointsResultCount) {
    int codepointsNoDupsCount = codepointCount;
    int *codepointsNoDups = (int *)calloc(codepointCount, sizeof(int));
    memcpy(codepointsNoDups, codepoints, codepointCount*sizeof(int));

    // Remove duplicates
    for (int i = 0; i < codepointsNoDupsCount; i++)
    {
        for (int j = i + 1; j < codepointsNoDupsCount; j++)
        {
            if (codepointsNoDups[i] == codepointsNoDups[j])
            {
                for (int k = j; k < codepointsNoDupsCount; k++) codepointsNoDups[k] = codepointsNoDups[k + 1];

                codepointsNoDupsCount--;
                j--;
            }
        }
    }

    // NOTE: The size of codepointsNoDups is the same as original array but
    // only required positions are filled (codepointsNoDupsCount)

    *codepointsResultCount = codepointsNoDupsCount;
    return codepointsNoDups;
}

bool is_utf8(const char * string) {
    if(!string)
        return 0;

    const unsigned char * bytes = (const unsigned char *)string;
    while(*bytes)
    {
        if( (// ASCII
             // use bytes[0] <= 0x7F to allow ASCII control characters
                bytes[0] == 0x09 ||
                bytes[0] == 0x0A ||
                bytes[0] == 0x0D ||
                (0x20 <= bytes[0] && bytes[0] <= 0x7E)
            )
        ) {
            bytes += 1;
            continue;
        }

        if( (// non-overlong 2-byte
                (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
        ) {
            bytes += 2;
            continue;
        }

        if( (// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                    bytes[0] == 0xEE ||
                    bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
        ) {
            bytes += 3;
            continue;
        }

        if( (// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
        ) {
            bytes += 4;
            continue;
        }

        return 0;
    }

    return 1;
}

bool is_file_encrypted(char *filepath) {
  char *contents = read_entire_file(filepath);
  return !is_utf8(contents);
}

char *decrypt_file(const char *input_filename, const char *password) {
    gcry_cipher_hd_t handle;
    gcry_error_t err;

    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        handle_error("Failed to open input file");
    }

    // Read the IV from the beginning of the encrypted file
    unsigned char iv[IV_LENGTH];
    fread(iv, 1, IV_LENGTH, input_file);

    // Derive the key from the password using a key derivation function (KDF)
    unsigned char key[KEY_LENGTH];
    err = gcry_kdf_derive(password, strlen(password), GCRY_KDF_PBKDF2, GCRY_MD_SHA256, "fafm_donatus", 4, 4096, KEY_LENGTH, key);
    if (err) {
        handle_error("Key derivation failed");
    }

    // Initialize the cipher handle
    err = gcry_cipher_open(&handle, GCRY_CIPHER, GCRY_MODE, 0);
    if (err) {
        handle_error("Cipher initialization failed");
    }

    // Set the key for the cipher handle
    err = gcry_cipher_setkey(handle, key, KEY_LENGTH);
    if (err) {
        gcry_cipher_close(handle);
        handle_error("Failed to set cipher key");
    }

    // Set the IV for the cipher handle
    err = gcry_cipher_setiv(handle, iv, IV_LENGTH);
    if (err) {
        gcry_cipher_close(handle);
        handle_error("Failed to set cipher IV");
    }

    // Decrypt the file content block by block
    size_t block_size = gcry_cipher_get_algo_blklen(GCRY_CIPHER);
    unsigned char *buffer = malloc(block_size);
    size_t decrypted_capacity = block_size; // Initial capacity
    size_t decrypted_size = 0;              // Actual size

    char *decrypted_data = malloc(decrypted_capacity + 1); // +1 for null terminator

    while (1) {
        size_t bytes_read = fread(buffer, 1, block_size, input_file);
        if (bytes_read == 0) {
            break; // End of file
        }

        // Decrypt the block
        err = gcry_cipher_decrypt(handle, buffer, block_size, NULL, 0);
        if (err) {
            free(buffer);
            free(decrypted_data);
            gcry_cipher_close(handle);
            fclose(input_file);
            handle_error("Decryption failed");
        }

        // Resize the buffer if needed
        while (decrypted_size + bytes_read > decrypted_capacity) {
            decrypted_capacity *= 2;
            decrypted_data = realloc(decrypted_data, decrypted_capacity + 1); // +1 for null terminator
        }

        // Append the decrypted block to the result
        memcpy(decrypted_data + decrypted_size, buffer, bytes_read);
        decrypted_size += bytes_read;
    }

    free(buffer);
    gcry_cipher_close(handle);
    fclose(input_file);

    // Resize the final result to the actual size
    decrypted_data = realloc(decrypted_data, decrypted_size + 1); // +1 for null terminator

    // Null-terminate the string
    decrypted_data[decrypted_size] = '\0';

    char last_char = decrypted_data[strlen(decrypted_data)-1];
    while (last_char == '\r' || last_char == '\v' || last_char == '\t' || last_char == '\a' || last_char == '\b' || last_char == '\f') {
      decrypted_data[strlen(decrypted_data)-1] = '\0';
      last_char = decrypted_data[strlen(decrypted_data)-1];
    }

    return decrypted_data;
}

size_t read_entire_file_to_lines(char *file_path, char **buffer, char ***lines) {
  *buffer = read_entire_file(file_path);
  size_t num_lines = string_to_lines(buffer, lines);
  return num_lines;
}

char *read_entire_file(char *file_path) {
  // Reads an entire file into a char array, and returns a ptr to this. The ptr should be freed by the caller
  FILE *f = fopen(file_path, "r");
  if (f==NULL) {
    fprintf(stderr, "Could not read %s: %s\n", file_path, strerror(errno));
    exit(1);
  }

  fseek(f, 0L, SEEK_END);
  int sz = ftell(f);
  fseek(f, 0L, SEEK_SET);

  char *contents = calloc(2*sz, sizeof(char));
  if (contents==NULL) {
    fprintf(stderr, "Could not allocate memory. Buy more RAM I guess?\n");
    exit(1);
  }
  fread(contents, 1, sz, f);

  fclose(f);
  
  return contents;
}

size_t string_to_lines(char **string, char ***lines) {
  char *cursor = *string;
  size_t num_lines = count_lines(cursor);

  *lines = calloc(num_lines, sizeof(char*));

  size_t line_ctr = 0;
  while (*cursor) {
    (*lines)[line_ctr] = cursor;
    assert(line_ctr < num_lines);
    advance_to_char(&cursor, '\n');
    *cursor = '\0';
    (cursor)++;
    line_ctr++;
  }

  return num_lines;
}

size_t count_lines(char *contents) {
  size_t result = 0;
  while (*contents) {
    if (*contents == '\n') result++;
    contents++;
  }
  return result;
}

void advance_to_char(char **string, char c) {
  while (**string != c) (*string)++;
}

QandA empty_qanda(void) {
  QandA result = {0};
  result.capacity = INITIAL_QA_COUNT;
  result.statements = calloc(INITIAL_QA_COUNT, sizeof(char*));

  if (result.statements==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }

  return result;
}

size_t space_estimate_for_qanda(QandA qanda) {
  size_t result = 0;
  for (size_t i=0; i<qanda.length; i++) {
    result += 2 * strlen(qanda.statements[i]);
  }
  return result;
}

void expand_qanda(QandA *qanda) {
  qanda->capacity *= 2;
  qanda->statements = realloc(qanda->statements, qanda->capacity*sizeof(char*));
  if (qanda->statements==NULL) {
    fprintf(stderr, "Unable to allocate memory. Stopping execution\n");
    exit(1);
  }
}

void parse_lines_to_qanda(QandA *qanda, char **lines, size_t num_lines) {
  qanda->title = lines[0];
  qanda->length = 0;
  for (size_t i=1; i<num_lines; i++) {
    if (strlen(lines[i]) == 0) {
      continue;
    }
    append_to_qanda(qanda, lines[i]);
  }
}

void append_to_qanda(QandA *qanda, char *statement) {
  while (qanda->length >= qanda->capacity) expand_qanda(qanda);

  qanda->statements[qanda->length] = statement;
  qanda->length++;
}

void free_qanda(QandA *qanda) {
  free(qanda->statements);
}

void get_qanda_string(QandA qanda, char *str, size_t statement_num) {
  strcpy(str, "\0");
  if (statement_num == 0) {
    return;
  }
  for (size_t i=0; i<statement_num; i++) {
    strcat(str, qanda.statements[i]);
    if (i != statement_num-1) strcat(str, "\n\n");
  }
  strcat(str, "\0");
} 

void adjust_string_for_width(char *orig_str, float usable_width, Font font, float fontsize) {
  Vector2 text_size = MeasureTextEx(font, orig_str, fontsize, 0);
  if (text_size.x <= usable_width) return;

  // Split into words
  size_t num_chars = strlen(orig_str);
  for (size_t i=0; i<num_chars; i++) {
    if (orig_str[i] == ' ') {
      orig_str[i] = '\n';
    }
  }

  for (size_t i=0; i<strlen(orig_str); i++) {
    if (orig_str[i] == '\n') {
      if (num_chars - i > 4 && orig_str[i+1] == '\n' && (orig_str[i+2] == 'Q' || orig_str[i+2] == 'A') && orig_str[i+3] == ':') {
        i += 3;
        continue;
      }
      orig_str[i] = ' ';
      Vector2 size = MeasureTextEx(font, orig_str, fontsize, 0);
      if (size.x > usable_width) {
        orig_str[i] = '\n';
      }
    }
  }
  
  return;
}

