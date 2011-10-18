#include <v8.h>
#include <node.h>
#include <node_events.h>

#include <libsysevent.h>
#include <libnvpair.h>

#include "common.h"
#include "subscription.h"

using namespace v8;
using namespace node;

Handle<Value> Post(const Arguments &args) {
  HandleScope scope;

  REQUIRE_STRING_ARG(args, 0, event_class);
  REQUIRE_STRING_ARG(args, 1, event_subclass);
  REQUIRE_STRING_ARG(args, 2, vendor);
  REQUIRE_STRING_ARG(args, 3, process);

  printf("Posting event %s:%s\n", *event_class, *event_subclass);

  int err;
  sysevent_id_t eid;

  err = sysevent_post_event(*event_class, *event_subclass, *vendor, *process,
    NULL, &eid);

  return Undefined();
}


extern "C" void init (v8::Handle<Object> target) {
  target->Set(v8::String::NewSymbol("post"),
    FunctionTemplate::New(Post)->GetFunction());
  Subscription::Init(target);
}

