/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Identifiers.
 */

#ifndef FWK_ID_H
#define FWK_ID_H

#include <internal/fwk_id.h>

#include <fwk_attributes.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupLibFramework Framework
 * \{
 */

/*!
 * \defgroup GroupId Identifiers
 * \{
 */

/*!
 * \brief Identifier type.
 */
enum fwk_id_type {
    /*! None */
    FWK_ID_TYPE_NONE = __FWK_ID_TYPE_NONE,
    /*! Module */
    FWK_ID_TYPE_MODULE = __FWK_ID_TYPE_MODULE,
    /*! Element */
    FWK_ID_TYPE_ELEMENT = __FWK_ID_TYPE_ELEMENT,
    /*! Sub-element */
    FWK_ID_TYPE_SUB_ELEMENT = __FWK_ID_TYPE_SUB_ELEMENT,
    /*! API */
    FWK_ID_TYPE_API = __FWK_ID_TYPE_API,
    /*! Event */
    FWK_ID_TYPE_EVENT = __FWK_ID_TYPE_EVENT,
    /*! Notification */
    FWK_ID_TYPE_NOTIFICATION = __FWK_ID_TYPE_NOTIFICATION,
};

/*!
 * \brief Build a 'none' identifier.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Build an id that explicitly refers to nothing.
 *
 *      Example usage:
 *      \code{.c}
 *      static const fwk_id_t unknown = FWK_ID_NONE_INIT;
 *      \endcode
 *
 * \return invalid identifier.
 */
#define FWK_ID_NONE_INIT \
    { \
        .common = { \
            .type = (uint32_t)__FWK_ID_TYPE_NONE, \
        }, \
    }

/*!
 * \brief Build a 'none' identifier.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Build an id that explicitly refers to nothing.
 *
 *      Example usage:
 *      \code{.c}
 *      fwk_id_t get_id(void)
 *      {
 *          if (...)
 *              return FWK_ID_NONE;
 *      }
 *      \endcode
 *
 * \return invalid identifier.
 */
#define FWK_ID_NONE ((fwk_id_t)FWK_ID_NONE_INIT)

/*!
 * \brief Build a module identifier from a module index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_module = FWK_ID_MODULE_INIT(42);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 *
 * \return Module identifier.
 */
#define FWK_ID_MODULE_INIT(MODULE_IDX) \
    { \
        .common = { \
            .type = (uint32_t)__FWK_ID_TYPE_MODULE, \
            .module_idx = (uint32_t)MODULE_IDX, \
        }, \
    }

/*!
 * \brief Build a module identifier from a module index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_module_42(void)
 *      {
 *          return FWK_ID_MODULE(42);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 *
 * \return Module identifier.
 */
#define FWK_ID_MODULE(MODULE_IDX) ((fwk_id_t)FWK_ID_MODULE_INIT(MODULE_IDX))

/*!
 * \brief Build an element identifier from a module index and an element index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_element = FWK_ID_ELEMENT_INIT(42, 58);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param ELEMENT_IDX Element index.
 *
 * \return Element identifier.
 */
#define FWK_ID_ELEMENT_INIT(MODULE_IDX, ELEMENT_IDX) \
    { \
        .element = { \
            .type = (uint32_t)__FWK_ID_TYPE_ELEMENT, \
            .module_idx = (uint32_t)MODULE_IDX, \
            .element_idx = (uint32_t)ELEMENT_IDX, \
        }, \
    }

/*!
 * \brief Build an element identifier from a module index and an element index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_element_42_58(void)
 *      {
 *          return FWK_ID_ELEMENT(42, 58);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param ELEMENT_IDX Element index.
 *
 * \return Element identifier.
 */
#define FWK_ID_ELEMENT(MODULE_IDX, ELEMENT_IDX) \
    ((fwk_id_t)FWK_ID_ELEMENT_INIT(MODULE_IDX, ELEMENT_IDX))

/*!
 * \brief Build a sub-element identifier from a module index, an element index
 *      and a sub-element index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_sub_element =
            FWK_ID_SUB_ELEMENT_INIT(42, 58, 1);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param ELEMENT_IDX Element index.
 * \param SUB_ELEMENT_IDX Sub-element index.
 *
 * \return Sub-element identifier.
 */
#define FWK_ID_SUB_ELEMENT_INIT(MODULE_IDX, ELEMENT_IDX, SUB_ELEMENT_IDX) \
    { \
        .sub_element = { \
            .type = (uint32_t)__FWK_ID_TYPE_SUB_ELEMENT, \
            .module_idx = (uint32_t)MODULE_IDX, \
            .element_idx = (uint32_t)ELEMENT_IDX, \
            .sub_element_idx = (uint32_t)SUB_ELEMENT_IDX, \
        }, \
    }

/*!
 * \brief Build an sub-element identifier from a module index, an element index
 *      and a sub-element index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_sub_element_42_58_1(void)
 *      {
 *          return FWK_ID_SUB_ELEMENT(42, 58, 1);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param ELEMENT_IDX Element index.
 * \param SUB_ELEMENT_IDX Sub-element index.
 *
 * \return Sub-element identifier.
 */
#define FWK_ID_SUB_ELEMENT(MODULE_IDX, ELEMENT_IDX, SUB_ELEMENT_IDX) \
    ((fwk_id_t)FWK_ID_SUB_ELEMENT_INIT( \
        MODULE_IDX, ELEMENT_IDX, SUB_ELEMENT_IDX))

/*!
 * \brief Build an API identifier from a module index and an API index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_api = FWK_ID_API_INIT(42, 14);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param API_IDX API index.
 *
 * \return API identifier.
 */
#define FWK_ID_API_INIT(MODULE_IDX, API_IDX) \
    { \
        .api = { \
            .type = (uint32_t)__FWK_ID_TYPE_API, \
            .module_idx = (uint32_t)MODULE_IDX, \
            .api_idx = (uint32_t)API_IDX, \
        }, \
    }

/*!
 * \brief Build an API identifier from a module index and an API index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_api_42_14(void)
 *      {
 *          return FWK_ID_API(42, 14);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param API_IDX API index.
 *
 * \return API identifier.
 */
#define FWK_ID_API(MODULE_IDX, API_IDX) \
    ((fwk_id_t)FWK_ID_API_INIT(MODULE_IDX, API_IDX))

/*!
 * \brief Build an event identifier from a module index and an event index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_event = FWK_ID_EVENT_INIT(42, 56);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param EVENT_IDX Event index.
 *
 * \return Event identifier.
 */
#define FWK_ID_EVENT_INIT(MODULE_IDX, EVENT_IDX) \
    { \
        .event = { \
            .type = (uint32_t)__FWK_ID_TYPE_EVENT, \
            .module_idx = (uint32_t)MODULE_IDX, \
            .event_idx = (uint32_t)EVENT_IDX, \
        }, \
    }

/*!
 * \brief Build an event identifier from a module index and an event index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_event_42_56(void)
 *      {
 *          return FWK_ID_EVENT(42, 56);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param EVENT_IDX Event index.
 *
 * \return Event identifier.
 */
#define FWK_ID_EVENT(MODULE_IDX, EVENT_IDX) \
    ((fwk_id_t)FWK_ID_EVENT_INIT(MODULE_IDX, EVENT_IDX))

/*!
 * \brief Build a notification identifier from a module index and a
 *      notification index.
 *
 * \note This macro expands to a designated initializer, and can be used to
 *      initialize a ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      static const fwk_id_t my_notification =
            FWK_ID_NOTIFICATION_INIT(42, 56);
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param NOTIFICATION_IDX Notification index.
 *
 * \return Notification identifier.
 */
#define FWK_ID_NOTIFICATION_INIT(MODULE_IDX, NOTIFICATION_IDX) \
    { \
        .notification = { \
            .type = (uint32_t)__FWK_ID_TYPE_NOTIFICATION, \
            .module_idx = (uint32_t)MODULE_IDX, \
            .notification_idx = (uint32_t)NOTIFICATION_IDX, \
        }, \
    }

/*!
 * \brief Build an notification identifier from a module index and a
 *      notification index.
 *
 * \note This macro expands to a compound literal, and can be used as an lvalue
 *      expression returning type ::fwk_id_t.
 *
 * \details Example usage:
 *      \code{.c}
 *      fwk_id_t get_notification_42_56(void)
 *      {
 *          return FWK_ID_NOTIFICATION(42, 56);
 *      }
 *      \endcode
 *
 * \param MODULE_IDX Module index.
 * \param NOTIFICATION_IDX Notification index.
 *
 * \return Notification identifier.
 */
#define FWK_ID_NOTIFICATION(MODULE_IDX, NOTIFICATION_IDX) \
    ((fwk_id_t)FWK_ID_NOTIFICATION_INIT(MODULE_IDX, NOTIFICATION_IDX))

/*!
 * \brief Build a string representation of an identifier.
 *
 * \details Strings returned by this macro are in the format <tt>[MMM]</tt> for
 *      module identifiers and <tt>[MMM:EEEE]</tt> for element identifiers,
 *      where \c M refers to the module index and \c E refers to the element
 *      index.
 *
 * \param ID Module or element identifier.
 *
 * \return String representation of the identifier.
 *
 * \hideinitializer
 */
#define FWK_ID_STR(ID) (__fwk_id_str(ID).str)

/*!
 * \brief Generic identifier.
 */
typedef union __fwk_id fwk_id_t;

/*!
 * \brief Generic optional identifier.
 *
 * \details Optionals identifiers are fwk_id_t elements which can be left
 *      undefined. This are only intended to be used for configuration
 *      proposes and should not be used to bypass fwk_id_t checks.
 */
typedef fwk_id_t fwk_optional_id_t;

/*!
 * \brief Check if the identifier type is valid.
 *
 * \param id Identifier.
 *
 * \retval true The identifier type is valid.
 * \retval false The identifier type is not valid.
 */
bool fwk_id_type_is_valid(fwk_id_t id) FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Check if the identifier is of a certain identifier type.
 *
 * \param id Identifier.
 * \param type Identifier type.
 *
 * \retval true The identifier is of the type specified.
 * \retval false The identifier is not of the type specified.
 */
bool fwk_id_is_type(fwk_id_t id, enum fwk_id_type type) FWK_CONST FWK_LEAF
    FWK_NOTHROW;

/*!
 * \brief Retrieve the type of an identifier.
 *
 * \param id Identifier.
 *
 * \return Identifier type.
 */
enum fwk_id_type fwk_id_get_type(fwk_id_t id) FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Check if two identifiers refer to the same entity.
 *
 * \param left First identifier.
 * \param right Second identifier.
 *
 * \retval true The identifiers refer to the same entity.
 * \retval false The identifiers do not refer to the same entity.
 */
bool fwk_id_is_equal(fwk_id_t left, fwk_id_t right) FWK_CONST FWK_LEAF
    FWK_NOTHROW;

/*!
 * \brief Check if an optional identifier is defined.
 *
 * \param id Optional identifier.
 *
 * \retval true The optional identifiers is defined.
 * \retval false The optional identifiers is not defined.
 */
bool fwk_optional_id_is_defined(fwk_optional_id_t id) FWK_CONST FWK_LEAF
    FWK_NOTHROW;

/*!
 * \brief Retrieve the identifier of the module that owns a given identifier.
 *
 * \note If the given identifier already refers to a module, the returned
 *     identifier refers to that same module.
 *
 * \param id Identifier.
 *
 * \return Identifier of the owning module.
 */
fwk_id_t fwk_id_build_module_id(fwk_id_t id) FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Retrieve the identifier of an element for a given identifier and
 *      element index.
 *
 * \details Usually the given identifier will be a module identifier and the
 *      function will be used to build an element identifier for an element with
 *      the given index that is owned by that module.
 *
 * \note If the identifier provided refers to an element, sub-element, API,
 *      event, or notification instead of a module then the parent module of
 *      the identifier is used when building the returned element identifier.
 *
 * \param id Identifier.
 * \param element_idx Element index.
 *
 * \return Element identifier associated with the element index for the module.
 */
fwk_id_t fwk_id_build_element_id(fwk_id_t id, unsigned int element_idx)
    FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Retrieve the identifier of a sub-element for a given element
 *      identifier and sub-element index.
 *
 * \details The given identifier must be an element identifier and the
 *      function will be used to build an sub-element identifier for a
 *      sub-element with the given index that is owned by that element.
 *
 * \param id Identifier.
 * \param sub_element_idx Sub-element index.
 *
 * \return Sub-element identifier associated with the sub-element index for the
 *      element.
 */
fwk_id_t fwk_id_build_sub_element_id(fwk_id_t id, unsigned int sub_element_idx)
    FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Retrieve the identifier of an API for a given identifier and
 *      API index.
 *
 * \details Usually the given identifier will be a module identifier and the
 *      function will be used to build an API identifier for an API with the
 *      given index that is owned by that module.
 *
 * \note If the identifier provided refers to an element, sub-element, API,
 *      event, or notification instead of a module then the parent module of
 *      the identifier is used when building the returned API identifier.
 *
 * \param id Identifier.
 * \param api_idx API index.
 *
 * \return API identifier associated with the API index for the module.
 */
fwk_id_t fwk_id_build_api_id(fwk_id_t id, unsigned int api_idx)
    FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Retrieve the module index of an identifier.
 *
 * \param id Identifier.
 *
 * \return Module index.
 *
 * \note Since this function is used very frequently, inlining is useful
 *     to avoid function call overhead. The inlining is applicable only in
 *     release build.
 */
#if !defined(NDEBUG)
unsigned int fwk_id_get_module_idx(fwk_id_t id) FWK_CONST FWK_LEAF FWK_NOTHROW;
#else
inline static unsigned int fwk_id_get_module_idx(fwk_id_t id)
{
    return id.common.module_idx;
}
#endif

/*!
 * \brief Retrieve the index of an element from its identifier or the identifier
 *      of one of its sub-element.
 *
 * \param element_id Element or sub-element identifier.
 *
 * \return Element index.
 *
 * \note Since this function is used very frequently, inlining is useful
 *     to avoid function call overhead. The inlining is applicable only in
 *     release build.
 */
#if !defined(NDEBUG)
unsigned int fwk_id_get_element_idx(fwk_id_t element_id) FWK_CONST FWK_LEAF
    FWK_NOTHROW;
#else
inline static unsigned int fwk_id_get_element_idx(fwk_id_t element_id)
{
    return element_id.element.element_idx;
}
#endif

/*!
 * \brief Retrieve the index of a sub-element from its identifier.
 *
 * \param sub_element_id Sub-element identifier.
 *
 * \return Sub-element index.
 *
 * \note Since this function is used very frequently, inlining is useful
 *     to avoid function call overhead. The inlining is applicable only in
 *     release build.
 */
#if !defined(NDEBUG)
unsigned int fwk_id_get_sub_element_idx(
    fwk_id_t sub_element_id) FWK_CONST FWK_LEAF FWK_NOTHROW;
#else
inline static unsigned int fwk_id_get_sub_element_idx(fwk_id_t sub_element_id)
{
    return sub_element_id.sub_element.sub_element_idx;
}
#endif

/*!
 * \brief Retrieve the index of an API from its identifier.
 *
 * \param api_id API identifier.
 *
 * \return API index.
 */
unsigned int fwk_id_get_api_idx(fwk_id_t api_id) FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \brief Retrieve the index of an event from its identifier.
 *
 * \param event_id Event identifier.
 *
 * \return Event index.
 */
unsigned int fwk_id_get_event_idx(fwk_id_t event_id) FWK_CONST FWK_LEAF
    FWK_NOTHROW;

/*!
 * \brief Retrieve the index of a notification from its identifier.
 *
 * \param notification_id Notification identifier.
 *
 * \return Notification index.
 */
unsigned int fwk_id_get_notification_idx(
    fwk_id_t notification_id) FWK_CONST FWK_LEAF FWK_NOTHROW;

/*!
 * \internal
 *
 * \brief Verbose identifier string helper structure.
 *
 * \details C does not support returning an array from a function. This
 *      structure hides the array within a structure, and instead the structure
 *      is returned to the caller. This allows them to access the array without
 *      it decaying to a pointer when returned.
 */
struct fwk_id_verbose_fmt {
    char str[128]; /*!< Identifier string representation. */
};

/*!
 * \internal
 *
 * \brief Generate a verbose string description of an identifier.
 *
 * \details Generates a string description of an identifier, including the names
 *      of any entities that form part of the identifier.
 *
 * \param[in] id Identifier to describe.
 *
 * \return Verbose string representation of the identifier.
 */
struct fwk_id_verbose_fmt fwk_id_verbose_str(fwk_id_t id);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* FWK_ID_H */
