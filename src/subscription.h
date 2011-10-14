#ifndef NODE_SYSEVENT_SUBSCRIPTION_H
#define NODE_SYSEVENT_SUBSCRIPTION_H

#include <v8.h>
#include <node.h>
#include <node_events.h>

using namespace v8;
using namespace node;

class Subscription : public EventEmitter {

  public:
    static Persistent<FunctionTemplate> constructor_template;
    static void Init(v8::Handle<Object> target);

  protected:
    Subscription() : EventEmitter() {}

    ~Subscription() {}

    static Handle<Value> New(const Arguments &args);
    static Handle<Value> Subscribe(const Arguments& args);

  private:

};

#endif
