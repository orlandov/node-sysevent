#include "common.h"
#include "subscription.h"
#include "strings.h"

using namespace v8;
using namespace node;

Persistent<FunctionTemplate> Subscription::constructor_template;
ev_async Subscription::eio_notifier;
subscription_node *Subscription::all_subscriptions = NULL;
subscription_node *Subscription::last_subscription = NULL;
event_node *Subscription::event_queue = NULL;
int Subscription::subscription_count = 0;

void EnqueueEvent(sysevent_t *ev) {
  int ev_sz = 0;
  sysevent_t *copy = NULL;
  event_node *cur = NULL;

  // Push the event onto the queue.
  ev_sz = sysevent_get_size(ev);
  copy = (sysevent_t *) malloc(ev_sz);
  // XXX check return value
  bcopy(ev, copy, ev_sz);

  cur = Subscription::event_queue;
  if (!cur) {
    Subscription::event_queue = cur =
     (event_node *) malloc(sizeof(event_node));
    // XXX check return value
  }
  else {
     while((cur = cur->next));
  }

  cur->ev = copy;
  cur->next = NULL;
}

void Subscription::event_handler(sysevent_t *ev) {
  EnqueueEvent(ev);
  
  // Trigger the notifier.
  ev_async_send(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  
  //subscription_node *cursor = Subscription::all_subscriptions;
  //Subscription *sub = cursor->sub;
}

void Event(EV_P_ ev_async *watcher, int revents) {
  HandleScope scope;
  Subscription *subscr = NULL;
  subscription_node *cursor = NULL;

  // XXX TODO find the right subscription object, not just the first.
  cursor = Subscription::all_subscriptions;
  subscr = cursor->sub;

  event_node *cur = Subscription::event_queue;
  pid_t pid;
  int seq;

  do {
    // Create new JavaScript event hash
    // { pub: 'foo', vendor: 'bar', class: 'baz', subclass: 'quux' }
    sysevent_t *ev = cur->ev;
    sysevent_get_pid(ev, &pid);
    seq = sysevent_get_seq(ev);
    v8::Local<v8::Object> event = v8::Object::New();
    event->Set(String::New("class"), String::New(sysevent_get_class_name(ev)));
    event->Set(String::New("subclass"),
      String::New(sysevent_get_subclass_name(ev)));
    event->Set(String::New("vendor"), String::New(sysevent_get_vendor_name(ev)));
    event->Set(String::New("pub"), String::New(sysevent_get_pub_name(ev)));
    event->Set(String::New("pid"), Number::New(pid));
    event->Set(String::New("seq"), Number::New(seq));

    subscr->Emit(String::NewSymbol("event"), 1,
      reinterpret_cast<Handle<Value> *>(&event));
    free(cur->ev);
  } while ((Subscription::event_queue = cur->next));
}

void Subscription::Init(v8::Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(t);                          
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);                   
  constructor_template->SetClassName(String::NewSymbol("Subscription"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "subscribe", Subscribe);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "unsubscribe", Unsubscribe);
                                                                                        
  target->Set(v8::String::NewSymbol("Subscription"),
    constructor_template->GetFunction());
}

Subscription::Subscription() : EventEmitter() {} 

Subscription::~Subscription() {}

Handle<Value> Subscription::New(const Arguments &args) {
  HandleScope scope;
  Subscription *subscr = new Subscription();
  subscr->Wrap(args.This());
  return args.This();
}

Handle<Value> Subscription::Subscribe(const Arguments &args) {
  HandleScope scope;

  REQUIRE_STRING_ARG(args, 0, event_class);
  REQUIRE_STRING_ARG(args, 1, event_subclass);

  if (Subscription::subscription_count++ == 0) {
    // Initialize the async notifier and start watching it. This is what let
    // us know when the event_handler function has added an to the event queue.
    ev_async_init(&Subscription::eio_notifier, &Event);
    ev_async_start(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  }

  // Get a reference to our related Subscription object.
  Subscription *subscr = ObjectWrap::Unwrap<Subscription>(args.This());

  // Record the class and subclass to which we are subscribing.
  subscr->subscribed_class = strdup(*event_class);
  subscr->num_subclasses = 1;
  subscr->subscribed_subclasses
    = (char **) malloc(sizeof(char) * subscr->num_subclasses);
  subscr->subscribed_subclasses[0] = strdup(*event_subclass);

  // Create and check sysevent bind handle.
  subscr->shp = sysevent_bind_handle(&Subscription::event_handler);
  if (subscr->shp == NULL) {
    // XXX fix
    exit(1);
  }

  // Keep track of this subscription information so we can traverse it when an
  // event is fired.
  subscription_node *cur
    = (subscription_node *) malloc(sizeof(subscription_node));

  cur->sub = subscr;
  cur->next = Subscription::all_subscriptions;
  Subscription::all_subscriptions = cur;

  // Subscribe to events.
  const char *subclasses[] = { EC_SUB_ALL };

  if (sysevent_subscribe_event(subscr->shp, *event_class, subclasses, 1) != 0) {
    sysevent_unbind_handle(subscr->shp);
    exit(1); // XXX fix
  }

  // Increate the ev and gc reference counts.
  subscr->Ref();
  ev_ref(EV_DEFAULT_UC);

  return Undefined();
}

Handle<Value> Subscription::Unsubscribe(const Arguments &args) {
  HandleScope scope;
  Subscription *subscr = ObjectWrap::Unwrap<Subscription>(args.This());
  sysevent_unbind_handle(subscr->shp);

  if (--Subscription::subscription_count == 0) {
    ev_async_stop(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  }

  subscr->Unref();
  ev_unref(EV_DEFAULT_UC);

  return Undefined();
}

