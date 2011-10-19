#ifndef NODE_SYSEVENT_SUBSCRIPTION_H
#define NODE_SYSEVENT_SUBSCRIPTION_H

#include <v8.h>
#include <node.h>
#include <node_events.h>

#include <list>
#include <vector>

#include <libsysevent.h>
#include <libnvpair.h>

using namespace v8;
using namespace node;

class Subscription: public EventEmitter {
  public:
    static Persistent<FunctionTemplate> constructor_template;
    static void Init(v8::Handle<Object> target);

    static void event_handler(sysevent_t *ev);
    static ev_async eio_notifier;
    static std::list<Subscription *> all_subscriptions;
    static std::list<sysevent_t *> event_queue;
    static int subscription_count;

  protected:
    Subscription();
    ~Subscription();

    static Handle<Value> New(const Arguments &args);
    static Handle<Value> Subscribe(const Arguments& args);
    static Handle<Value> Unsubscribe(const Arguments& args);

    char *subscribed_class;
    std::vector<char *> subscribed_subclasses;

  private:
    sysevent_handle_t *shp;
};

#endif
