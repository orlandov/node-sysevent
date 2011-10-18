#include "common.h"
#include "subscription.h"

using namespace v8;
using namespace node;

Persistent<FunctionTemplate> Subscription::constructor_template;
ev_async Subscription::eio_notifier;
subscriber_list_node *Subscription::all_subscriptions;
subscriber_list_node *Subscription::last_subscription;

void Subscription::event_handler(sysevent_t *ev) {
  printf("the handler\n");

  ev_async_send(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  
  //subscriber_list_node *cursor = Subscription::all_subscriptions;
  //Subscription *sub = cursor->sub;
}

void Event(EV_P_ ev_async *watcher, int revents) {
  printf("This is the event watcher");
  subscriber_list_node *cursor = Subscription::all_subscriptions;
  Subscription *sub = cursor->sub;
  sub->Emit(String::NewSymbol("event"), 0, NULL);
}

void Subscription::Init(v8::Handle<Object> target) {
  printf("subscription init\n");
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(t);                          
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);                   
  constructor_template->SetClassName(String::NewSymbol("Subscription"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "subscribe", Subscribe);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "unsubscribe", Unsubscribe);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "post", Post);
                                                                                        
  target->Set(v8::String::NewSymbol("Subscription"),
    constructor_template->GetFunction());
}

Subscription::Subscription() 
: EventEmitter() {

  this->shp = sysevent_bind_handle(&Subscription::event_handler);

  Subscription::all_subscriptions
    = (subscriber_list_node *) malloc(sizeof(subscriber_list_node));
  
  Subscription::all_subscriptions->sub = this;

  ev_async_init(&Subscription::eio_notifier, &Event);
  ev_async_start(EV_DEFAULT_UC_ &Subscription::eio_notifier);

  if (this->shp == NULL) {
    printf("shp not good\n");
    exit(1);
  }
}

Subscription::~Subscription() {
}

Handle<Value> Subscription::New(const Arguments &args) {
  printf("subscription new\n");
  HandleScope scope;
  Subscription *subObj = new Subscription();
  subObj->Wrap(args.This());
  return args.This();
}

Handle<Value> Subscription::Subscribe(const Arguments &args) {
  printf("subscription subscribe\n");
  HandleScope scope;

  REQUIRE_STRING_ARG(args, 0, event_class);
  REQUIRE_STRING_ARG(args, 1, event_subclass);

  Subscription *subObj = ObjectWrap::Unwrap<Subscription>(args.This());

  const char *subclasses[] = { EC_SUB_ALL };

  if (sysevent_subscribe_event(subObj->shp, *event_class, subclasses, 1) != 0) {
    printf("syevent subscrib failed?\n");
    sysevent_unbind_handle(subObj->shp);
    exit(1); // XXX fix
  }

  printf("syevent subscrib reffing\n");
  subObj->Ref();
  ev_ref(EV_DEFAULT_UC);

  return Undefined();
}

Handle<Value> Subscription::Unsubscribe(const Arguments &args) {
  printf("subscription unsubscribe\n");
  HandleScope scope;
  Subscription *subObj = ObjectWrap::Unwrap<Subscription>(args.This());
  sysevent_unbind_handle(subObj->shp);
  ev_async_stop(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  ev_unref(EV_DEFAULT_UC);
  subObj->Unref();
  return Undefined();
}

Handle<Value> Subscription::Post(const Arguments &args) {
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

