#pragma once

#include "Entities.h"
#include "Exceptions.h"
#include "private/SubscriptionBasePrivate.h"
#include "private/defines.h"
#include "private/utils.h"

namespace NatsMq
{
    namespace Js
    {
        class SubscriptionBaseTemplate : public SubscriptionBasePrivate<NatsJsSubscriptionPtr, decltype(jsSubDeleter)>
        {
        public:
            SubscriptionBaseTemplate(jsCtx* ctx)
                : SubscriptionBasePrivate(nullptr, &jsSubDeleter)
                , _ctx(ctx)
            {
            }

            template <typename Creator, typename... Args>
            SubscriptionBaseTemplate(jsCtx* ctx, Creator creator, const Js::SubscriptionOptions& options, Args... args)
                : SubscriptionBasePrivate(nullptr, &jsSubDeleter)
                , _ctx(ctx)
            {
                auto cnatsSubOptions = Js::toJsCnatsSubOptions(options);

                natsSubscription* natsSub{ nullptr };

                jsErrCode  jerr;
                const auto status = creator(&natsSub, ctx, args..., &cnatsSubOptions, &jerr);

                jsExceptionIfError(status, jerr);

                _sub.reset(natsSub);
            }

            ~SubscriptionBaseTemplate() override;

            Js::SubscriptionMismatch mismatch() const;

            Js::Consumer consumerInfo() const;

        protected:
            jsCtx* _ctx;
        };
    }
};
