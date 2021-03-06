
#include <exception>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define NODE_TRANSPARENT
#include "../node.h"

#include <iconv.h>
#include <errno.h>

TEST_CASE( "tests run", "[meta]" ) {
  REQUIRE( 1 == 1 );
}

void node_error( const char * psError ) {
  throw std::exception();
}

int node_memory( size_t /*cb*/ ) {
  throw std::bad_alloc();
  return NODE_MEMORY_FAIL;
}

class node_exception : public std::exception {
public:
  char * reason;
  node_exception(const char * s) :reason(strdup(s)) {}
  ~node_exception() throw() { free(reason); } 

  const char * what() const throw() {
    return reason;
  }
};

void node_assert( const char * psExpr, const char * psFile, unsigned int nLine ) {
  char acBuffer[1024] = {0};

  sprintf( acBuffer, "Node assertion failed: %s at %s:%d", psExpr, psFile, nLine );

  throw(node_exception(acBuffer));
}


// get a filename
// g_psFilename = _strdup( "test_nodeXXXXXX" );
//

char * g_psFileName;

struct setup {
  setup () {
    node_set_error_funcs(node_error, node_memory, (node_assert_func_t) node_assert );
    g_psFileName = strdup("/tmp/test_nodeXXXXXX");
    mktemp(g_psFileName);
  }
  ~setup () {
    remove(g_psFileName);
    free(g_psFileName);
  }
};

setup __setup;

TEST_CASE("report version", "[meta][version]" ) {
  REQUIRE( node_version() != NULL );
  fprintf(stderr, "%s\n", node_version());
}

TEST_CASE("check size", "[meta][size]" ) {
  size_t nSize = sizeof(struct __node);

#if defined(_WIN64) || defined(__LP64__)
  REQUIRE( nSize == 64 );
#else
  REQUIRE( nSize == 48 );
#endif
}

TEST_CASE("regression - A parser", "[regression][parse][7552]" ) {

     const char * expectations[] = {
       "%7D: \'foo\'\r\n",
       "%29: \'foo\'\r\n",
       "%24: \'foo\'\r\n"
     };

     const char * names[] = {
       "}",
       ")",
       "$"
     };

     for (int i = 0; i < 1; i += 1) {
     node_t * pn = NULL;

     const char * psExpected = expectations[i];
     const char * psName = names[i];

     pn = node_alloc();

     REQUIRE(pn->psWValue == NULL);

     node_set(pn, NODE_STRINGA, "foo");
     node_set_nameA( pn, psName );

     REQUIRE(pn->psWValue == NULL);

     FILE * pf = fopen(g_psFileName, "wb");
     node_dumpA( pn, pf, 0 );
     fclose(pf);

     char acBuffer[1024];
     pf = fopen(g_psFileName, "rb");
     fread(acBuffer, 1, sizeof(acBuffer), pf);
     fclose(pf);

     REQUIRE( strncmp( psExpected, acBuffer, 12 ) == 0 );

     REQUIRE(pn->psWValue == NULL);
     node_free(pn);

     pf = fopen( g_psFileName, "rb" );
     int nResult = node_parseA( pf, &pn );
     fclose(pf);

     REQUIRE(nResult == NP_NODE);
     REQUIRE(std::string(psName) == node_get_nameA(pn));
     REQUIRE(std::string("foo") == node_get_stringA(pn));

     REQUIRE(pn->psWValue == NULL);
     node_free(pn);
}
}

