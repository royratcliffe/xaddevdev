/* SPDX-License-Identifier: MIT */

#ifndef PR_H_
#define PR_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern int pr_verbosity;

#define pr_debug(...) do { if (pr_verbosity > 2) fprintf(stdout, ##__VA_ARGS__); } while (0)
#define pr_info(...) do { if (pr_verbosity > 1) fprintf(stdout, ##__VA_ARGS__); } while (0)
#define pr_warn(...) do { if (pr_verbosity > 0) fprintf(stderr, ##__VA_ARGS__); } while (0)
#define pr_err(...) do { fprintf(stderr, ##__VA_ARGS__); } while (0)

#endif /* PR_H_ */
