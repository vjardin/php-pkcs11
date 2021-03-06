/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Guillaume Amringer                                           |
   +----------------------------------------------------------------------+
*/

#include "pkcs11int.h"

zend_class_entry *ce_Pkcs11_DigestContext;
static zend_object_handlers pkcs11_digestcontext_handlers;

ZEND_BEGIN_ARG_INFO_EX(arginfo_update, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_keyUpdate, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, key, IS_OBJECT, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_finalize, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(DigestContext, update) {

    CK_RV rv;
    zend_string *data;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_STR(data)
    ZEND_PARSE_PARAMETERS_END();

    pkcs11_digestcontext_object *objval = Z_PKCS11_DIGESTCONTEXT_P(ZEND_THIS);

    rv = objval->session->pkcs11->functionList->C_DigestUpdate(
        objval->session->session,
        ZSTR_VAL(data),
        ZSTR_LEN(data)
    );
    if (rv != CKR_OK) {
        pkcs11_error(rv, "Unable to update digest");
        return;
    }
}

PHP_METHOD(DigestContext, keyUpdate) {

    CK_RV rv;
    zval *key;

    ZEND_PARSE_PARAMETERS_START(1,1)
        Z_PARAM_ZVAL(key)
    ZEND_PARSE_PARAMETERS_END();

    pkcs11_digestcontext_object *objval = Z_PKCS11_DIGESTCONTEXT_P(ZEND_THIS);
    pkcs11_key_object *keyobjval = Z_PKCS11_KEY_P(key);

    rv = objval->session->pkcs11->functionList->C_DigestKey(
        objval->session->session,
        keyobjval->key
    );
    if (rv != CKR_OK) {
        pkcs11_error(rv, "Unable to update digest with key");
        return;
    }
}

PHP_METHOD(DigestContext, finalize) {

    CK_RV rv;

    ZEND_PARSE_PARAMETERS_START(0,0)
    ZEND_PARSE_PARAMETERS_END();

    pkcs11_digestcontext_object *objval = Z_PKCS11_DIGESTCONTEXT_P(ZEND_THIS);

    CK_ULONG digestLen;
    rv = objval->session->pkcs11->functionList->C_DigestFinal(
        objval->session->session,
        NULL_PTR,
        &digestLen
    );
    if (rv != CKR_OK) {
        pkcs11_error(rv, "Unable to finalize digest");
        return;
    }

    CK_BYTE_PTR digest = ecalloc(digestLen, sizeof(CK_BYTE));
    rv = objval->session->pkcs11->functionList->C_DigestFinal(
        objval->session->session,
        digest,
        &digestLen
    );
    if (rv != CKR_OK) {
        pkcs11_error(rv, "Unable to finalize digest");
        return;
    }

    zend_string *returnval;
    returnval = zend_string_alloc(digestLen, 0);
    memcpy(
        ZSTR_VAL(returnval),
        digest,
        digestLen
    );
    efree(digest);

    RETURN_STR(returnval);
}

void pkcs11_digestcontext_shutdown(pkcs11_digestcontext_object *obj) {
    GC_DELREF(&obj->session->std);
}

static zend_function_entry digestcontext_class_functions[] = {
    PHP_ME(DigestContext, update, arginfo_update, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(DigestContext, keyUpdate, arginfo_keyUpdate, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(DigestContext, finalize, arginfo_finalize, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_FE_END
};


DEFINE_MAGIC_FUNCS(pkcs11_digestcontext, digestcontext, DigestContext)