TEST_CASE("regression - W parser", "[regression][parse][7552]" ) {

     // TODO: implement node_dumpA_to_string()
     return;

     const wchar_t * expectations[] = {
       L"\xFEFF%7D: \'foo\'\r\n",
       L"\xFEFF%29: \'foo\'\r\n",
       L"\xFEFF%24: \'foo\'\r\n"
     };

     const wchar_t * names[] = {
       L"}",
       L")",
       L"$"
     };

     for (int i = 0; i < 1; i += 1) {
     node_t * pn = NULL;

     const wchar_t * psExpected = expectations[i];
     const wchar_t * psName = names[i];

     pn = node_alloc();

     REQUIRE(pn->psWValue == NULL);

     node_set(pn, NODE_STRINGW, L"foo");
     node_set_nameW( pn, psName );

     FILE * pf = fopen(g_psFileName, "wb");
     wchar_t wBOM = 0xFEFF;
     fwrite(&wBOM, sizeof(wchar_t), 1, pf);
     node_dumpW( pn, pf, 0 );
     fclose(pf);

     wchar_t acBuffer[1024];
     pf = fopen(g_psFileName, "rb");
     fread(acBuffer, 1, sizeof(acBuffer), pf);
     fclose(pf);

     REQUIRE( std::wstring(psExpected) == std::wstring(acBuffer) );

     node_free(pn);

     pf = fopen( g_psFileName, "rb" );
     int nResult = node_parseA( pf, &pn );
     fclose(pf);

     REQUIRE(nResult == NP_NODE);
     REQUIRE(std::wstring(psName) == node_get_nameW(pn));
     REQUIRE(std::wstring(L"foo") == node_get_stringW(pn));

     node_free(pn);
}
}

#define TS_ASSERT REQUIRE
#define TS_ASSERT_EQUALS(a,b)   REQUIRE( (a) == (b) )
#define TS_ASSERT_SAME_DATA(a,b,l)  REQUIRE( memcmp((a),(b),(l)) == 0 )
#define TS_ASSERT_THROWS_ANYTHING(a)  REQUIRE_THROWS(a)

TEST_CASE("regression 8400 - A parser", "[regression][parse][8400]" ) {
     node_t * pn = NULL;
     FILE * pf = NULL;

     pf = fopen( "i90demo.hst", "rb" );
     TS_ASSERT( pf != NULL );

     int nResult = node_parseA( pf, &pn );
     TS_ASSERT_EQUALS( NP_NODE, nResult );

     node_t * pnName = node_hash_getA( pn, "Name" );
     const char * psA = node_get_stringA( pnName );

     std::string s("I90Demo : Module 1,01,06 -- 1010601A CAD Drawing");

     TS_ASSERT_EQUALS( s, psA );

     node_free( pn );
     fclose( pf );
}

TEST_CASE("regression 8400 - W parser", "[regression][parse][8400]" ) {

     // TODO: fix W parser
     return;

     node_t * pn = NULL;
     FILE * pf = NULL;

     pf = fopen( "I90Demo.hst", "rb" );
     REQUIRE( pf != NULL );

     int nResult = node_parseW( pf, &pn );
     REQUIRE(NP_NODE == nResult );

     node_t * pnName = node_hash_getW( pn, L"Name" );
     const wchar_t * psW = node_get_stringW( pnName );

     std::wstring s(L"I90Demo : Module 1,01,06 -- 1010601A CAD Drawing");

     REQUIRE(s == std::wstring(psW) );

     node_free( pn );
     fclose( pf );
}

     const size_t TEST_DATA_SIZE = 17;

     void checkData( node_t * pn, data_t * ad, size_t nLengthIn, const char * sMessage ) {
          int nLengthOut;
          const data_t * pd = NULL;

          REQUIRE( nLengthIn <= TEST_DATA_SIZE );

          node_set_data(pn, nLengthIn, ad);
          pd = node_get_data(pn, &nLengthOut);

          REQUIRE(nLengthOut == nLengthIn);
          REQUIRE( memcmp(ad, pd, nLengthOut) == 0);
     }



