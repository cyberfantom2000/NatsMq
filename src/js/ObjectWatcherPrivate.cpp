#include "ObjectWatcherPrivate.h"

#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

namespace
{
    Js::SubscriptionOptions subOptions(const std::string& stream)
    {
        Js::SubscriptionOptions opts;
        opts.stream  = stream;
        opts.ordered = true;
        return opts;
    }
}

Js::ObjectWatcherPrivate::ObjectWatcherPrivate(jsCtx* ctx, const std::string& subject, const std::string& stream)
    : _sub(ctx)
{
    _sub.registerListener(subject, subOptions(stream), //
                          [this](Js::IncomingMessage msg) {
                              if (!_removed && !_updated)
                                  return;

                              const auto meta = deserializeObjectMeta(msg.msg);

                              if (meta.deleted)
                              {
                                  if (_removed)
                                      _removed(std::move(meta));
                              }
                              else
                              {
                                  if (_updated)
                                      _updated(std::move(meta));
                              }
                          });
}

void Js::ObjectWatcherPrivate::stop() const
{
    _sub.unsubscribe();
}

void Js::ObjectWatcherPrivate::updated(ObjectWatchCb cb)
{
    _updated = std::move(cb);
}

void Js::ObjectWatcherPrivate::removed(ObjectWatchCb cb)
{
    _removed = std::move(cb);
}
