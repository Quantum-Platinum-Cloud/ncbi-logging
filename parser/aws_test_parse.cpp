#include <gtest/gtest.h>

#include <sstream>
#include <vector>

#include "log_lines.hpp"
#include "helper.hpp"

using namespace std;
using namespace NCBI::Logging;

struct SRequest
{
    string server;
    string method;
    string path;
    string params;
    string vers;

    SRequest& operator= ( const t_request &req )
    {
       server = ToString( req.server );
       method = ToString( req.method );
       path = ToString( req.path );
       params = ToString( req.params );
       vers = ToString( req.vers );
       return *this;
    }
};

struct SLogAWSEvent
{
    string      ip;
    t_timepoint time;
    SRequest    request;
    int64_t     res_code;
    int64_t     res_len;
    string      referer;
    string      agent;

    string      owner;
    string      bucket;
    string      requester;
    string      request_id;
    string      operation;
    string      key;
    string      error;
    int64_t     obj_size;
    int64_t     total_time;
    int64_t     turnaround_time;
    string      version_id;
    string      host_id;
    string      cipher_suite;
    string      auth_type;
    string      host_header;
    string      tls_version;

    SLogAWSEvent( const LogAWSEvent &ev )
    {
        ip          = ToString( ev . ip );
        time        = ev . time;
        request     = ev . request;
        res_code    = ev . res_code;
        res_len     = ev . res_len;
        referer     = ToString( ev . referer );
        agent       = ToString( ev . agent );

        owner       = ToString( ev . owner );
        bucket      = ToString( ev . bucket );
        requester   = ToString( ev . requester );
        request_id  = ToString( ev . request_id );
        operation   = ToString( ev . operation );
        key         = ToString( ev . key );
        error       = ToString( ev . error );
        obj_size    = ev . obj_size;
        total_time  = ev . total_time;
        turnaround_time = ev . turnaround_time;
        version_id  = ToString( ev . version_id );
        host_id     = ToString( ev . host_id );
        cipher_suite = ToString( ev . cipher_suite );
        auth_type   = ToString( ev . auth_type );
        host_header = ToString( ev . host_header );
        tls_version = ToString( ev . tls_version );
    }
};

struct TestLogLines : public AWS_LogLines
{
    virtual int unrecognized( const t_str & text )
    {
        m_unrecognized . push_back( ToString( text ) );
        return 0;
    }

    virtual int acceptLine( const LogAWSEvent & event )
    {
        last_accepted = new SLogAWSEvent( event );
        return 0;
    }

    virtual int rejectLine( const LogAWSEvent & event )
    {
        last_rejected = new SLogAWSEvent( event );
        return 0;
    }

    vector< string > m_unrecognized;
    SLogAWSEvent * last_accepted = nullptr;
    SLogAWSEvent * last_rejected = nullptr;

    virtual ~TestLogLines()
    {
        delete last_accepted;
        delete last_rejected;
    }
};

TEST ( TestParse, InitDestroy )
{
    TestLogLines lines;
    AWS_Parser p( lines );
}

class TestParseFixture : public ::testing::Test
{
public:
    virtual void SetUp() {}
    virtual void TearDown() {}

    const SLogAWSEvent * parse_aws( const char * input, bool debug_it = false )
    {
        std::istringstream inputstream( input );
        {
            AWS_Parser p( m_lines, inputstream );
            p.setDebug( debug_it );
            if ( !p.parse() ) throw logic_error( "parsing failed" );
            if ( nullptr == m_lines.last_accepted ) throw logic_error( "last_accepted is null" );
            return m_lines . last_accepted;
        }
    }

    TestLogLines m_lines;
};

TEST_F ( TestParseFixture, Empty )
{
    std::istringstream input ( "" );
    {
        AWS_Parser p( m_lines, input );
        ASSERT_TRUE( p.parse() );
    }
}