TEST_CASE("basic node - alloc", "[basic][alloc]" ) {

     data_t ad[TEST_DATA_SIZE];

     for(size_t i = 0; i < TEST_DATA_SIZE; i += 1) {
          ad[i] = i;
     }

     node_t * pn = node_alloc();

     SECTION("can set int") {
          node_set(pn, NODE_INT, 3);
          REQUIRE(node_get_int(pn) == 3);
     }

     SECTION("can set float") {
          node_set(pn, NODE_REAL, 3.0);
          REQUIRE(node_get_int(pn) == 3.0);
     }

     SECTION("can set stringA") {
          node_set(pn, NODE_STRINGA, "Three");
          REQUIRE(std::string(node_get_stringA(pn)) == std::string("Three"));
     }

     SECTION("can set stringA with self") {
          node_set(pn, NODE_STRINGA, "Three");
          REQUIRE(std::string(node_get_stringA(pn)) == std::string("Three"));

          node_set(pn, NODE_STRINGA, node_get_stringA(pn));
          REQUIRE(std::string(node_get_stringA(pn)) == std::string("Three"));
     }

     SECTION("can set stringW") {
          node_set(pn, NODE_STRINGW, L"Three");
          REQUIRE(std::wstring(node_get_stringW(pn)) == std::wstring(L"Three"));
     }

     SECTION("can set stringW with self") {
          node_set(pn, NODE_STRINGW, L"Three");
          REQUIRE(std::wstring(node_get_stringW(pn)) == std::wstring(L"Three"));

          node_set(pn, NODE_STRINGW, node_get_stringW(pn));
          REQUIRE(std::wstring(node_get_stringW(pn)) == std::wstring(L"Three"));
     }

     SECTION("can set data to 1 byte") {
          checkData(pn, ad, 1u, "1 byte");
     }

     SECTION("can set data to 8 bytes") {
          checkData(pn, ad, 8u, "8 bytes");
     }

     SECTION("can set data to 15 bytes") {
          checkData(pn, ad, 15u, "15 bytes");
     }

     SECTION("can set data to 16 bytes") {
          checkData(pn, ad, 16u, "16 bytes");
     }

     SECTION("can set data to 17 bytes") {
          checkData(pn, ad, 17u, "17 bytes");
     }

     SECTION("test data size change: grow") {
       int nLengthOut;
       const data_t * pd = NULL;
       int nLengthIn = 17;

       node_set_data( pn, nLengthIn, ad );
       pd = node_get_data( pn, &nLengthOut );

       TS_ASSERT_EQUALS( nLengthOut, nLengthIn );
       TS_ASSERT_SAME_DATA( ad, pd, nLengthOut );

       nLengthIn = 15;
       node_set_data( pn, nLengthIn, pd );
       pd = node_get_data( pn, &nLengthOut );
       TS_ASSERT_EQUALS( nLengthOut, nLengthIn );
       TS_ASSERT_SAME_DATA( ad, pd, nLengthOut );

       nLengthIn = 17;
       TS_ASSERT_THROWS_ANYTHING( node_set_data( pn, nLengthIn, pd ) );
     }

     node_free( pn );
}

TEST_CASE("iconv", "[iconv]") {
  iconv_t cd = iconv_open("UTF-8", "UTF-16");

  wchar_t awBuffer[16] = {0};
  char acBuffer[16] = "3";
  size_t nC = 2;
  size_t nRoom = 16;

  char * pc = acBuffer;
  wchar_t * pw = awBuffer;

  errno = 0;
  //  size_t nResult = iconv(cd, &pc, &nC, (char**)&pw, &nRoom);

  mbstowcs(pw, pc, 3);

  REQUIRE(errno == 0);
  //  REQUIRE(nResult == 0);
  REQUIRE(awBuffer[0] == '3');
  REQUIRE(awBuffer[1] == 0);
  REQUIRE(std::wstring(awBuffer) == std::wstring(L"3"));
}

TEST_CASE("basic node - convert", "[basic][convert]" ) {

     data_t ad[TEST_DATA_SIZE];

     for(size_t i = 0; i < TEST_DATA_SIZE; i += 1) {
          ad[i] = i;
     }

     node_t * pn = node_alloc();

     SECTION("int to real") {
       node_set(pn, NODE_INT, 3);
       REQUIRE( node_get_real(pn) == 3.0 );
     }

     SECTION("int to stringA") {
       node_set(pn, NODE_INT, 3);
       REQUIRE( std::string(node_get_stringA(pn)) == std::string("3") );

       node_set(pn, NODE_INT, 4);
       REQUIRE( std::string(node_get_stringA(pn)) == std::string("4") );

       /* second time to get coverage when value already set */
       REQUIRE( std::string(node_get_stringA(pn)) == std::string("4") );
     }
     
     SECTION("int to stringW") {
       node_set(pn, NODE_INT, 3);
       const wchar_t * pW = node_get_stringW(pn);
       REQUIRE( pW != NULL );
       REQUIRE( std::wstring(pW) == std::wstring(L"3") );

       node_set(pn, NODE_INT, 4);
       REQUIRE( std::wstring(node_get_stringW(pn)) == std::wstring(L"4") );

       /* second time to get coverage when value already set */
       REQUIRE( std::wstring(node_get_stringW(pn)) == std::wstring(L"4") );
     }

     node_free(pn);

}
