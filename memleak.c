#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void format_bytes(size_t bytes, char *output, size_t output_size) {
  const char *suffixes[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
  int i = 0;
  double dblBytes = bytes;

  if (bytes == 0) {
    snprintf(output, output_size, "0 %s", suffixes[i]);
    return;
  }

  // 二进制换算
  while (dblBytes >= 1024.0 && i < (int)(sizeof(suffixes) / sizeof(suffixes[0]) - 1)) {
    dblBytes /= 1024.0;
    i++;
  }

  // 根据大小选择合适的格式
  if (i == 0) {
    snprintf(output, output_size, "%d %s", (int)dblBytes, suffixes[i]);
  } else if (dblBytes < 100.0) {
    snprintf(output, output_size, "%.2f %s", dblBytes, suffixes[i]);
  } else {
    snprintf(output, output_size, "%.1f %s", dblBytes, suffixes[i]);
  }

  return;
}

size_t parse_size(const char *size_str) {
  char *endptr;
  size_t multiplier = 1;
  long size = strtol(size_str, &endptr, 10);

  if (endptr == size_str) {
    fprintf(stderr, "Error: Invalid size format: %s\n", size_str);
    exit(1);
  }

  // 检查单位
  if (*endptr != '\0') {
    if (strcasecmp(endptr, "B") == 0 || strcmp(endptr, "") == 0) {
      // 字节，不需要转换
    } else if (strcasecmp(endptr, "KB") == 0) {
      multiplier = 1024;
    } else if (strcasecmp(endptr, "MB") == 0) {
      multiplier = 1024 * 1024;
    } else if (strcasecmp(endptr, "GB") == 0) {
      multiplier = 1024 * 1024 * 1024;
    } else {
      fprintf(stderr, "Error: Unknown size unit: %s\n", endptr);
      fprintf(stderr, "Supported units: B, KB, MB, GB\n");
      exit(1);
    }
  }

  return size * multiplier;
}

void print_usage(const char *program_name) {
  printf("Usage: %s [OPTIONS]\n", program_name);
  printf("Simulates a memory leak.\n");
  printf("\n");
  printf("OPTIONS:\n");
  printf("  -i <size>    Block size to leak when init the program (default: 0B)\n");
  printf("               Supports units: B, KB, MB, GB (e.g., 10MB, 1GB)\n");
  printf("  -b <size>    Block size to leak (default: 1KB)\n");
  printf("               Supports units: B, KB, MB, GB (e.g., 10MB, 1GB)\n");
  printf("  -t <ms>      Interval between leaks in milliseconds (default: 1000)\n");
  printf("  -c <count>   Number of blocks to leak before pausing (default: infinite)\n");
  printf("  -p <secs>    Pause after leaking <count> blocks for <secs> seconds (default: 0)\n");
  printf("  -h           Show this help message\n");
  printf("\n");
  printf("EXAMPLE:\n");
  printf("  %s -b 1MB -t 2000    # Leak 1MB every 2 seconds\n", program_name);
  printf("  %s -b 512KB -t 500    # Leak 512KB every 0.5 seconds\n", program_name);
}

bool leak(size_t size) {
  void *ptr = malloc(size);
  if (!ptr) {
    fprintf(stderr, "Failed to allocate memory\n");
    return false;
  }

  // Touch the memory to ensure it's really allocated
  memset(ptr, 0, size);

  return true;
}

int main(int argc, char *argv[]) {
  size_t start_size = 0;
  size_t block_size = 1024;
  int interval_ms = 1000;
  int count = -1;
  int pause_secs = 0;

  int opt;
  while ((opt = getopt(argc, argv, "i:b:t:c:p:h")) != -1) {
    switch (opt) {
    case 'i':
      start_size = parse_size(optarg);
      break;
    case 'b':
      block_size = parse_size(optarg);
      break;
    case 't':
      interval_ms = atoi(optarg);
      break;
    case 'c':
      count = atoi(optarg);
      break;
    case 'p':
      pause_secs = atoi(optarg);
      break;
    case 'h':
    default:
      print_usage(argv[0]);
      return 0;
    }
  }

  char size_str[50];
  printf("Starting memory leak simulation:\n");
  format_bytes(start_size, size_str, sizeof(size_str));
  printf("  Init Block size: %s bytes\n", size_str);
  format_bytes(block_size, size_str, sizeof(size_str));
  printf("  Block size: %s bytes\n", size_str);
  printf("  Interval: %d ms\n", interval_ms);
  if (count > 0) {
    printf("  Count: %d blocks\n", count);
    printf("  Pause: %d seconds\n", pause_secs);
  } else {
    printf("  Count: infinite\n");
  }
  printf("Press Ctrl+C to stop.\n\n");

  struct timespec req;
  req.tv_sec = interval_ms / 1000;
  req.tv_nsec = (interval_ms % 1000) * 1000000;

  long leaked_blocks = 0;
  size_t total_leaked = 0;
  char total_str[50];

  if (start_size > 0) {
    const size_t max_block_size = 1024 * 1024;
    if (start_size > max_block_size) {
      for (size_t i = 0; i < start_size / max_block_size; i++) {
        leak(max_block_size);
      }
      size_t left = start_size % max_block_size;
      if (left > 0) {
        leak(left);
      }
    } else {
      leak(start_size);
    }
    format_bytes(start_size, size_str, sizeof(size_str));
    printf("Init leaked: %s\n", size_str);
  }

  while (count < 0 || leaked_blocks < count) {
    // Allocate memory and intentionally don't free it (LEAK)
    bool leak_result = leak(block_size);
    if (!leak_result) {
      fprintf(stderr, "Failed to leak memory at block %ld\n", leaked_blocks);
      break;
    }

    leaked_blocks++;
    total_leaked = leaked_blocks * block_size + start_size;

    format_bytes(total_leaked, total_str, sizeof(total_str));
    printf("Leaked block #%ld (%s total)\n", leaked_blocks, total_str);

    // Sleep for the specified interval
    if (nanosleep(&req, NULL) != 0) {
      break;
    }

    // Pause if count and pause are specified
    if (count > 0 && leaked_blocks % count == 0) {
      printf("Pausing for %d seconds...\n", pause_secs);
      sleep(pause_secs);
    }
  }

  format_bytes(total_leaked, total_str, sizeof(total_str));
  printf("\nFinished. Total leaked: %s in %ld blocks.\n", total_str, leaked_blocks);
  printf("Process will now exit and all memory will be freed.\n");

  // Note: We're intentionally NOT freeing the allocated memory
  // to simulate a real leak. When the process exits, the OS will
  // reclaim all memory.

  return 0;
}
