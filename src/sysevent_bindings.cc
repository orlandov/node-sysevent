#include <v8.h>
#include <node.h>
#include <node_events.h>

#include "subscription.h"

extern "C" void init (v8::Handle<Object> target) {
  Subscription::Init(target);
}

