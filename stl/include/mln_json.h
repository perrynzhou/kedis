
/*
 * Copyright (C) Niklaus F.Schen.
 */

#ifndef __MLN_JSON_H
#define __MLN_JSON_H

#include "mln_string.h"
#include "mln_alloc.h"
#include "mln_hash.h"
#include "mln_rbtree.h"

#define M_JSON_HASH_LEN         31

#define M_JSON_V_FALSE          0
#define M_JSON_V_TRUE           1
#define M_JSON_V_NULL           NULL

typedef struct mln_json_s mln_json_t;

enum json_type {
    M_JSON_NONE = 0,
    M_JSON_OBJECT,
    M_JSON_ARRAY,
    M_JSON_STRING,
    M_JSON_NUM,
    M_JSON_TRUE,
    M_JSON_FALSE,
    M_JSON_NULL
};

typedef struct {
    mln_json_t                  *key;
    mln_json_t                  *val;
} mln_json_obj_t;

struct mln_json_s {
    mln_uauto_t                  index;
    enum json_type               type;
    union {
        mln_hash_t       *m_j_obj;
        mln_rbtree_t     *m_j_array;
        mln_string_t     *m_j_string;
        double            m_j_number;
        mln_u8_t          m_j_true;
        mln_u8_t          m_j_false;
        mln_u8ptr_t       m_j_null;
    }                            data;
};

#define M_JSON_IS_OBJECT(json)                 ((json)->type == M_JSON_OBJECT)
#define M_JSON_IS_ARRAY(json)                  ((json)->type == M_JSON_ARRAY)
#define M_JSON_IS_STRING(json)                 ((json)->type == M_JSON_STRING)
#define M_JSON_IS_NUMBER(json)                 ((json)->type == M_JSON_NUM)
#define M_JSON_IS_TRUE(json)                   ((json)->type == M_JSON_TRUE)
#define M_JSON_IS_FALSE(json)                  ((json)->type == M_JSON_FALSE)
#define M_JSON_IS_NULL(json)                   ((json)->type == M_JSON_NULL)
#define M_JSON_IS_NONE(json)                   ((json)->type == M_JSON_NONE)

#define M_JSON_GET_DATA_OBJECT(json)           ((json)->data.m_j_obj)
#define M_JSON_GET_DATA_ARRAY(json)            ((json)->data.m_j_array)
#define M_JSON_GET_DATA_STRING(json)           ((json)->data.m_j_string)
#define M_JSON_GET_DATA_NUMBER(json)           ((json)->data.m_j_number)
#define M_JSON_GET_DATA_TRUE(json)             ((json)->data.m_j_true)
#define M_JSON_GET_DATA_FALSE(json)            ((json)->data.m_j_false)
#define M_JSON_GET_DATA_NULL(json)             ((json)->data.m_j_null)

#define M_JSON_SET_INDEX(json,i)               (json)->index = (i)

#define M_JSON_SET_TYPE_NONE(json)             (json)->type = M_JSON_NONE
#define M_JSON_SET_TYPE_OBJECT(json)           (json)->type = M_JSON_OBJECT
#define M_JSON_SET_TYPE_ARRAY(json)            (json)->type = M_JSON_ARRAY
#define M_JSON_SET_TYPE_STRING(json)           (json)->type = M_JSON_STRING
#define M_JSON_SET_TYPE_NUMBER(json)           (json)->type = M_JSON_NUM
#define M_JSON_SET_TYPE_TRUE(json)             (json)->type = M_JSON_TRUE
#define M_JSON_SET_TYPE_FALSE(json)            (json)->type = M_JSON_FALSE
#define M_JSON_SET_TYPE_NULL(json)             (json)->type = M_JSON_NULL

#define M_JSON_SET_DATA_STRING(json,str)       (json)->data.m_j_string = (str)
#define M_JSON_SET_DATA_NUMBER(json,num)       (json)->data.m_j_number = (num)
#define M_JSON_SET_DATA_TRUE(json)             (json)->data.m_j_true = 1
#define M_JSON_SET_DATA_FALSE(json)            (json)->data.m_j_false = 1
#define M_JSON_SET_DATA_NULL(json)             (json)->data.m_j_null = NULL

extern mln_json_t *mln_json_new(void);
extern mln_json_t *mln_json_parse(mln_string_t *jstr);
extern void mln_json_free(void *json);
extern void mln_json_dump(mln_json_t *j, int n_space, char *prefix);
extern mln_string_t *mln_json_generate(mln_json_t *j);
extern mln_json_t *mln_json_value_search(mln_json_t *j, mln_string_t *key);
extern mln_json_t *mln_json_element_search(mln_json_t *j, mln_uauto_t index);
extern mln_uauto_t mln_json_array_length(mln_json_t *j);
extern int mln_json_obj_update(mln_json_t *j, mln_json_t *key, mln_json_t *val);
extern int mln_json_element_add(mln_json_t *j, mln_json_t *value);
extern int mln_json_element_update(mln_json_t *j, mln_json_t *value, mln_uauto_t index);
extern void mln_json_reset(mln_json_t *j);
extern mln_json_t *mln_json_obj_remove(mln_json_t *j, mln_string_t *key);
extern mln_json_t *mln_json_element_remove(mln_json_t *j, mln_uauto_t index);

#endif

