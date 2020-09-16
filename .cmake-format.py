#
# Arm SCP/MCP Software
# Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# -----------------------------
# Options affecting formatting.
# -----------------------------
with section("format"):

  # If <use_tabchars> is True, then the value of this variable indicates how
  # fractional indentions are handled during whitespace replacement. If set to
  # 'use-space', fractional indentation is left as spaces (utf-8 0x20). If set
  # to `round-up` fractional indentation is replaced with a single tab character
  # (utf-8 0x09) effectively shifting the column to the next tabstop
  fractional_tab_policy = u'use-space'

  # Format command names consistently as 'lower' or 'upper' case
  command_case = u'canonical'

  # If the statement spelling length (including space and parenthesis) is larger
  # than the tab width by more than this amount, then force reject un-nested
  # layouts.
  max_prefix_chars = 10

  # By default, if cmake-format cannot successfully fit everything into the
  # desired linewidth it will apply the last, most agressive attempt that it
  # made. If this flag is True, however, cmake-format will print error, exit
  # with non-zero status code, and write-out nothing
  require_valid_layout = False

  # If the trailing parenthesis must be 'dangled' on its on line, then align it
  # to this reference: `prefix`: the start of the statement,  `prefix-indent`:
  # the start of the statement, plus one indentation  level, `child`: align to
  # the column of the arguments
  dangle_align = u'prefix'

  # If an argument group contains more than this many sub-groups (parg or kwarg
  # groups) then force it to a vertical layout.
  max_subgroups_hwrap = 2

  # Disable formatting entirely, making cmake-format a no-op
  disable = False

  # If the statement spelling length (including space and parenthesis) is
  # smaller than this amount, then force reject nested layouts.
  min_prefix_chars = 4

  # If a positional argument group contains more than this many arguments, then
  # force it to a vertical layout.
  max_pargs_hwrap = 6

  # If a candidate layout is wrapped horizontally but it exceeds this many
  # lines, then reject the layout.
  max_lines_hwrap = 2

  # If true, the parsers may infer whether or not an argument list is sortable
  # (without annotation).
  autosort = False

  # What style line endings to use in the output.
  line_ending = u'unix'

  # How wide to allow formatted cmake files
  line_width = 80

  # If a statement is wrapped to more than one line, than dangle the closing
  # parenthesis on its own line.
  dangle_parens = False

  # A list of command names which should always be wrapped
  always_wrap = []

  # If true, separate flow control names from their parentheses with a space
  separate_ctrl_name_with_space = False

  # If a cmdline positional group consumes more than this many lines without
  # nesting, then invalidate the layout (and nest)
  max_rows_cmdline = 2

  # If true, separate function names from parentheses with a space
  separate_fn_name_with_space = False

  # How many spaces to tab for indent
  tab_size = 4

  # If true, lines are indented using tab characters (utf-8 0x09) instead of
  # <tab_size> space characters (utf-8 0x20). In cases where the layout would
  # require a fractional tab character, the behavior of the  fractional
  # indentation is governed by <fractional_tab_policy>
  use_tabchars = False

  # Format keywords consistently as 'lower' or 'upper' case
  keyword_case = u'unchanged'

  # If true, the argument lists which are known to be sortable will be sorted
  # lexicographicall
  enable_sort = True

  # A dictionary mapping layout nodes to a list of wrap decisions. See the
  # documentation for more information.
  layout_passes = { }

# ------------------------------------------------
# Options affecting comment reflow and formatting.
# ------------------------------------------------
with section("markup"):

  # If comment markup is enabled, don't reflow any comment block which matches
  # this (regex) pattern. Default is `None` (disabled).
  literal_comment_pattern = None

  # If a comment line starts with at least this many consecutive hash
  # characters, then don't lstrip() them off. This allows for lazy hash rulers
  # where the first hash char is not separated by space
  hashruler_min_length = 10

  # Regular expression to match preformat fences in comments default=
  # ``r'^\s*([`~]{3}[`~]*)(.*)$'``
  fence_pattern = u'^\\s*([`~]{3}[`~]*)(.*)$'

  # If true, then insert a space between the first hash char and remaining hash
  # chars in a hash ruler, and normalize its length to fill the column
  canonicalize_hashrulers = True

  # If a comment line matches starts with this pattern then it is explicitly a
  # trailing comment for the preceeding argument. Default is '#<'
  explicit_trailing_pattern = u'#<'

  # If comment markup is enabled, don't reflow the first comment block in each
  # listfile. Use this to preserve formatting of your copyright/license
  # statements.
  first_comment_is_literal = True

  # enable comment markup parsing and reflow
  enable_markup = True

  # Regular expression to match rulers in comments default=
  # ``r'^\s*[^\w\s]{3}.*[^\w\s]{3}$'``
  ruler_pattern = u'^\\s*[^\\w\\s]{3}.*[^\\w\\s]{3}$'

  # What character to use as punctuation after numerals in an enumerated list
  enum_char = u'.'

  # What character to use for bulleted lists
  bullet_char = u'*'

# ----------------------------
# Options affecting the linter
# ----------------------------
with section("lint"):

  # regular expression pattern describing valid function names
  function_pattern = u'[0-9a-z_]+'

  # regular expression pattern describing valid names for function/macro
  # arguments and loop variables.
  argument_var_pattern = u'[a-z][a-z0-9_]+'

  # a list of lint codes to disable
  disabled_codes = []

  # Require at least this many newlines between statements
  min_statement_spacing = 1

  # regular expression pattern describing valid macro names
  macro_pattern = u'scp_[0-9a-z_]+'

  # regular expression pattern describing valid names for public directory
  # variables
  public_var_pattern = u'(CMAKE|SCP)_[A-Z][0-9A-Z_]+'
  max_statements = 50

  # In the heuristic for C0201, how many conditionals to match within a loop in
  # before considering the loop a parser.
  max_conditionals_custom_parser = 2

  # regular expression pattern describing valid names for variables with global
  # (cache) scope
  global_var_pattern = u'(CMAKE|SCP)_[A-Z][0-9A-Z_]+'

  # regular expression pattern describing valid names for keywords used in
  # functions or macros
  keyword_pattern = u'[A-Z][0-9A-Z_]+'
  max_arguments = 5

  # regular expression pattern describing valid names for privatedirectory
  # variables
  private_var_pattern = u'scp_[0-9a-z_]+'
  max_localvars = 15
  max_branches = 12

  # regular expression pattern describing valid names for variables with local
  # scope
  local_var_pattern = u'[a-z][a-z0-9_]+'

  # Require no more than this many newlines between statements
  max_statement_spacing = 2

  # regular expression pattern describing valid names for variables with global
  # scope (but internal semantic)
  internal_var_pattern = u'(CMAKE|SCP)?_[A-Z][0-9A-Z_]+'
  max_returns = 6

