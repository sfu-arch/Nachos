/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 61 "parser/parser.y" /* yacc.c:339  */

#include <string>
#include <stdio.h>
#include <assert.h>
#include "ASTs.h"

#define YYMAXDEPTH 100000
#define YYERROR_VERBOSE

extern char* yytext;

extern "C" void yyerror(const char*);
extern "C" int yylex();


#line 82 "generated/parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
#ifndef YY_YY_GENERATED_PARSER_H_INCLUDED
# define YY_YY_GENERATED_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENT = 258,
    STRING = 259,
    NUMBER = 260,
    FLOATNUMBER = 261,
    LIT_BOOL = 262,
    VOID = 263,
    IMBED = 264,
    IMBED_TYPE = 265,
    CHIP = 266,
    THIS = 267,
    ASSIGN = 268,
    DOUBLE_COLON = 269,
    DOT = 270,
    SEMICOLON = 271,
    COLON = 272,
    GLOBAL_DECL = 273,
    MACHINE_DECL = 274,
    IN_PORT_DECL = 275,
    OUT_PORT_DECL = 276,
    PEEK = 277,
    ENQUEUE = 278,
    COPY_HEAD = 279,
    CHECK_ALLOCATE = 280,
    CHECK_STOP_SLOTS = 281,
    DEBUG_EXPR_TOKEN = 282,
    DEBUG_MSG_TOKEN = 283,
    ACTION_DECL = 284,
    TRANSITION_DECL = 285,
    TYPE_DECL = 286,
    STRUCT_DECL = 287,
    EXTERN_TYPE_DECL = 288,
    ENUM_DECL = 289,
    TYPE_FIELD = 290,
    OTHER = 291,
    IF = 292,
    ELSE = 293,
    RETURN = 294,
    EQ = 295,
    NE = 296,
    LE = 297,
    GE = 298,
    NOT = 299,
    AND = 300,
    OR = 301,
    PLUS = 302,
    DASH = 303,
    STAR = 304,
    SLASH = 305,
    RIGHTSHIFT = 306,
    LEFTSHIFT = 307
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 77 "parser/parser.y" /* yacc.c:355  */

  string* str_ptr;
  Vector<string>* string_vector_ptr; 

  // Decls
  DeclAST* decl_ptr;
  DeclListAST* decl_list_ptr;
  Vector<DeclAST*>* decl_vector_ptr; 
  
  // TypeField
  TypeFieldAST* type_field_ptr;
  Vector<TypeFieldAST*>* type_field_vector_ptr;
  
  // Type
  TypeAST* type_ptr;
  Vector<TypeAST*>* type_vector_ptr;

  // Formal Params
  FormalParamAST* formal_param_ptr;
  Vector<FormalParamAST*>* formal_param_vector_ptr;

  // Statements
  StatementAST* statement_ptr;
  StatementListAST* statement_list_ptr;
  Vector<StatementAST*>* statement_vector_ptr; 

  // Pairs
  PairAST* pair_ptr;
  PairListAST* pair_list_ptr;

  // Expressions
  VarExprAST* var_expr_ptr;
  ExprAST* expr_ptr;
  Vector<ExprAST*>* expr_vector_ptr; 

#line 211 "generated/parser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GENERATED_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 228 "generated/parser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   710

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  110
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  316

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      56,    57,     2,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    63,     2,
      42,    61,    43,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    55,     2,    62,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,     2,    59,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   165,   165,   167,   169,   170,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   193,   194,   197,   198,   202,   203,   206,   210,
     211,   214,   218,   219,   222,   223,   226,   229,   232,   233,
     237,   238,   241,   242,   245,   249,   251,   252,   255,   256,
     257,   258,   262,   263,   265,   266,   269,   270,   271,   276,
     279,   280,   283,   284,   285,   288,   289,   290,   291,   293,
     294,   295,   296,   297,   300,   301,   302,   305,   306,   307,
     308,   312,   314,   316,   318,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   341,   344,   345,   346,   347,   350,   353,
     356
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENT", "STRING", "NUMBER",
  "FLOATNUMBER", "LIT_BOOL", "VOID", "IMBED", "IMBED_TYPE", "CHIP", "THIS",
  "ASSIGN", "DOUBLE_COLON", "DOT", "SEMICOLON", "COLON", "GLOBAL_DECL",
  "MACHINE_DECL", "IN_PORT_DECL", "OUT_PORT_DECL", "PEEK", "ENQUEUE",
  "COPY_HEAD", "CHECK_ALLOCATE", "CHECK_STOP_SLOTS", "DEBUG_EXPR_TOKEN",
  "DEBUG_MSG_TOKEN", "ACTION_DECL", "TRANSITION_DECL", "TYPE_DECL",
  "STRUCT_DECL", "EXTERN_TYPE_DECL", "ENUM_DECL", "TYPE_FIELD", "OTHER",
  "IF", "ELSE", "RETURN", "EQ", "NE", "'<'", "'>'", "LE", "GE", "NOT",
  "AND", "OR", "PLUS", "DASH", "STAR", "SLASH", "RIGHTSHIFT", "LEFTSHIFT",
  "'['", "'('", "')'", "'{'", "'}'", "','", "'='", "']'", "':'", "$accept",
  "file", "decl_list", "decls", "decl", "type_members", "type_member",
  "type_methods", "type_method", "type_enums", "type_enum", "type_list",
  "types", "type", "void", "type_or_void", "formal_param_list",
  "formal_params", "formal_param", "ident", "ident_list", "idents",
  "pair_list", "pairs", "pair", "statement_list", "statements",
  "expr_list", "statement", "if_statement", "expr", "literal",
  "enumeration", "var", "field", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    60,    62,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,    91,    40,    41,   123,   125,
      44,    61,    93,    58
};
# endif

