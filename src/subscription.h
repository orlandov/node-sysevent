#ifndef NODE_SYSEVENT_SUBSCRIPTION_H
#define NODE_SYSEVENT_SUBSCRIPTION_H

#include <libsysevent.h>
#include <libnvpair.h>

#include <v8.h>
#include <node.h>
#include <node_events.h>

using namespace v8;
using namespace node;

struct subscriber_list_node;

class Subscription: public EventEmitter {
  public:
    static Persistent<FunctionTemplate> constructor_template;
    static void Init(v8::Handle<Object> target);

    static void event_handler(sysevent_t *ev);
    static ev_async eio_notifier;
    static subscriber_list_node *all_subscriptions;
    static subscriber_list_node *last_subscription;

  protected:
    Subscription();
    ~Subscription();

    static Handle<Value> New(const Arguments &args);
    static Handle<Value> Subscribe(const Arguments& args);
    static Handle<Value> Unsubscribe(const Arguments& args);
    static Handle<Value> Post(const Arguments& args);

  private:
    char *subscribed_class;
    char *subscribed_subclasses[];

    sysevent_handle_t *shp;
};

struct subscriber_list_node {
  Subscription *sub;

  subscriber_list_node *next;
  subscriber_list_node *prev;
};


#endif
