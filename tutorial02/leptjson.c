#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define ISWHITESPACE(ch)    ((ch) == '\t'|| (ch) == ' ' || (ch) == '\n' || (ch) == '\r')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    EXPECT(c, *literal);
    literal++;
    while (*literal) {
        if (*c->json != *literal)
            return LEPT_PARSE_INVALID_VALUE;
        c->json++;
        literal++;
    }
    v->type = type;
    return LEPT_PARSE_OK;
}

static int validate_exp(char const** number_ptr) {
    assert(**number_ptr == 'E' || **number_ptr == 'e');
    (*number_ptr)++;
    if (**number_ptr == '+' || **number_ptr == '-')
        (*number_ptr)++;
    while (ISDIGIT(**number_ptr))
        (*number_ptr)++;
    if (isspace(**number_ptr) || **number_ptr == '\0')
        return LEPT_PARSE_OK;
    else
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
}

static int validate_frac(char const** number_ptr) {
    assert(**number_ptr == '.');
    (*number_ptr)++;
    if (ISDIGIT(**number_ptr)) {
        (*number_ptr)++;
        while (ISDIGIT(**number_ptr))
            (*number_ptr)++;
        if (**number_ptr == 'e' || **number_ptr == 'E')
            return validate_exp(number_ptr);
        else if (ISWHITESPACE(**number_ptr) || **number_ptr == '\0')
            return LEPT_PARSE_OK;
        else
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    } else
        return LEPT_PARSE_INVALID_VALUE;
}

static int validate_int(char const** number_ptr) {
    assert(ISDIGIT1TO9(**number_ptr));
    (*number_ptr)++;
    while (ISDIGIT(**number_ptr))
        (*number_ptr)++;
    if (**number_ptr == '.')
        return validate_frac(number_ptr);
    else if (**number_ptr == 'e' || **number_ptr == 'E')
        return validate_exp(number_ptr);
    else if (ISWHITESPACE(**number_ptr) || **number_ptr == '\0')
        return LEPT_PARSE_OK;
    else
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
}

static int validate_number(const char* number) {
    if (*number == '-')
        number++;

    if (*number == '0') {
        number++;
        if (*number == '.')
            return validate_frac(&number);
        else if (*number == 'e' || *number == 'E')
            return validate_exp(&number);
        else if (*number == '\0' || ISWHITESPACE(*number))
            return LEPT_PARSE_OK;
        else
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    } else if (ISDIGIT1TO9(*number))
        return validate_int(&number);
    else
        return LEPT_PARSE_INVALID_VALUE;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    int ret;
    if (ret = validate_number(c->json))
        return ret;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
    case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
    case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
    case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
    default:   return lept_parse_number(c, v);
    case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