#define YYPACT_NINF -270

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-270)))

#define YYTABLE_NINF -37

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-37)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     250,  -270,  -270,   -46,   -43,   -21,   -18,    -8,    11,    13,
      16,    20,     7,  -270,  -270,   250,    91,    91,  -270,    91,
      91,    91,    91,    91,     1,    91,    91,    91,  -270,  -270,
       5,    40,    41,    41,    56,    59,    41,    91,  -270,    62,
      41,    41,    41,    91,    26,    82,    91,    63,    66,    91,
      91,    68,    15,    67,     1,    73,    74,    75,    91,    76,
    -270,    77,  -270,    78,  -270,    80,  -270,    79,    84,    85,
      87,    89,    92,    91,    91,  -270,  -270,    93,    96,   -13,
      98,  -270,    41,    91,   100,    26,    41,    91,   250,    91,
      91,   331,  -270,  -270,  -270,    26,   101,    91,  -270,    60,
      91,   -11,  -270,  -270,  -270,  -270,    -7,   106,    91,    91,
     107,  -270,    41,    41,  -270,  -270,  -270,  -270,   112,   119,
     113,   117,   120,   121,   122,   127,   200,   200,   143,    14,
     128,   331,  -270,   469,  -270,  -270,  -270,    45,     1,   129,
     132,    60,  -270,  -270,    91,   134,    91,    41,  -270,  -270,
    -270,  -270,  -270,  -270,    -5,  -270,   135,   138,   200,    91,
      91,    91,    91,    91,    91,   200,   532,   489,    91,   200,
      91,  -270,  -270,   200,    91,  -270,   200,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   140,  -270,  -270,  -270,  -270,   144,  -270,  -270,   180,
     200,   186,    92,   192,   348,   154,   155,   157,   158,   162,
     160,   510,  -270,  -270,   166,   169,   448,  -270,   553,   167,
    -270,   635,   635,   329,   329,   329,   329,   655,   614,    27,
      27,    38,    38,   594,   594,   152,     1,    91,  -270,   573,
    -270,  -270,  -270,   212,   200,    91,    91,    91,   213,   131,
      92,   200,  -270,   200,  -270,   200,  -270,  -270,   171,  -270,
     170,  -270,    91,   391,   174,    41,    41,  -270,   172,   195,
     177,  -270,   178,    41,    91,   184,   225,    92,   185,   187,
     237,    -1,  -270,  -270,   232,  -270,   200,    91,  -270,    92,
     233,   193,  -270,  -270,  -270,   407,   240,  -270,  -270,   241,
     246,    91,  -270,    91,   207,  -270,   251,   200,    91,   208,
     211,  -270,  -270,   200,   215,  -270
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       5,    45,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     3,     5,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,     4,
      53,     0,    53,    53,     0,     0,    53,    51,    47,     0,
      53,    53,    53,    41,     0,     0,    41,     0,     0,     0,
       0,     0,    51,     0,     0,     0,     0,     0,     0,     0,
      40,    43,    58,     0,    52,    55,    17,     0,     0,     0,
       0,     0,     0,    51,    51,    50,    46,    53,     0,     0,
       0,    44,    53,     0,     0,     0,    53,    23,     5,     0,
       0,    61,     7,    48,    49,     0,     0,    23,    12,    27,
      30,     0,    42,    56,    57,    54,     0,     0,    23,     0,
       0,   109,    53,    53,   104,   105,   106,   107,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   109,
       0,    61,    72,     0,    78,    79,    77,    53,     0,     0,
       0,    27,    38,    39,     0,     0,    30,    53,    18,    20,
      19,    21,    14,    22,    53,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    64,
       0,    59,    60,     0,     0,    65,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      64,     0,    11,    15,    13,    26,     0,    16,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    73,   103,     0,     0,    63,   108,     0,   110,
      85,    97,    98,    93,    94,    95,    96,    99,   100,    91,
      92,    89,    90,   101,   102,     0,     0,    33,    31,     0,
      24,     8,     9,     0,     0,     0,     0,     0,     0,     0,
       0,    64,    80,    64,    66,    64,    88,    10,     0,    32,
      35,    25,     0,     0,     0,    53,    53,    70,     0,    75,
       0,    62,     0,    53,     0,     0,     0,     0,     0,     0,
       0,     0,    87,    86,     0,    34,     0,     0,    68,     0,
       0,     0,    74,    76,    28,     0,     0,    67,    69,     0,
       0,     0,    71,     0,   110,    82,     0,    64,     0,     0,
     110,    84,    81,    64,     0,    83
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -270,  -270,   189,   258,  -270,   -37,  -270,   137,  -270,   142,
    -270,  -270,    18,   102,   -85,  -270,   229,   198,  -270,     0,
     -52,   -40,   139,   204,  -270,   -95,   159,  -189,  -270,     4,
     133,  -270,  -270,   -49,  -269
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    12,    13,    14,    15,   107,   108,   140,   141,   145,
     146,   258,   259,   128,    17,   144,    59,    60,    61,   129,
      39,    53,    45,    64,    65,    92,   130,   215,   131,   132,
     216,   134,   135,   136,   220
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      18,   235,    77,    98,     1,   148,   149,    28,   200,   150,
      19,   151,    75,    20,   143,    18,    30,    31,     1,    18,
      33,    34,    35,    36,    38,    18,    18,    18,   -36,     1,
      62,    73,   305,    93,    94,    21,   125,    52,    22,   311,
     112,   113,   174,    18,    63,    99,    18,    91,    23,    18,
      18,    91,    52,   174,    38,    44,   143,    91,    81,    37,
     139,    43,   270,     1,   271,    44,   272,    24,     2,    25,
     169,   153,    26,    52,    52,    74,    27,   170,   186,   187,
     188,   189,   190,    18,   104,    63,   192,    18,    18,   111,
     111,   188,   189,   190,     1,   137,    46,    18,    66,    18,
     147,    44,    16,     1,   103,    44,    84,   241,    18,   154,
     205,   206,   207,   208,   209,   210,    49,    16,   309,    50,
      68,    32,    54,    69,   314,    72,    76,    40,    41,    42,
      78,    79,    80,    82,   159,   268,    86,    83,    38,    84,
      85,    18,    87,    88,   196,    58,   147,    89,    58,    90,
      91,    70,    71,    95,    97,   269,   100,   168,   138,   111,
     111,   111,   111,   111,   111,   152,   155,   158,   214,   160,
     217,    47,    48,   161,   219,    51,   162,   163,   164,    55,
      56,    57,   288,   165,   257,    58,   292,   171,   193,   109,
      16,   194,   202,   197,   297,   203,   238,   236,   266,   109,
     237,   142,   240,     1,   114,   115,   116,   117,   242,   244,
     109,   118,   119,   275,   256,   245,    96,   246,   247,   248,
     249,   101,   251,   255,   133,   106,   252,   262,   273,   267,
     274,   277,   280,   281,   282,   283,    38,    18,   296,   286,
     287,   291,   289,   142,   290,    18,    18,   111,   294,   298,
     299,   156,   157,     1,   306,   301,   127,   302,     2,   166,
     167,   303,   111,   307,   133,   312,   308,   313,     3,     4,
       5,     6,   315,    29,    18,    67,   191,   110,   195,     7,
       8,   102,     9,    10,    11,   293,   199,   111,   198,   105,
     172,   204,   285,   201,     0,     0,     0,     0,   211,     0,
       0,   304,     0,   111,     0,     0,   218,     0,   310,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   239,     1,   114,   115,   116,   117,   260,
       0,     0,   118,   119,   174,     0,     0,   264,   265,     0,
       0,     0,     0,   120,   121,   122,   123,   124,     0,     0,
       0,     0,     0,   174,     0,     0,     0,     0,   125,     0,
     126,   -37,   -37,   -37,   -37,     0,   260,   263,   184,   185,
     186,   187,   188,   189,   190,     0,     0,   127,   176,   177,
     178,   179,   180,   181,     0,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   278,   279,   174,     0,     0,     0,
     243,     0,   284,     0,     0,     0,     0,     0,     0,   295,
       0,     0,   174,     0,     0,     0,     0,     0,     0,     0,
       0,   176,   177,   178,   179,   180,   181,     0,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   176,   177,   178,
     179,   180,   181,   276,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   174,     0,     0,     0,     0,     0,   300,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   173,     0,   174,   175,     0,     0,   176,   177,
     178,   179,   180,   181,     0,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   174,     0,     0,     0,   253,   176,
     177,   178,   179,   180,   181,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   174,     0,     0,     0,   176,
     177,   178,   179,   180,   181,     0,   182,   183,   184,   185,
     186,   187,   188,   189,   190,     0,   213,   174,   212,     0,
     176,   177,   178,   179,   180,   181,     0,   182,   183,   184,
     185,   186,   187,   188,   189,   190,     0,   250,   174,   254,
       0,     0,   176,   177,   178,   179,   180,   181,     0,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   174,   261,
       0,     0,     0,   176,   177,   178,   179,   180,   181,     0,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   174,
       0,     0,     0,   176,   177,   178,   179,   180,   181,     0,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   174,
       0,     0,     0,     0,   176,   177,   178,   179,   180,   181,
       0,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     174,     0,     0,     0,   176,   177,   178,   179,   180,   181,
       0,   182,     0,   184,   185,   186,   187,   188,   189,   190,
     174,     0,     0,     0,     0,   -37,   -37,   178,   179,   180,
     181,     0,     0,     0,   184,   185,   186,   187,   188,   189,
     190,     0,     0,     0,     0,   176,   177,   178,   179,   180,
     181,     0,     0,     0,   184,   185,   186,   187,   188,   189,
     190
};

