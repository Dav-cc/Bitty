#include <criterion/criterion.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bencode.h"
#include "buffer.h"
#include "log.h"

static byte_buffer make_buffer(const char *input)
{
    byte_buffer bb = {
        .data = (uint8_t *)input,
        .len = strlen(input),
        .offset = 0
    };

    return bb;
}

Test(bencode_string, parses_regular_string)
{
    const char *input = "4:spam";
    byte_buffer bb = make_buffer(input);
    byte_slice *out = NULL;
    error_t err = {0};

    int rc = bencode_parse_string(&bb, &out, &err);

    cr_assert_eq(rc, 0);
    cr_assert_not_null(out);
    cr_assert_eq(out->length, 4);
    cr_assert_arr_eq(out->data, "spam", 4);

    free(out);
}

Test(bencode_string, parses_empty_string)
{
    const char *input = "0:";
    byte_buffer bb = make_buffer(input);
    byte_slice *out = NULL;
    error_t err = {0};

    int rc = bencode_parse_string(&bb, &out, &err);

    cr_assert_eq(rc, 0);
    cr_assert_not_null(out);
    cr_assert_eq(out->length, 0);

    free(out);
}

Test(bencode_string, rejects_missing_length)
{
    const char *input = ":abc";
    byte_buffer bb = make_buffer(input);
    byte_slice *out = NULL;
    error_t err = {0};

    int rc = bencode_parse_string(&bb, &out, &err);

    cr_assert_eq(rc, -1);

    free(out);
}
