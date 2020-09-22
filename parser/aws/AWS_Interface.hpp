#pragma once

#include "ReceiverInterface.hpp"

#include <memory>

namespace NCBI
{
    namespace Logging
    {
        /* we are using the receiver for parsing original lines as well as for reconstructing original lines! */
        struct AWSReceiver : public ReceiverInterface
        {
            using ReceiverInterface::set;

            AWSReceiver( std::unique_ptr<FormatterInterface> & fmt );

            typedef enum {
                owner = LastMemberId+1,
                bucket,
                time,
                requester,
                request_id,
                operation,
                key,
                res_code,
                error,
                res_len,
                obj_size,
                total_time,
                turnaround_time,
                version_id,
                host_id,
                sig_ver,
                cipher_suite,
                auth_type,
                host_header,
                tls_version,
                _extra,
                AWS_LastMemberId = _extra
            } AWS_Members; // all are t_str values
            virtual void set( AWS_Members m, const t_str & ); // will invoke set( ReceiverInterface::Members ) if necessary
        };

        class AWSParseBlockFactory : public ParseBlockFactoryInterface
        {
        public:
            virtual ~AWSParseBlockFactory();
            virtual std::unique_ptr<ParseBlockInterface> MakeParseBlock() const;
        };

        class AWSReverseBlockFactory : public ParseBlockFactoryInterface
        {
        public:
            virtual ~AWSReverseBlockFactory();
            virtual std::unique_ptr<ParseBlockInterface> MakeParseBlock() const;
        };

    }
}
