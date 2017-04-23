/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


#ifndef _EXCEPT_H_
#define _EXCEPT_H_


#define SUCCESS             (0)   /* The task bundling external API finishes successfully. */
#define FAIL_FILE_IO        (-10) /* The general exception regarding file IO. */
#define FAIL_MEM_ALLOC      (-20) /* The general exception regarding memory allocation. */
#define FAIL_MISC           (-30) /* The exception regarding miscellaneous utility tasks. */
#define FAIL_EXT_LIB_CALL   (FAIL_MISC - 1) /* Fail to call external shared library. */
#define FAIL_PLUGIN_RESOLVE (FAIL_MISC - 2) /* Fail to resolve cluster module. */
#define FAIL_OPT_PARSE      (FAIL_MISC - 3) /* Fail to parse command line option. */
#define FAIL_CONF_PARSE     (FAIL_MISC - 4) /* Fail to parse cluster configuration. */
#define FAIL_FILE_FORMAT    (FAIL_MISC - 5) /* Fail to resolve the given file. */


#endif
