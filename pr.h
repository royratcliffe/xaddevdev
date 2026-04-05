/* SPDX-License-Identifier: MIT */

#ifndef PR_H_
#define PR_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define pr_info(...) fprintf(stdout, ##__VA_ARGS__)
#define pr_err(...) fprintf(stderr, ##__VA_ARGS__)

#endif /* PR_H_ */