# -------------------------------------
# Miscellaneous configurations options.
# -------------------------------------
with section("misc"):

  # A dictionary containing any per-command configuration overrides. Currently
  # only `command_case` is supported.
  per_command = { }

# ----------------------------------
# Options affecting listfile parsing
# ----------------------------------
with section("parse"):

  # Specify structure for custom cmake functions
  additional_commands = {
    "set_property": {
      "kwargs": {
        "GLOBAL": {
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "ALLOW_DUPLICATE_CUSTOM_TARGETS": "*",
                "AUTOGEN_SOURCE_GROUP": "*",
                "AUTOGEN_TARGETS_FOLDER": "*",
                "AUTOMOC_SOURCE_GROUP": "*",
                "AUTOMOC_TARGETS_FOLDER": "*",
                "AUTORCC_SOURCE_GROUP": "*",
                "CMAKE_C_KNOWN_FEATURES": "*",
                "CMAKE_CUDA_KNOWN_FEATURES": "*",
                "CMAKE_CXX_KNOWN_FEATURES": "*",
                "CMAKE_ROLE": "*",
                "DEBUG_CONFIGURATIONS": "*",
                "DISABLED_FEATURES": "*",
                "ECLIPSE_EXTRA_CPROJECT_CONTENTS": "*",
                "ECLIPSE_EXTRA_NATURES": "*",
                "ENABLED_FEATURES": "*",
                "ENABLED_LANGUAGES": "*",
                "FIND_LIBRARY_USE_LIB32_PATHS": "*",
                "FIND_LIBRARY_USE_LIB64_PATHS": "*",
                "FIND_LIBRARY_USE_LIBX32_PATHS": "*",
                "FIND_LIBRARY_USE_OPENBSD_VERSIONING": "*",
                "GENERATOR_IS_MULTI_CONFIG": "*",
                "GLOBAL_DEPENDS_DEBUG_MODE": "*",
                "GLOBAL_DEPENDS_NO_CYCLES": "*",
                "IN_TRY_COMPILE": "*",
                "JOB_POOLS": "*",
                "PACKAGES_FOUND": "*",
                "PACKAGES_NOT_FOUND": "*",
                "PREDEFINED_TARGETS_FOLDER": "*",
                "REPORT_UNDEFINED_PROPERTIES": "*",
                "RULE_LAUNCH_COMPILE": "*",
                "RULE_LAUNCH_CUSTOM": "*",
                "RULE_LAUNCH_LINK": "*",
                "RULE_MESSAGES": "*",
                "TARGET_ARCHIVES_MAY_BE_SHARED_LIBS": "*",
                "TARGET_MESSAGES": "*",
                "TARGET_SUPPORTS_SHARED_LIBS": "*",
                "USE_FOLDERS": "*",
                "XCODE_EMIT_EFFECTIVE_PLATFORM_NAME": "*",
              },
            },
          },
        },
        "DIRECTORY": {
          "pargs": 1,
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "ADDITIONAL_CLEAN_FILES": "*",
                "BINARY_DIR": "*",
                "BUILDSYSTEM_TARGETS": "*",
                "CACHE_VARIABLES": "*",
                "CLEAN_NO_CUSTOM": "*",
                "CMAKE_CONFIGURE_DEPENDS": "*",
                "COMPILE_DEFINITIONS": "*",
                "COMPILE_OPTIONS": "*",
                "DEFINITIONS": "*",
                "EXCLUDE_FROM_ALL": "*",
                "IMPLICIT_DEPENDS_INCLUDE_TRANSFORM": "*",
                "INCLUDE_DIRECTORIES": "*",
                "INCLUDE_REGULAR_EXPRESSION": "*",
                "INTERPROCEDURAL_OPTIMIZATION_DEBUG": "*",
                "INTERPROCEDURAL_OPTIMIZATION_RELEASE": "*",
                "INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL": "*",
                "INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO": "*",
                "INTERPROCEDURAL_OPTIMIZATION": "*",
                "LABELS": "*",
                "LINK_DIRECTORIES": "*",
                "LINK_OPTIONS": "*",
                "LISTFILE_STACK": "*",
                "MACROS": "*",
                "PARENT_DIRECTORY": "*",
                "RULE_LAUNCH_COMPILE": "*",
                "RULE_LAUNCH_CUSTOM": "*",
                "RULE_LAUNCH_LINK": "*",
                "SOURCE_DIR": "*",
                "SUBDIRECTORIES": "*",
                "TESTS": "*",
                "TEST_INCLUDE_FILES": "*",
                "VARIABLES": "*",
                "VS_GLOBAL_SECTION_POST_ExtensibilityAddIns": "*",
                "VS_GLOBAL_SECTION_POST_ExtensibilityGlobals": "*",
                "VS_GLOBAL_SECTION_PRE_ExtensibilityAddIns": "*",
                "VS_GLOBAL_SECTION_PRE_ExtensibilityGlobals": "*",
                "VS_STARTUP_PROJECT": "*",
              },
            },
          },
        },
        "TARGET": {
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "ADDITIONAL_CLEAN_FILES": "*",
                "AIX_EXPORT_ALL_SYMBOLS": "*",
                "ALIAS_GLOBAL": "*",
                "ALIASED_TARGET": "*",
                "ANDROID_ANT_ADDITIONAL_OPTIONS": "*",
                "ANDROID_API": "*",
                "ANDROID_API_MIN": "*",
                "ANDROID_ARCH": "*",
                "ANDROID_ASSETS_DIRECTORIES": "*",
                "ANDROID_GUI": "*",
                "ANDROID_JAR_DEPENDENCIES": "*",
                "ANDROID_JAR_DIRECTORIES": "*",
                "ANDROID_JAVA_SOURCE_DIR": "*",
                "ANDROID_NATIVE_LIB_DEPENDENCIES": "*",
                "ANDROID_NATIVE_LIB_DIRECTORIES": "*",
                "ANDROID_PROCESS_MAX": "*",
                "ANDROID_PROGUARD": "*",
                "ANDROID_PROGUARD_CONFIG_PATH": "*",
                "ANDROID_SECURE_PROPS_PATH": "*",
                "ANDROID_SKIP_ANT_STEP": "*",
                "ANDROID_STL_TYPE": "*",
                "ARCHIVE_OUTPUT_DIRECTORY": "*",
                "ARCHIVE_OUTPUT_NAME": "*",
                "AUTOGEN_BUILD_DIR": "*",
                "AUTOGEN_ORIGIN_DEPENDS": "*",
                "AUTOGEN_PARALLEL": "*",
                "AUTOGEN_TARGET_DEPENDS": "*",
                "AUTOMOC": "*",
                "AUTOMOC_COMPILER_PREDEFINES": "*",
                "AUTOMOC_DEPEND_FILTERS": "*",
                "AUTOMOC_EXECUTABLE": "*",
                "AUTOMOC_MACRO_NAMES": "*",
                "AUTOMOC_MOC_OPTIONS": "*",
                "AUTOMOC_PATH_PREFIX": "*",
                "AUTORCC": "*",
                "AUTORCC_EXECUTABLE": "*",
                "AUTORCC_OPTIONS": "*",
                "AUTOUIC": "*",
                "AUTOUIC_EXECUTABLE": "*",
                "AUTOUIC_OPTIONS": "*",
                "AUTOUIC_SEARCH_PATHS": "*",
                "BINARY_DIR": "*",
                "BUILD_RPATH": "*",
                "BUILD_RPATH_USE_ORIGIN": "*",
                "BUILD_WITH_INSTALL_NAME_DIR": "*",
                "BUILD_WITH_INSTALL_RPATH": "*",
                "BUNDLE_EXTENSION": "*",
                "BUNDLE": "*",
                "C_EXTENSIONS": "*",
                "C_STANDARD": "*",
                "C_STANDARD_REQUIRED": "*",
                "COMMON_LANGUAGE_RUNTIME": "*",
                "COMPATIBLE_INTERFACE_BOOL": "*",
                "COMPATIBLE_INTERFACE_NUMBER_MAX": "*",
                "COMPATIBLE_INTERFACE_NUMBER_MIN": "*",
                "COMPATIBLE_INTERFACE_STRING": "*",
                "COMPILE_DEFINITIONS": "*",
                "COMPILE_FEATURES": "*",
                "COMPILE_FLAGS": "*",
                "COMPILE_OPTIONS": "*",
                "COMPILE_PDB_NAME": "*",
                "COMPILE_PDB_OUTPUT_DIRECTORY": "*",
                "CROSSCOMPILING_EMULATOR": "*",
                "CUDA_ARCHITECTURES": "*",
                "CUDA_PTX_COMPILATION": "*",
                "CUDA_SEPARABLE_COMPILATION": "*",
                "CUDA_RESOLVE_DEVICE_SYMBOLS": "*",
                "CUDA_RUNTIME_LIBRARY": "*",
                "CUDA_EXTENSIONS": "*",
                "CUDA_STANDARD": "*",
                "CUDA_STANDARD_REQUIRED": "*",
                "CXX_EXTENSIONS": "*",
                "CXX_STANDARD": "*",
                "CXX_STANDARD_REQUIRED": "*",
                "DEBUG_POSTFIX": "*",
                "DEFINE_SYMBOL": "*",
                "DEPLOYMENT_REMOTE_DIRECTORY": "*",
                "DEPLOYMENT_ADDITIONAL_FILES": "*",
                "DEPRECATION": "*",
                "DISABLE_PRECOMPILE_HEADERS": "*",
                "DOTNET_TARGET_FRAMEWORK": "*",
                "DOTNET_TARGET_FRAMEWORK_VERSION": "*",
                "EchoString": "*",
                "ENABLE_EXPORTS": "*",
                "EXCLUDE_FROM_ALL": "*",
                "EXCLUDE_FROM_DEFAULT_BUILD": "*",
                "EXPORT_NAME": "*",
                "EXPORT_PROPERTIES": "*",
                "FOLDER": "*",
                "Fortran_FORMAT": "*",
                "Fortran_MODULE_DIRECTORY": "*",
                "Fortran_PREPROCESS": "*",
                "FRAMEWORK": "*",
                "FRAMEWORK_VERSION": "*",
                "GENERATOR_FILE_NAME": "*",
                "GHS_INTEGRITY_APP": "*",
                "GHS_NO_SOURCE_GROUP_FILE": "*",
                "GNUtoMS": "*",
                "HAS_CXX": "*",
                "IMPLICIT_DEPENDS_INCLUDE_TRANSFORM": "*",
                "IMPORTED_COMMON_LANGUAGE_RUNTIME": "*",
                "IMPORTED_CONFIGURATIONS": "*",
                "IMPORTED_GLOBAL": "*",
                "IMPORTED_IMPLIB": "*",
                "IMPORTED_LIBNAME": "*",
                "IMPORTED_LINK_DEPENDENT_LIBRARIES": "*",
                "IMPORTED_LINK_INTERFACE_LANGUAGES": "*",
                "IMPORTED_LINK_INTERFACE_LIBRARIES": "*",
                "IMPORTED_LINK_INTERFACE_MULTIPLICITY": "*",
                "IMPORTED_LOCATION": "*",
                "IMPORTED_NO_SONAME": "*",
                "IMPORTED_OBJECTS": "*",
                "IMPORTED": "*",
                "IMPORTED_SONAME": "*",
                "IMPORT_PREFIX": "*",
                "IMPORT_SUFFIX": "*",
                "INCLUDE_DIRECTORIES": "*",
                "INSTALL_NAME_DIR": "*",
                "INSTALL_REMOVE_ENVIRONMENT_RPATH": "*",
                "INSTALL_RPATH": "*",
                "INSTALL_RPATH_USE_LINK_PATH": "*",
                "INTERFACE_AUTOUIC_OPTIONS": "*",
                "INTERFACE_COMPILE_DEFINITIONS": "*",
                "INTERFACE_COMPILE_FEATURES": "*",
                "INTERFACE_COMPILE_OPTIONS": "*",
                "INTERFACE_INCLUDE_DIRECTORIES": "*",
                "INTERFACE_LINK_DEPENDS": "*",
                "INTERFACE_LINK_DIRECTORIES": "*",
                "INTERFACE_LINK_LIBRARIES": "*",
                "INTERFACE_LINK_OPTIONS": "*",
                "INTERFACE_PRECOMPILE_HEADERS": "*",
                "INTERFACE_POSITION_INDEPENDENT_CODE": "*",
                "INTERFACE_SOURCES": "*",
                "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES": "*",
                "INTERPROCEDURAL_OPTIMIZATION": "*",
                "IOS_INSTALL_COMBINED": "*",
                "JOB_POOL_COMPILE": "*",
                "JOB_POOL_LINK": "*",
                "JOB_POOL_PRECOMPILE_HEADER": "*",
                "LABELS": "*",
                "LIBRARY_OUTPUT_DIRECTORY": "*",
                "LIBRARY_OUTPUT_NAME": "*",
                "LINK_DEPENDS_NO_SHARED": "*",
                "LINK_DEPENDS": "*",
                "LINKER_LANGUAGE": "*",
                "LINK_DIRECTORIES": "*",
                "LINK_FLAGS": "*",
                "LINK_INTERFACE_LIBRARIES": "*",
                "LINK_INTERFACE_MULTIPLICITY": "*",
                "LINK_LIBRARIES": "*",
                "LINK_OPTIONS": "*",
                "LINK_SEARCH_END_STATIC": "*",
                "LINK_SEARCH_START_STATIC": "*",
                "LINK_WHAT_YOU_USE": "*",
                "LOCATION": "*",
                "MACHO_COMPATIBILITY_VERSION": "*",
                "MACHO_CURRENT_VERSION": "*",
                "MACOSX_BUNDLE_INFO_PLIST": "*",
                "MACOSX_BUNDLE": "*",
                "MACOSX_FRAMEWORK_INFO_PLIST": "*",
                "MACOSX_RPATH": "*",
                "MANUALLY_ADDED_DEPENDENCIES": "*",
                "MSVC_RUNTIME_LIBRARY": "*",
                "NAME": "*",
                "NO_SONAME": "*",
                "NO_SYSTEM_FROM_IMPORTED": "*",
                "OBJC_EXTENSIONS": "*",
                "OBJC_STANDARD": "*",
                "OBJC_STANDARD_REQUIRED": "*",
                "OBJCXX_EXTENSIONS": "*",
                "OBJCXX_STANDARD": "*",
                "OBJCXX_STANDARD_REQUIRED": "*",
                "OSX_ARCHITECTURES": "*",
                "OUTPUT_NAME": "*",
                "PCH_WARN_INVALID": "*",
                "PDB_NAME": "*",
                "PDB_OUTPUT_DIRECTORY": "*",
                "POSITION_INDEPENDENT_CODE": "*",
                "PRECOMPILE_HEADERS": "*",
                "PRECOMPILE_HEADERS_REUSE_FROM": "*",
                "PREFIX": "*",
                "PRIVATE_HEADER": "*",
                "PROJECT_LABEL": "*",
                "PUBLIC_HEADER": "*",
                "RESOURCE": "*",
                "RULE_LAUNCH_COMPILE": "*",
                "RULE_LAUNCH_CUSTOM": "*",
                "RULE_LAUNCH_LINK": "*",
                "RUNTIME_OUTPUT_DIRECTORY": "*",
                "RUNTIME_OUTPUT_NAME": "*",
                "SKIP_BUILD_RPATH": "*",
                "SOURCE_DIR": "*",
                "SOURCES": "*",
                "SOVERSION": "*",
                "STATIC_LIBRARY_FLAGS": "*",
                "STATIC_LIBRARY_OPTIONS": "*",
                "SUFFIX": "*",
                "Swift_DEPENDENCIES_FILE": "*",
                "Swift_LANGUAGE_VERSION": "*",
                "Swift_MODULE_DIRECTORY": "*",
                "Swift_MODULE_NAME": "*",
                "TYPE": "*",
                "UNITY_BUILD": "*",
                "UNITY_BUILD_BATCH_SIZE": "*",
                "UNITY_BUILD_CODE_AFTER_INCLUDE": "*",
                "UNITY_BUILD_CODE_BEFORE_INCLUDE": "*",
                "UNITY_BUILD_MODE": "*",
                "VERSION": "*",
                "VISIBILITY_INLINES_HIDDEN": "*",
                "VS_CONFIGURATION_TYPE": "*",
                "VS_DEBUGGER_COMMAND": "*",
                "VS_DEBUGGER_COMMAND_ARGUMENTS": "*",
                "VS_DEBUGGER_ENVIRONMENT": "*",
                "VS_DEBUGGER_WORKING_DIRECTORY": "*",
                "VS_DESKTOP_EXTENSIONS_VERSION": "*",
                "VS_DOTNET_REFERENCES": "*",
                "VS_DOTNET_REFERENCES_COPY_LOCAL": "*",
                "VS_DOTNET_TARGET_FRAMEWORK_VERSION": "*",
                "VS_DOTNET_DOCUMENTATION_FILE": "*",
                "VS_DPI_AWARE": "*",
                "VS_GLOBAL_KEYWORD": "*",
                "VS_GLOBAL_PROJECT_TYPES": "*",
                "VS_GLOBAL_ROOTNAMESPACE": "*",
                "VS_IOT_EXTENSIONS_VERSION": "*",
                "VS_IOT_STARTUP_TASK": "*",
                "VS_JUST_MY_CODE_DEBUGGING": "*",
                "VS_KEYWORD": "*",
                "VS_MOBILE_EXTENSIONS_VERSION": "*",
                "VS_NO_SOLUTION_DEPLOY": "*",
                "VS_PACKAGE_REFERENCES": "*",
                "VS_PLATFORM_TOOLSET": "*",
                "VS_PROJECT_IMPORT": "*",
                "VS_SCC_AUXPATH": "*",
                "VS_SCC_LOCALPATH": "*",
                "VS_SCC_PROJECTNAME": "*",
                "VS_SCC_PROVIDER": "*",
                "VS_SDK_REFERENCES": "*",
                "VS_SOLUTION_DEPLOY": "*",
                "VS_USER_PROPS": "*",
                "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION": "*",
                "VS_WINRT_COMPONENT": "*",
                "VS_WINRT_EXTENSIONS": "*",
                "VS_WINRT_REFERENCES": "*",
                "WIN32_EXECUTABLE": "*",
                "WINDOWS_EXPORT_ALL_SYMBOLS": "*",
                "XCODE_EXPLICIT_FILE_TYPE": "*",
                "XCODE_GENERATE_SCHEME": "*",
                "XCODE_PRODUCT_TYPE": "*",
                "XCODE_SCHEME_ADDRESS_SANITIZER": "*",
                "XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN": "*",
                "XCODE_SCHEME_ARGUMENTS": "*",
                "XCODE_SCHEME_DEBUG_AS_ROOT": "*",
                "XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING": "*",
                "XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER": "*",
                "XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS": "*",
                "XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE": "*",
                "XCODE_SCHEME_ENVIRONMENT": "*",
                "XCODE_SCHEME_EXECUTABLE": "*",
                "XCODE_SCHEME_GUARD_MALLOC": "*",
                "XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP": "*",
                "XCODE_SCHEME_MALLOC_GUARD_EDGES": "*",
                "XCODE_SCHEME_MALLOC_SCRIBBLE": "*",
                "XCODE_SCHEME_MALLOC_STACK": "*",
                "XCODE_SCHEME_THREAD_SANITIZER": "*",
                "XCODE_SCHEME_THREAD_SANITIZER_STOP": "*",
                "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER": "*",
                "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP": "*",
                "XCODE_SCHEME_WORKING_DIRECTORY": "*",
                "XCODE_SCHEME_ZOMBIE_OBJECTS": "*",
                "XCTEST": "*",
              },
            },
          },
        },
        "SOURCE": {
          "pargs": "*",
          "kwargs": {
            "DIRECTORY": "+",
            "TARGET_DIRECTORY": "+",
            "PROPERTY": {
              "kwargs": {
                "ABSTRACT": "*",
                "AUTORCC_OPTIONS": "*",
                "AUTOUIC_OPTIONS": "*",
                "COMPILE_DEFINITIONS": "*",
                "COMPILE_FLAGS": "*",
                "COMPILE_OPTIONS": "*",
                "EXTERNAL_OBJECT": "*",
                "Fortran_FORMAT": "*",
                "Fortran_PREPROCESS": "*",
                "GENERATED": "*",
                "HEADER_FILE_ONLY": "*",
                "INCLUDE_DIRECTORIES": "*",
                "KEEP_EXTENSION": "*",
                "LABELS": "*",
                "LANGUAGE": "*",
                "LOCATION": "*",
                "MACOSX_PACKAGE_LOCATION": "*",
                "OBJECT_DEPENDS": "*",
                "OBJECT_OUTPUTS": "*",
                "SKIP_AUTOGEN": "*",
                "SKIP_AUTOMOC": "*",
                "SKIP_AUTORCC": "*",
                "SKIP_AUTOUIC": "*",
                "SKIP_PRECOMPILE_HEADERS": "*",
                "SKIP_UNITY_BUILD_INCLUSION": "*",
                "Swift_DEPENDENCIES_FILE": "*",
                "Swift_DIAGNOSTICS_FILE": "*",
                "SYMBOLIC": "*",
                "UNITY_GROUP": "*",
                "VS_COPY_TO_OUT_DIR": "*",
                "VS_CSHARP_<tagname>": "*",
                "VS_DEPLOYMENT_CONTENT": "*",
                "VS_DEPLOYMENT_LOCATION": "*",
                "VS_INCLUDE_IN_VSIX": "*",
                "VS_RESOURCE_GENERATOR": "*",
                "VS_SETTINGS": "*",
                "VS_SHADER_DISABLE_OPTIMIZATIONS": "*",
                "VS_SHADER_ENABLE_DEBUG": "*",
                "VS_SHADER_ENTRYPOINT": "*",
                "VS_SHADER_FLAGS": "*",
                "VS_SHADER_MODEL": "*",
                "VS_SHADER_OBJECT_FILE_NAME": "*",
                "VS_SHADER_OUTPUT_HEADER_FILE": "*",
                "VS_SHADER_TYPE": "*",
                "VS_SHADER_VARIABLE_NAME": "*",
                "VS_TOOL_OVERRIDE": "*",
                "VS_XAML_TYPE": "*",
                "WRAP_EXCLUDE": "*",
                "XCODE_EXPLICIT_FILE_TYPE": "*",
                "XCODE_FILE_ATTRIBUTES": "*",
                "XCODE_LAST_KNOWN_FILE_TYPE": "*",
              },
            },
          },
        },
        "INSTALL": {
          "pargs": "*",
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "CPACK_DESKTOP_SHORTCUTS": "*",
                "CPACK_NEVER_OVERWRITE": "*",
                "CPACK_PERMANENT": "*",
                "CPACK_START_MENU_SHORTCUTS": "*",
                "CPACK_STARTUP_SHORTCUTS": "*",
                "CPACK_WIX_ACL": "*",
              },
            },
          },
        },
        "TEST": {
          "pargs": "*",
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "ATTACHED_FILES_ON_FAIL": "*",
                "ATTACHED_FILES": "*",
                "COST": "*",
                "DEPENDS": "*",
                "DISABLED": "*",
                "ENVIRONMENT": "*",
                "FAIL_REGULAR_EXPRESSION": "*",
                "FIXTURES_CLEANUP": "*",
                "FIXTURES_REQUIRED": "*",
                "FIXTURES_SETUP": "*",
                "LABELS": "*",
                "MEASUREMENT": "*",
                "PASS_REGULAR_EXPRESSION": "*",
                "PROCESSOR_AFFINITY": "*",
                "PROCESSORS": "*",
                "REQUIRED_FILES": "*",
                "RESOURCE_GROUPS": "*",
                "RESOURCE_LOCK": "*",
                "RUN_SERIAL": "*",
                "SKIP_REGULAR_EXPRESSION": "*",
                "SKIP_RETURN_CODE": "*",
                "TIMEOUT": "*",
                "TIMEOUT_AFTER_MATCH": "*",
                "WILL_FAIL": "*",
                "WORKING_DIRECTORY": "*",
              },
            },
          },
        },
        "CACHE": {
          "pargs": "*",
          "kwargs": {
            "PROPERTY": {
              "kwargs": {
                "ADVANCED": "*",
                "HELPSTRING": "*",
                "MODIFIED": "*",
                "STRINGS": "*",
                "TYPE": "*",
                "VALUE": "*",
              },
            },
          },
        },
      },
    },
    "set_target_properties": {
      "pargs": "+",
      "kwargs": {
        "PROPERTIES": {
          "kwargs": {
            "ADDITIONAL_CLEAN_FILES": 1,
            "AIX_EXPORT_ALL_SYMBOLS": 1,
            "ALIAS_GLOBAL": 1,
            "ALIASED_TARGET": 1,
            "ANDROID_ANT_ADDITIONAL_OPTIONS": 1,
            "ANDROID_API": 1,
            "ANDROID_API_MIN": 1,
            "ANDROID_ARCH": 1,
            "ANDROID_ASSETS_DIRECTORIES": 1,
            "ANDROID_GUI": 1,
            "ANDROID_JAR_DEPENDENCIES": 1,
            "ANDROID_JAR_DIRECTORIES": 1,
            "ANDROID_JAVA_SOURCE_DIR": 1,
            "ANDROID_NATIVE_LIB_DEPENDENCIES": 1,
            "ANDROID_NATIVE_LIB_DIRECTORIES": 1,
            "ANDROID_PROCESS_MAX": 1,
            "ANDROID_PROGUARD": 1,
            "ANDROID_PROGUARD_CONFIG_PATH": 1,
            "ANDROID_SECURE_PROPS_PATH": 1,
            "ANDROID_SKIP_ANT_STEP": 1,
            "ANDROID_STL_TYPE": 1,
            "ARCHIVE_OUTPUT_DIRECTORY": 1,
            "ARCHIVE_OUTPUT_NAME": 1,
            "AUTOGEN_BUILD_DIR": 1,
            "AUTOGEN_ORIGIN_DEPENDS": 1,
            "AUTOGEN_PARALLEL": 1,
            "AUTOGEN_TARGET_DEPENDS": 1,
            "AUTOMOC": 1,
            "AUTOMOC_COMPILER_PREDEFINES": 1,
            "AUTOMOC_DEPEND_FILTERS": 1,
            "AUTOMOC_EXECUTABLE": 1,
            "AUTOMOC_MACRO_NAMES": 1,
            "AUTOMOC_MOC_OPTIONS": 1,
            "AUTOMOC_PATH_PREFIX": 1,
            "AUTORCC": 1,
            "AUTORCC_EXECUTABLE": 1,
            "AUTORCC_OPTIONS": 1,
            "AUTOUIC": 1,
            "AUTOUIC_EXECUTABLE": 1,
            "AUTOUIC_OPTIONS": 1,
            "AUTOUIC_SEARCH_PATHS": 1,
            "BINARY_DIR": 1,
            "BUILD_RPATH": 1,
            "BUILD_RPATH_USE_ORIGIN": 1,
            "BUILD_WITH_INSTALL_NAME_DIR": 1,
            "BUILD_WITH_INSTALL_RPATH": 1,
            "BUNDLE_EXTENSION": 1,
            "BUNDLE": 1,
            "C_EXTENSIONS": 1,
            "C_STANDARD": 1,
            "C_STANDARD_REQUIRED": 1,
            "COMMON_LANGUAGE_RUNTIME": 1,
            "COMPATIBLE_INTERFACE_BOOL": 1,
            "COMPATIBLE_INTERFACE_NUMBER_MAX": 1,
            "COMPATIBLE_INTERFACE_NUMBER_MIN": 1,
            "COMPATIBLE_INTERFACE_STRING": 1,
            "COMPILE_DEFINITIONS": 1,
            "COMPILE_FEATURES": 1,
            "COMPILE_FLAGS": 1,
            "COMPILE_OPTIONS": 1,
            "COMPILE_PDB_NAME": 1,
            "COMPILE_PDB_OUTPUT_DIRECTORY": 1,
            "CROSSCOMPILING_EMULATOR": 1,
            "CUDA_ARCHITECTURES": 1,
            "CUDA_PTX_COMPILATION": 1,
            "CUDA_SEPARABLE_COMPILATION": 1,
            "CUDA_RESOLVE_DEVICE_SYMBOLS": 1,
            "CUDA_RUNTIME_LIBRARY": 1,
            "CUDA_EXTENSIONS": 1,
            "CUDA_STANDARD": 1,
            "CUDA_STANDARD_REQUIRED": 1,
            "CXX_EXTENSIONS": 1,
            "CXX_STANDARD": 1,
            "CXX_STANDARD_REQUIRED": 1,
            "DEBUG_POSTFIX": 1,
            "DEFINE_SYMBOL": 1,
            "DEPLOYMENT_REMOTE_DIRECTORY": 1,
            "DEPLOYMENT_ADDITIONAL_FILES": 1,
            "DEPRECATION": 1,
            "DISABLE_PRECOMPILE_HEADERS": 1,
            "DOTNET_TARGET_FRAMEWORK": 1,
            "DOTNET_TARGET_FRAMEWORK_VERSION": 1,
            "EchoString": 1,
            "ENABLE_EXPORTS": 1,
            "EXCLUDE_FROM_ALL": 1,
            "EXCLUDE_FROM_DEFAULT_BUILD": 1,
            "EXPORT_NAME": 1,
            "EXPORT_PROPERTIES": 1,
            "FOLDER": 1,
            "Fortran_FORMAT": 1,
            "Fortran_MODULE_DIRECTORY": 1,
            "Fortran_PREPROCESS": 1,
            "FRAMEWORK": 1,
            "FRAMEWORK_VERSION": 1,
            "GENERATOR_FILE_NAME": 1,
            "GHS_INTEGRITY_APP": 1,
            "GHS_NO_SOURCE_GROUP_FILE": 1,
            "GNUtoMS": 1,
            "HAS_CXX": 1,
            "IMPLICIT_DEPENDS_INCLUDE_TRANSFORM": 1,
            "IMPORTED_COMMON_LANGUAGE_RUNTIME": 1,
            "IMPORTED_CONFIGURATIONS": 1,
            "IMPORTED_GLOBAL": 1,
            "IMPORTED_IMPLIB": 1,
            "IMPORTED_LIBNAME": 1,
            "IMPORTED_LINK_DEPENDENT_LIBRARIES": 1,
            "IMPORTED_LINK_INTERFACE_LANGUAGES": 1,
            "IMPORTED_LINK_INTERFACE_LIBRARIES": 1,
            "IMPORTED_LINK_INTERFACE_MULTIPLICITY": 1,
            "IMPORTED_LOCATION": 1,
            "IMPORTED_NO_SONAME": 1,
            "IMPORTED_OBJECTS": 1,
            "IMPORTED": 1,
            "IMPORTED_SONAME": 1,
            "IMPORT_PREFIX": 1,
            "IMPORT_SUFFIX": 1,
            "INCLUDE_DIRECTORIES": 1,
            "INSTALL_NAME_DIR": 1,
            "INSTALL_REMOVE_ENVIRONMENT_RPATH": 1,
            "INSTALL_RPATH": 1,
            "INSTALL_RPATH_USE_LINK_PATH": 1,
            "INTERFACE_AUTOUIC_OPTIONS": 1,
            "INTERFACE_COMPILE_DEFINITIONS": 1,
            "INTERFACE_COMPILE_FEATURES": 1,
            "INTERFACE_COMPILE_OPTIONS": 1,
            "INTERFACE_INCLUDE_DIRECTORIES": 1,
            "INTERFACE_LINK_DEPENDS": 1,
            "INTERFACE_LINK_DIRECTORIES": 1,
            "INTERFACE_LINK_LIBRARIES": 1,
            "INTERFACE_LINK_OPTIONS": 1,
            "INTERFACE_PRECOMPILE_HEADERS": 1,
            "INTERFACE_POSITION_INDEPENDENT_CODE": 1,
            "INTERFACE_SOURCES": 1,
            "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES": 1,
            "INTERPROCEDURAL_OPTIMIZATION": 1,
            "IOS_INSTALL_COMBINED": 1,
            "JOB_POOL_COMPILE": 1,
            "JOB_POOL_LINK": 1,
            "JOB_POOL_PRECOMPILE_HEADER": 1,
            "LABELS": 1,
            "LIBRARY_OUTPUT_DIRECTORY": 1,
            "LIBRARY_OUTPUT_NAME": 1,
            "LINK_DEPENDS_NO_SHARED": 1,
            "LINK_DEPENDS": 1,
            "LINKER_LANGUAGE": 1,
            "LINK_DIRECTORIES": 1,
            "LINK_FLAGS": 1,
            "LINK_INTERFACE_LIBRARIES": 1,
            "LINK_INTERFACE_MULTIPLICITY": 1,
            "LINK_LIBRARIES": 1,
            "LINK_OPTIONS": 1,
            "LINK_SEARCH_END_STATIC": 1,
            "LINK_SEARCH_START_STATIC": 1,
            "LINK_WHAT_YOU_USE": 1,
            "LOCATION": 1,
            "MACHO_COMPATIBILITY_VERSION": 1,
            "MACHO_CURRENT_VERSION": 1,
            "MACOSX_BUNDLE_INFO_PLIST": 1,
            "MACOSX_BUNDLE": 1,
            "MACOSX_FRAMEWORK_INFO_PLIST": 1,
            "MACOSX_RPATH": 1,
            "MANUALLY_ADDED_DEPENDENCIES": 1,
            "MSVC_RUNTIME_LIBRARY": 1,
            "NAME": 1,
            "NO_SONAME": 1,
            "NO_SYSTEM_FROM_IMPORTED": 1,
            "OBJC_EXTENSIONS": 1,
            "OBJC_STANDARD": 1,
            "OBJC_STANDARD_REQUIRED": 1,
            "OBJCXX_EXTENSIONS": 1,
            "OBJCXX_STANDARD": 1,
            "OBJCXX_STANDARD_REQUIRED": 1,
            "OSX_ARCHITECTURES": 1,
            "OUTPUT_NAME": 1,
            "PCH_WARN_INVALID": 1,
            "PDB_NAME": 1,
            "PDB_OUTPUT_DIRECTORY": 1,
            "POSITION_INDEPENDENT_CODE": 1,
            "PRECOMPILE_HEADERS": 1,
            "PRECOMPILE_HEADERS_REUSE_FROM": 1,
            "PREFIX": 1,
            "PRIVATE_HEADER": 1,
            "PROJECT_LABEL": 1,
            "PUBLIC_HEADER": 1,
            "RESOURCE": 1,
            "RULE_LAUNCH_COMPILE": 1,
            "RULE_LAUNCH_CUSTOM": 1,
            "RULE_LAUNCH_LINK": 1,
            "RUNTIME_OUTPUT_DIRECTORY": 1,
            "RUNTIME_OUTPUT_NAME": 1,
            "SKIP_BUILD_RPATH": 1,
            "SOURCE_DIR": 1,
            "SOURCES": 1,
            "SOVERSION": 1,
            "STATIC_LIBRARY_FLAGS": 1,
            "STATIC_LIBRARY_OPTIONS": 1,
            "SUFFIX": 1,
            "Swift_DEPENDENCIES_FILE": 1,
            "Swift_LANGUAGE_VERSION": 1,
            "Swift_MODULE_DIRECTORY": 1,
            "Swift_MODULE_NAME": 1,
            "TYPE": 1,
            "UNITY_BUILD": 1,
            "UNITY_BUILD_BATCH_SIZE": 1,
            "UNITY_BUILD_CODE_AFTER_INCLUDE": 1,
            "UNITY_BUILD_CODE_BEFORE_INCLUDE": 1,
            "UNITY_BUILD_MODE": 1,
            "VERSION": 1,
            "VISIBILITY_INLINES_HIDDEN": 1,
            "VS_CONFIGURATION_TYPE": 1,
            "VS_DEBUGGER_COMMAND": 1,
            "VS_DEBUGGER_COMMAND_ARGUMENTS": 1,
            "VS_DEBUGGER_ENVIRONMENT": 1,
            "VS_DEBUGGER_WORKING_DIRECTORY": 1,
            "VS_DESKTOP_EXTENSIONS_VERSION": 1,
            "VS_DOTNET_REFERENCES": 1,
            "VS_DOTNET_REFERENCES_COPY_LOCAL": 1,
            "VS_DOTNET_TARGET_FRAMEWORK_VERSION": 1,
            "VS_DOTNET_DOCUMENTATION_FILE": 1,
            "VS_DPI_AWARE": 1,
            "VS_GLOBAL_KEYWORD": 1,
            "VS_GLOBAL_PROJECT_TYPES": 1,
            "VS_GLOBAL_ROOTNAMESPACE": 1,
            "VS_IOT_EXTENSIONS_VERSION": 1,
            "VS_IOT_STARTUP_TASK": 1,
            "VS_JUST_MY_CODE_DEBUGGING": 1,
            "VS_KEYWORD": 1,
            "VS_MOBILE_EXTENSIONS_VERSION": 1,
            "VS_NO_SOLUTION_DEPLOY": 1,
            "VS_PACKAGE_REFERENCES": 1,
            "VS_PLATFORM_TOOLSET": 1,
            "VS_PROJECT_IMPORT": 1,
            "VS_SCC_AUXPATH": 1,
            "VS_SCC_LOCALPATH": 1,
            "VS_SCC_PROJECTNAME": 1,
            "VS_SCC_PROVIDER": 1,
            "VS_SDK_REFERENCES": 1,
            "VS_SOLUTION_DEPLOY": 1,
            "VS_USER_PROPS": 1,
            "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION": 1,
            "VS_WINRT_COMPONENT": 1,
            "VS_WINRT_EXTENSIONS": 1,
            "VS_WINRT_REFERENCES": 1,
            "WIN32_EXECUTABLE": 1,
            "WINDOWS_EXPORT_ALL_SYMBOLS": 1,
            "XCODE_EXPLICIT_FILE_TYPE": 1,
            "XCODE_GENERATE_SCHEME": 1,
            "XCODE_PRODUCT_TYPE": 1,
            "XCODE_SCHEME_ADDRESS_SANITIZER": 1,
            "XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN": 1,
            "XCODE_SCHEME_ARGUMENTS": 1,
            "XCODE_SCHEME_DEBUG_AS_ROOT": 1,
            "XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING": 1,
            "XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER": 1,
            "XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS": 1,
            "XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE": 1,
            "XCODE_SCHEME_ENVIRONMENT": 1,
            "XCODE_SCHEME_EXECUTABLE": 1,
            "XCODE_SCHEME_GUARD_MALLOC": 1,
            "XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP": 1,
            "XCODE_SCHEME_MALLOC_GUARD_EDGES": 1,
            "XCODE_SCHEME_MALLOC_SCRIBBLE": 1,
            "XCODE_SCHEME_MALLOC_STACK": 1,
            "XCODE_SCHEME_THREAD_SANITIZER": 1,
            "XCODE_SCHEME_THREAD_SANITIZER_STOP": 1,
            "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER": 1,
            "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP": 1,
            "XCODE_SCHEME_WORKING_DIRECTORY": 1,
            "XCODE_SCHEME_ZOMBIE_OBJECTS": 1,
            "XCTEST": 1,
          },
        },
      },
    },
    "target_link_options": {
      "pargs": 1,
      "flags": ["BEFORE"],
      "kwargs": {
        "INTERFACE": "*",
        "PUBLIC": "*",
        "PRIVATE": "*",
      },
    },
  }

  # Specify property tags.
  proptags = []

  # Specify variable tags.
  vartags = []

  # Override configurations per-command where available
  override_spec = { }

# -------------------------------
# Options affecting file encoding
# -------------------------------
with section("encode"):

  # If true, emit the unicode byte-order mark (BOM) at the start of the file
  emit_byteorder_mark = False

  # Specify the encoding of the input file. Defaults to utf-8
  input_encoding = u'utf-8'

  # Specify the encoding of the output file. Defaults to utf-8. Note that cmake
  # only claims to support utf-8 so be careful when using anything else
  output_encoding = u'utf-8'
