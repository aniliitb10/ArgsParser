#include <gtest/gtest.h>
#include <string>

// A handwritten macro to test the exception and the exception message
#define EXPECT_EXCEPTION( TRY_BLOCK, EXCEPTION_TYPE, MESSAGE )        \
try                                                                   \
{                                                                     \
    TRY_BLOCK;                                          \
    FAIL() << "exception '" << MESSAGE << "' not thrown at all!";     \
}                                                                     \
catch( const EXCEPTION_TYPE& e )                                      \
{                                                                     \
    EXPECT_EQ( std::string(MESSAGE), e.what() )                       \
        << " exception message is incorrect. Expected the following " \
           "message:\n\n"                                             \
        << MESSAGE << "\n";                                           \
}                                                                     \
catch( ... )                                                          \
{                                                                     \
    FAIL() << "exception '" << MESSAGE                                \
           << "' not thrown with expected type '" << #EXCEPTION_TYPE  \
           << "'!";                                                   \
}