TEST_F ( TestParseFixture, AWS )
{
    const char * InputLine =
    "922194806485875312b252374a3644f1feecd16802a50d4729885c1d11e1fd37 s"
    "ra-pub-src-14 "
    "[09/May/2020:22:07:21 +0000] "
    "18.207.254.142 "
    "arn:aws:sts::783971887864:assumed-role/sra-developer-instance-profile-role/i-0d76b79326eb0165a "
    "B6301F55C2486C74 "
    "REST.PUT.PART "
    "SRR9612637/DRGHT.TC.307_interleaved.fq.1 "
    "\"PUT /SRR9612637/DRGHT.TC.307_interleaved.fq.1?partNumber=1&uploadId=rl6yL37lb4xUuIa9RvC0ON4KgDqJNvtwLoquo_cALj95v4njBOTUHpISyEjOaMG30lVYAo5eR_UEXo4dVJjUJA3SfjJtKjg30rvVEpg._Z9DZZo8S6oUjXHGDCW15EVzLZcJMgRG6N7J8d.42.lMAw-- HTTP/1.1\" "
    "200 "
    "- "
    "- "
    "8388608 "
    "557 "
    "12 "
    "\"-\" "
    "\"aws-cli/1.16.102 \\\"Python\\\"/2.7.16 Linux/4.14.171-105.231.amzn1.x86_64 botocore/1.12.92\" "
    "- "
    "fV92QmqOf5ZNYPIj7KZeQWiqAOFqdFtMlOn82aRYjwQHt8QfsWfS3TTOft1Be+bY01d9TObk5Qg= "
    "SigV4 ECDHE-RSA-AES128-GCM-SHA256 "
    "AuthHeader "
    "sra-pub-src-14.s3.amazonaws.com "
    "TLSv1.2";
    "\n";

    const SLogAWSEvent &e = *( parse_aws( InputLine ) );

    ASSERT_EQ( "922194806485875312b252374a3644f1feecd16802a50d4729885c1d11e1fd37", e.owner );
    ASSERT_EQ( "sra-pub-src-14", e.bucket );

    ASSERT_EQ( 9, e.time.day );
    ASSERT_EQ( 5, e.time.month );
    ASSERT_EQ( 2020, e.time.year );
    ASSERT_EQ( 22, e.time.hour );
    ASSERT_EQ( 7, e.time.minute );
    ASSERT_EQ( 21, e.time.second );
    ASSERT_EQ( 0, e.time.offset );

    ASSERT_EQ( "18.207.254.142", e.ip );

    ASSERT_EQ( "arn:aws:sts::783971887864:assumed-role/sra-developer-instance-profile-role/i-0d76b79326eb0165a", e.requester );
    ASSERT_EQ( "B6301F55C2486C74", e.request_id );
    ASSERT_EQ( "REST.PUT.PART", e.operation );
    ASSERT_EQ( "SRR9612637/DRGHT.TC.307_interleaved.fq.1", e.key );

    ASSERT_EQ( "PUT", e.request.method );
    ASSERT_EQ( "/SRR9612637/DRGHT.TC.307_interleaved.fq.1", e.request.path );
    ASSERT_EQ( "partNumber=1&uploadId=rl6yL37lb4xUuIa9RvC0ON4KgDqJNvtwLoquo_cALj95v4njBOTUHpISyEjOaMG30lVYAo5eR_UEXo4dVJjUJA3SfjJtKjg30rvVEpg._Z9DZZo8S6oUjXHGDCW15EVzLZcJMgRG6N7J8d.42.lMAw--",
               e.request.params );
    ASSERT_EQ( "HTTP/1.1", e.request.vers );
    ASSERT_EQ( 200, e.res_code );
    ASSERT_EQ( "", e.error );
    ASSERT_EQ( 0, e.res_len );
    ASSERT_EQ( 8388608, e.obj_size );
    ASSERT_EQ( 557, e.total_time );
    ASSERT_EQ( 12, e.turnaround_time );
    ASSERT_EQ( "-", e.referer );
    ASSERT_EQ( "aws-cli/1.16.102 \"Python\"/2.7.16 Linux/4.14.171-105.231.amzn1.x86_64 botocore/1.12.92", e.agent );
    ASSERT_EQ( "-", e.version_id );
    ASSERT_EQ( "fV92QmqOf5ZNYPIj7KZeQWiqAOFqdFtMlOn82aRYjwQHt8QfsWfS3TTOft1Be+bY01d9TObk5Qg=", e.host_id );
    ASSERT_EQ( "SigV4 ECDHE-RSA-AES128-GCM-SHA256", e.cipher_suite );
    ASSERT_EQ( "AuthHeader", e.auth_type );
    ASSERT_EQ( "sra-pub-src-14.s3.amazonaws.com", e.host_header );
    ASSERT_EQ( "TLSv1.2", e.tls_version );
}

TEST_F ( TestParseFixture, AWS_total_time_is_dash )
{
    const char * InputLine =
    "7dd4dcfe9b004fb7433c61af3e87972f2e9477fa7f0760a02827f771b41b3455 sra-pub-src-1 [25/May/2020:22:54:57 +0000] 52.54.203.43 arn:aws:sts::783971887864:assumed-role/sra-developer-instance-profile-role/i-04b64e15519efb678 EF87C0499CB7FDCA REST.HEAD.OBJECT ERR792423/m150101_223627_42225_c100719502550000001823155305141526_s1_p0.bas.h5.1 \"HEAD /ERR792423/m150101_223627_42225_c100719502550000001823155305141526_s1_p0.bas.h5.1 HTTP/1.1\" 200 - - 1318480 30 - \"-\" \"aws-cli/1.16.249 Python/2.7.16 Linux/4.14.138-89.102.amzn1.x86_64 botocore/1.12.239\" - Is1QS5IgOb006L7yAqIz7zKBtUIxbAZgzvM1aQbbSHXeKUDoSVfPXro2v1AN4gf7Ek5VTV2FeV8= SigV4 ECDHE-RSA-AES128-GCM-SHA256 AuthHeader sra-pub-src-1.s3.amazonaws.com TLSv1.2";
    
    const SLogAWSEvent &e = *( parse_aws( InputLine ) );

    ASSERT_EQ( 200, e.res_code );
    ASSERT_EQ( "", e.error );
    ASSERT_EQ( 0, e.res_len );
    ASSERT_EQ( 1318480, e.obj_size );
    ASSERT_EQ( 30, e.total_time );
    ASSERT_EQ( 0, e.turnaround_time );
    ASSERT_EQ( "-", e.referer );
}


/*
*/
//TODO: GCP
//TODO AWS version with error-code
//TODO handle escaped quotes in quoted string
//TODO: various line endings
//TODO: rejected lines with more than IP recognized


extern "C"
{
    int main ( int argc, const char * argv [], const char * envp []  )
    {
        testing :: InitGoogleTest ( & argc, ( char ** ) argv );
        return RUN_ALL_TESTS ();
    }
}