static const yytype_int16 yycheck[] =
{
       0,   190,    54,    16,     3,    16,   101,     0,    13,    16,
      56,   106,    52,    56,    99,    15,    16,    17,     3,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    14,     3,
       4,    16,   301,    73,    74,    56,    37,    37,    56,   308,
      89,    90,    15,    43,    44,    58,    46,    58,    56,    49,
      50,    58,    52,    15,    54,    60,   141,    58,    58,    58,
      97,    56,   251,     3,   253,    60,   255,    56,     8,    56,
      56,   108,    56,    73,    74,    60,    56,    63,    51,    52,
      53,    54,    55,    83,    84,    85,   138,    87,    88,    89,
      90,    53,    54,    55,     3,    95,    56,    97,    16,    99,
     100,    60,     0,     3,     4,    60,    61,   202,   108,   109,
     159,   160,   161,   162,   163,   164,    60,    15,   307,    60,
      57,    19,    60,    57,   313,    57,    59,    25,    26,    27,
      57,    57,    57,    57,    15,     4,    57,    60,   138,    61,
      60,   141,    58,    58,   144,    43,   146,    60,    46,    60,
      58,    49,    50,    60,    58,   250,    58,    14,    57,   159,
     160,   161,   162,   163,   164,    59,    59,    55,   168,    56,
     170,    32,    33,    56,   174,    36,    56,    56,    56,    40,
      41,    42,   277,    56,   236,    83,   281,    59,    59,    87,
      88,    59,    57,    59,   289,    57,    16,    57,   247,    97,
      56,    99,    16,     3,     4,     5,     6,     7,    16,    55,
     108,    11,    12,   262,    62,    60,    77,    60,    60,    57,
      60,    82,    56,    56,    91,    86,    57,    15,    57,    16,
      60,    57,    60,    38,    57,    57,   236,   237,   287,    55,
      15,     4,    57,   141,    57,   245,   246,   247,    16,    16,
      57,   112,   113,     3,   303,    15,    56,    16,     8,   126,
     127,    15,   262,    56,   131,    57,    15,    56,    18,    19,
      20,    21,    57,    15,   274,    46,   137,    88,   141,    29,
      30,    83,    32,    33,    34,   281,   147,   287,   146,    85,
     131,   158,   274,   154,    -1,    -1,    -1,    -1,   165,    -1,
      -1,   301,    -1,   303,    -1,    -1,   173,    -1,   308,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,     3,     4,     5,     6,     7,   237,
      -1,    -1,    11,    12,    15,    -1,    -1,   245,   246,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,    37,    -1,
      39,    42,    43,    44,    45,    -1,   274,   244,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    56,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   265,   266,    15,    -1,    -1,    -1,
      62,    -1,   273,    -1,    -1,    -1,    -1,    -1,    -1,   286,
      -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    -1,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    40,    41,    42,
      43,    44,    45,    62,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    15,    -1,    -1,    -1,    -1,    -1,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    13,    -1,    15,    16,    -1,    -1,    40,    41,
      42,    43,    44,    45,    -1,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    15,    -1,    -1,    -1,    60,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    15,    -1,    -1,    -1,    40,
      41,    42,    43,    44,    45,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    57,    15,    16,    -1,
      40,    41,    42,    43,    44,    45,    -1,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,    15,    16,
      -1,    -1,    40,    41,    42,    43,    44,    45,    -1,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    15,    16,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    15,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    15,
      -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,    45,
      -1,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      15,    -1,    -1,    -1,    40,    41,    42,    43,    44,    45,
      -1,    47,    -1,    49,    50,    51,    52,    53,    54,    55,
      15,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,
      45,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     8,    18,    19,    20,    21,    29,    30,    32,
      33,    34,    65,    66,    67,    68,    77,    78,    83,    56,
      56,    56,    56,    56,    56,    56,    56,    56,     0,    67,
      83,    83,    77,    83,    83,    83,    83,    58,    83,    84,
      77,    77,    77,    56,    60,    86,    56,    86,    86,    60,
      60,    86,    83,    85,    60,    86,    86,    86,    77,    80,
      81,    82,     4,    83,    87,    88,    16,    80,    57,    57,
      77,    77,    57,    16,    60,    85,    59,    84,    57,    57,
      57,    83,    57,    60,    61,    60,    57,    58,    58,    60,
      60,    58,    89,    85,    85,    60,    86,    58,    16,    58,
      58,    86,    81,     4,    83,    87,    86,    69,    70,    77,
      66,    83,    97,    97,     4,     5,     6,     7,    11,    12,
      22,    23,    24,    25,    26,    37,    39,    56,    77,    83,
      90,    92,    93,    94,    95,    96,    97,    83,    57,    69,
      71,    72,    77,    78,    79,    73,    74,    83,    16,    89,
      16,    89,    59,    69,    83,    59,    86,    86,    55,    15,
      56,    56,    56,    56,    56,    56,    94,    94,    14,    56,
      63,    59,    90,    13,    15,    16,    40,    41,    42,    43,
      44,    45,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    86,    84,    59,    59,    71,    83,    59,    73,    86,
      13,    86,    57,    57,    94,    97,    97,    97,    97,    97,
      97,    94,    16,    57,    83,    91,    94,    83,    94,    83,
      98,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    91,    57,    56,    16,    94,
      16,    89,    16,    62,    55,    60,    60,    60,    57,    60,
      57,    56,    57,    60,    16,    56,    62,    84,    75,    76,
      77,    16,    15,    94,    77,    77,    97,    16,     4,    89,
      91,    91,    91,    57,    60,    97,    62,    57,    86,    86,
      60,    38,    57,    57,    86,    76,    55,    15,    89,    57,
      57,     4,    89,    93,    16,    94,    97,    89,    16,    57,
      62,    15,    16,    15,    83,    98,    97,    56,    15,    91,
      83,    98,    57,    56,    91,    57
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    64,    65,    66,    67,    67,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    69,    69,    70,    70,    71,    71,    72,    73,
      73,    74,    75,    75,    76,    76,    77,    78,    79,    79,
      80,    80,    81,    81,    82,    83,    84,    84,    85,    85,
      85,    85,    86,    86,    87,    87,    88,    88,    88,    89,
      90,    90,    91,    91,    91,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    93,    93,    93,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    95,    95,    95,    95,    96,    97,
      98
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     0,     8,     6,    10,    10,
      10,     8,     6,     8,     8,     8,     8,     4,     7,     7,
       7,     7,     2,     0,     4,     5,     2,     0,     7,     2,
       0,     3,     1,     0,     3,     1,     1,     1,     1,     1,
       1,     0,     3,     1,     2,     1,     3,     1,     3,     3,
       2,     0,     2,     0,     3,     1,     3,     3,     1,     3,
       2,     0,     3,     1,     0,     2,     4,     8,     7,     8,
       5,     9,     1,     3,     7,     5,     7,     1,     1,     1,
       4,    13,    10,    16,    13,     3,     6,     6,     4,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     1,     1,     3,     1,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 165 "parser/parser.y" /* yacc.c:1646  */
    { g_decl_list_ptr = (yyvsp[0].decl_list_ptr); }
#line 1593 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 167 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_list_ptr) = new DeclListAST((yyvsp[0].decl_vector_ptr)); }
#line 1599 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 169 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].decl_vector_ptr)->insertAtTop((yyvsp[-1].decl_ptr)); (yyval.decl_vector_ptr) = (yyvsp[0].decl_vector_ptr); }
#line 1605 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 170 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_vector_ptr) = new Vector<DeclAST*>; }
#line 1611 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 173 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new MachineAST((yyvsp[-5].str_ptr), (yyvsp[-4].pair_list_ptr), (yyvsp[-1].decl_list_ptr)); }
#line 1617 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 174 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new ActionDeclAST((yyvsp[-3].str_ptr), (yyvsp[-2].pair_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 1623 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 175 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new InPortDeclAST((yyvsp[-7].str_ptr), (yyvsp[-5].type_ptr), (yyvsp[-3].var_expr_ptr), (yyvsp[-2].pair_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 1629 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 176 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new OutPortDeclAST((yyvsp[-7].str_ptr), (yyvsp[-5].type_ptr), (yyvsp[-3].var_expr_ptr), (yyvsp[-2].pair_list_ptr)); }
#line 1635 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 177 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new TransitionDeclAST((yyvsp[-7].string_vector_ptr), (yyvsp[-5].string_vector_ptr), (yyvsp[-3].str_ptr), (yyvsp[-2].pair_list_ptr), (yyvsp[0].string_vector_ptr)); }
#line 1641 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 178 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new TransitionDeclAST((yyvsp[-5].string_vector_ptr), (yyvsp[-3].string_vector_ptr), NULL, (yyvsp[-2].pair_list_ptr), (yyvsp[0].string_vector_ptr)); }
#line 1647 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 179 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].pair_list_ptr)->addPair(new PairAST("external", "yes")); (yyval.decl_ptr) = new TypeDeclAST((yyvsp[-3].type_ptr), (yyvsp[-2].pair_list_ptr), NULL); }
#line 1653 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 180 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[-4].pair_list_ptr)->addPair(new PairAST("external", "yes")); (yyval.decl_ptr) = new TypeDeclAST((yyvsp[-5].type_ptr), (yyvsp[-4].pair_list_ptr), (yyvsp[-1].type_field_vector_ptr)); }
#line 1659 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 181 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[-4].pair_list_ptr)->addPair(new PairAST("global", "yes"));(yyval.decl_ptr) = new TypeDeclAST((yyvsp[-5].type_ptr), (yyvsp[-4].pair_list_ptr), (yyvsp[-1].type_field_vector_ptr)); }
#line 1665 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 182 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new TypeDeclAST((yyvsp[-5].type_ptr), (yyvsp[-4].pair_list_ptr), (yyvsp[-1].type_field_vector_ptr)); }
#line 1671 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 183 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[-4].pair_list_ptr)->addPair(new PairAST("enumeration", "yes")); (yyval.decl_ptr) = new EnumDeclAST((yyvsp[-5].type_ptr), (yyvsp[-4].pair_list_ptr), (yyvsp[-1].type_field_vector_ptr)); }
#line 1677 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 184 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new ObjDeclAST((yyvsp[-3].type_ptr), (yyvsp[-2].str_ptr), (yyvsp[-1].pair_list_ptr)); }
#line 1683 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 185 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new FuncDeclAST((yyvsp[-6].type_ptr), (yyvsp[-5].str_ptr), (yyvsp[-3].formal_param_vector_ptr), (yyvsp[-1].pair_list_ptr), NULL); }
#line 1689 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 186 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new FuncDeclAST((yyvsp[-6].type_ptr), (yyvsp[-5].str_ptr), (yyvsp[-3].formal_param_vector_ptr), (yyvsp[-1].pair_list_ptr), NULL); }
#line 1695 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 187 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new FuncDeclAST((yyvsp[-6].type_ptr), (yyvsp[-5].str_ptr), (yyvsp[-3].formal_param_vector_ptr), (yyvsp[-1].pair_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 1701 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 188 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.decl_ptr) = new FuncDeclAST((yyvsp[-6].type_ptr), (yyvsp[-5].str_ptr), (yyvsp[-3].formal_param_vector_ptr), (yyvsp[-1].pair_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 1707 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 193 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].type_field_vector_ptr)->insertAtTop((yyvsp[-1].type_field_ptr)); (yyval.type_field_vector_ptr) = (yyvsp[0].type_field_vector_ptr); }
#line 1713 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 194 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_vector_ptr) = new Vector<TypeFieldAST*>; }
#line 1719 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 197 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_ptr) = new TypeFieldMemberAST((yyvsp[-3].type_ptr), (yyvsp[-2].str_ptr), (yyvsp[-1].pair_list_ptr), NULL); }
#line 1725 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 198 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_ptr) = new TypeFieldMemberAST((yyvsp[-4].type_ptr), (yyvsp[-3].str_ptr), new PairListAST(), (yyvsp[-1].expr_ptr)); }
#line 1731 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 202 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].type_field_vector_ptr)->insertAtTop((yyvsp[-1].type_field_ptr)); (yyval.type_field_vector_ptr) = (yyvsp[0].type_field_vector_ptr); }
#line 1737 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 203 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_vector_ptr) = new Vector<TypeFieldAST*>; }
#line 1743 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 206 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_ptr) = new TypeFieldMethodAST((yyvsp[-6].type_ptr), (yyvsp[-5].str_ptr), (yyvsp[-3].type_vector_ptr), (yyvsp[-1].pair_list_ptr)); }
#line 1749 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 210 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].type_field_vector_ptr)->insertAtTop((yyvsp[-1].type_field_ptr)); (yyval.type_field_vector_ptr) = (yyvsp[0].type_field_vector_ptr); }
#line 1755 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 211 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_vector_ptr) = new Vector<TypeFieldAST*>; }
#line 1761 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 214 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_field_ptr) = new TypeFieldEnumAST((yyvsp[-2].str_ptr), (yyvsp[-1].pair_list_ptr)); }
#line 1767 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 218 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_vector_ptr) = (yyvsp[0].type_vector_ptr); }
#line 1773 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 219 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_vector_ptr) = new Vector<TypeAST*>; }
#line 1779 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 222 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].type_vector_ptr)->insertAtTop((yyvsp[-2].type_ptr)); (yyval.type_vector_ptr) = (yyvsp[0].type_vector_ptr); }
#line 1785 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 223 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_vector_ptr) = new Vector<TypeAST*>; (yyval.type_vector_ptr)->insertAtTop((yyvsp[0].type_ptr)); }
#line 1791 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 226 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_ptr) = new TypeAST((yyvsp[0].str_ptr)); }
#line 1797 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 229 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_ptr) = new TypeAST((yyvsp[0].str_ptr)); }
#line 1803 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 232 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_ptr) = (yyvsp[0].type_ptr); }
#line 1809 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 233 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.type_ptr) = (yyvsp[0].type_ptr); }
#line 1815 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 237 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.formal_param_vector_ptr) = (yyvsp[0].formal_param_vector_ptr); }
#line 1821 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 238 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.formal_param_vector_ptr) = new Vector<FormalParamAST*>; }
#line 1827 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 241 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].formal_param_vector_ptr)->insertAtTop((yyvsp[-2].formal_param_ptr)); (yyval.formal_param_vector_ptr) = (yyvsp[0].formal_param_vector_ptr); }
#line 1833 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 242 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.formal_param_vector_ptr) = new Vector<FormalParamAST*>; (yyval.formal_param_vector_ptr)->insertAtTop((yyvsp[0].formal_param_ptr)); }
#line 1839 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 245 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.formal_param_ptr) = new FormalParamAST((yyvsp[-1].type_ptr), (yyvsp[0].str_ptr)); }
#line 1845 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 249 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.str_ptr) = (yyvsp[0].str_ptr); }
#line 1851 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 251 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.string_vector_ptr) = (yyvsp[-1].string_vector_ptr); }
#line 1857 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 252 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.string_vector_ptr) = new Vector<string>; (yyval.string_vector_ptr)->insertAtTop(*((yyvsp[0].str_ptr))); delete (yyvsp[0].str_ptr); }
#line 1863 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 255 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].string_vector_ptr)->insertAtTop(*((yyvsp[-2].str_ptr))); (yyval.string_vector_ptr) = (yyvsp[0].string_vector_ptr); delete (yyvsp[-2].str_ptr); }
#line 1869 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 256 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].string_vector_ptr)->insertAtTop(*((yyvsp[-2].str_ptr))); (yyval.string_vector_ptr) = (yyvsp[0].string_vector_ptr); delete (yyvsp[-2].str_ptr); }
#line 1875 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 257 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].string_vector_ptr)->insertAtTop(*((yyvsp[-1].str_ptr))); (yyval.string_vector_ptr) = (yyvsp[0].string_vector_ptr); delete (yyvsp[-1].str_ptr); }
#line 1881 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 258 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.string_vector_ptr) = new Vector<string>; }
#line 1887 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 262 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_list_ptr) = (yyvsp[0].pair_list_ptr); }
#line 1893 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 263 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_list_ptr) = new PairListAST(); }
#line 1899 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 265 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].pair_list_ptr)->addPair((yyvsp[-2].pair_ptr)); (yyval.pair_list_ptr) = (yyvsp[0].pair_list_ptr); }
#line 1905 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 266 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_list_ptr) = new PairListAST(); (yyval.pair_list_ptr)->addPair((yyvsp[0].pair_ptr)); }
#line 1911 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 269 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_ptr) = new PairAST((yyvsp[-2].str_ptr), (yyvsp[0].str_ptr)); }
#line 1917 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 270 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_ptr) = new PairAST((yyvsp[-2].str_ptr), (yyvsp[0].str_ptr)); }
#line 1923 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 271 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.pair_ptr) = new PairAST(new string("short"), (yyvsp[0].str_ptr)); }
#line 1929 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 276 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_list_ptr) = new StatementListAST((yyvsp[-1].statement_vector_ptr)); }
#line 1935 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 279 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].statement_vector_ptr)->insertAtTop((yyvsp[-1].statement_ptr)); (yyval.statement_vector_ptr) = (yyvsp[0].statement_vector_ptr); }
#line 1941 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 280 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_vector_ptr) = new Vector<StatementAST*>; }
#line 1947 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 283 "parser/parser.y" /* yacc.c:1646  */
    { (yyvsp[0].expr_vector_ptr)->insertAtTop((yyvsp[-2].expr_ptr)); (yyval.expr_vector_ptr) = (yyvsp[0].expr_vector_ptr); }
#line 1953 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 284 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_vector_ptr) = new Vector<ExprAST*>; (yyval.expr_vector_ptr)->insertAtTop((yyvsp[0].expr_ptr)); }
#line 1959 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 285 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_vector_ptr) = new Vector<ExprAST*>; }
#line 1965 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 288 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new ExprStatementAST((yyvsp[-1].expr_ptr)); }
#line 1971 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 289 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new AssignStatementAST((yyvsp[-3].expr_ptr), (yyvsp[-1].expr_ptr)); }
#line 1977 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 290 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new EnqueueStatementAST((yyvsp[-5].var_expr_ptr), (yyvsp[-3].type_ptr), (yyvsp[-2].pair_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 1983 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 291 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new PeekStatementAST((yyvsp[-4].var_expr_ptr), (yyvsp[-2].type_ptr), (yyvsp[0].statement_list_ptr), "peek"); }
#line 1989 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 293 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new CopyHeadStatementAST((yyvsp[-5].var_expr_ptr), (yyvsp[-3].var_expr_ptr), (yyvsp[-2].pair_list_ptr)); }
#line 1995 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 294 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new CheckAllocateStatementAST((yyvsp[-2].var_expr_ptr)); }
#line 2001 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 295 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new CheckStopSlotsStatementAST((yyvsp[-6].var_expr_ptr), (yyvsp[-4].str_ptr), (yyvsp[-2].str_ptr)); }
#line 2007 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 296 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = (yyvsp[0].statement_ptr); }
#line 2013 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 297 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new ReturnStatementAST((yyvsp[-1].expr_ptr)); }
#line 2019 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 300 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new IfStatementAST((yyvsp[-4].expr_ptr), (yyvsp[-2].statement_list_ptr), (yyvsp[0].statement_list_ptr)); }
#line 2025 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 301 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new IfStatementAST((yyvsp[-2].expr_ptr), (yyvsp[0].statement_list_ptr), NULL); }
#line 2031 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 302 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.statement_ptr) = new IfStatementAST((yyvsp[-4].expr_ptr), (yyvsp[-2].statement_list_ptr), new StatementListAST((yyvsp[0].statement_ptr))); }
#line 2037 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 305 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = (yyvsp[0].var_expr_ptr); }
#line 2043 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 306 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2049 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 307 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = (yyvsp[0].expr_ptr); }
#line 2055 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 308 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new FuncCallExprAST((yyvsp[-3].str_ptr), (yyvsp[-1].expr_vector_ptr)); }
#line 2061 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 312 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new ChipComponentAccessAST((yyvsp[-10].var_expr_ptr), (yyvsp[-8].expr_ptr), (yyvsp[-5].var_expr_ptr), (yyvsp[-3].str_ptr), (yyvsp[-1].expr_vector_ptr) ); }
#line 2067 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 314 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new ChipComponentAccessAST((yyvsp[-7].var_expr_ptr), (yyvsp[-5].expr_ptr), (yyvsp[-2].var_expr_ptr), (yyvsp[0].str_ptr) ); }
#line 2073 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 316 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new ChipComponentAccessAST((yyvsp[-13].expr_ptr), (yyvsp[-10].var_expr_ptr), (yyvsp[-8].expr_ptr), (yyvsp[-5].var_expr_ptr), (yyvsp[-3].str_ptr), (yyvsp[-1].expr_vector_ptr) ); }
#line 2079 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 318 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new ChipComponentAccessAST((yyvsp[-10].expr_ptr), (yyvsp[-7].var_expr_ptr), (yyvsp[-5].expr_ptr), (yyvsp[-2].var_expr_ptr), (yyvsp[0].str_ptr) ); }
#line 2085 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 321 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new MemberExprAST((yyvsp[-2].expr_ptr), (yyvsp[0].str_ptr)); }
#line 2091 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 322 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new MethodCallExprAST((yyvsp[-5].expr_ptr), (yyvsp[-3].str_ptr), (yyvsp[-1].expr_vector_ptr)); }
#line 2097 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 323 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new MethodCallExprAST((yyvsp[-5].type_ptr), (yyvsp[-3].str_ptr), (yyvsp[-1].expr_vector_ptr)); }
#line 2103 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 324 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new MethodCallExprAST((yyvsp[-3].expr_ptr), new string("lookup"), (yyvsp[-1].expr_vector_ptr)); }
#line 2109 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 325 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2115 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 326 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2121 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 327 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2127 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 328 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2133 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 329 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2139 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 330 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2145 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 331 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2151 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 332 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2157 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 333 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2163 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 334 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2169 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 335 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2175 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 336 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2181 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 337 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2187 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 338 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new InfixOperatorExprAST((yyvsp[-2].expr_ptr), (yyvsp[-1].str_ptr), (yyvsp[0].expr_ptr)); }
#line 2193 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 341 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = (yyvsp[-1].expr_ptr); }
#line 2199 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 344 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new LiteralExprAST((yyvsp[0].str_ptr), "string"); }
#line 2205 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 345 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new LiteralExprAST((yyvsp[0].str_ptr), "int"); }
#line 2211 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 346 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new LiteralExprAST((yyvsp[0].str_ptr), "int"); }
#line 2217 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 347 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new LiteralExprAST((yyvsp[0].str_ptr), "bool");  }
#line 2223 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 350 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.expr_ptr) = new EnumExprAST(new TypeAST((yyvsp[-2].str_ptr)), (yyvsp[0].str_ptr)); }
#line 2229 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 353 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.var_expr_ptr) = new VarExprAST((yyvsp[0].str_ptr)); }
#line 2235 "generated/parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 356 "parser/parser.y" /* yacc.c:1646  */
    { (yyval.str_ptr) = (yyvsp[0].str_ptr); }
#line 2241 "generated/parser.c" /* yacc.c:1646  */
    break;


#line 2245 "generated/parser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 359 "parser/parser.y" /* yacc.c:1906  */


extern FILE *yyin;

DeclListAST* parse(string filename)
{
  FILE *file;
  file = fopen(filename.c_str(), "r");
  if (!file) {
    cerr << "Error: Could not open file: " << filename << endl;
    exit(1);
  }
  g_line_number = 1;
  g_file_name = filename;
  yyin = file;
  g_decl_list_ptr = NULL;
  yyparse();
  return g_decl_list_ptr;
}

extern "C" void yyerror(const char* s)
{
  fprintf(stderr, "%s:%d: %s at %s\n", g_file_name.c_str(), g_line_number, s, yytext);
  exit(1);
}

