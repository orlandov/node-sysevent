#include "common.h"
#include "subscription.h"
#include "strings.h"

#include <list>
#include <vector>
#include <algorithm>

using namespace v8;
using namespace node;

Persistent<FunctionTemplate> Subscription::constructor_template;
ev_async Subscription::eio_notifier;
std::list<Subscription *> Subscription::all_subscriptions;
std::list<sysevent_t *> Subscription::event_queue;

void EnqueueEvent(sysevent_t *ev) {
  int ev_sz = 0;
  sysevent_t *copy = NULL;

  // Push the event onto the queue.
  ev_sz = sysevent_get_size(ev);
  copy = (sysevent_t *) malloc(ev_sz);
  // XXX check return value
  bcopy(ev, copy, ev_sz);

  Subscription::event_queue.push_back(copy);
}

void Subscription::event_handler(sysevent_t *ev) {
  EnqueueEvent(ev);
  
  // Trigger the notifier.
  if (Subscription::all_subscriptions.size() > 0) {
    ev_async_send(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  }
}

void Event(EV_P_ ev_async *watcher, int revents) {
  HandleScope scope;
  pid_t pid;
  Subscription *subscr(NULL);

  // XXX TODO find the right subscription object, not just the first.
  //subscr = Subscription::all_subscriptions.front();


  std::list<sysevent_t *>::iterator iter_event = Subscription::event_queue.begin();
  for (; iter_event != Subscription::event_queue.end(); iter_event++) {
    sysevent_t *ev = *iter_event;
    char *class_name = sysevent_get_class_name(ev);
    char *subclass_name = sysevent_get_subclass_name(ev);

    std::list<Subscription *>::iterator iter_sub
      = Subscription::all_subscriptions.begin();

    for (; iter_sub != Subscription::all_subscriptions.end(); iter_sub++) {
      subscr = *iter_sub;
      // Abort cycle if class name does not match.
      if (strcmp(subscr->subscribed_class, class_name)) {
        continue;
      }

      std::vector<char *>::iterator iter_subclasses = subscr->subscribed_subclasses.begin();
      for (; iter_subclasses != subscr->subscribed_subclasses.end(); iter_subclasses++) {
        if (strcmp(*iter_subclasses, subclass_name)) {
          continue;
        }

        sysevent_get_pid(ev, &pid);
        // Create new JavaScript event hash
        // { pub: 'foo', vendor: 'bar', class: 'baz', subclass: 'quux' }
        v8::Local<v8::Object> event = v8::Object::New();
        event->Set(String::New("class"), String::New(class_name));
        event->Set(String::New("subclass"),
          String::New(subclass_name));
        event->Set(String::New("vendor"),
          String::New(sysevent_get_vendor_name(ev)));
        event->Set(String::New("pub"),
          String::New(sysevent_get_pub_name(ev)));
        event->Set(String::New("pid"), Number::New(pid));
        event->Set(String::New("seq"), Number::New(sysevent_get_seq(ev)));

        subscr->Emit(String::NewSymbol("event"), 1,
          reinterpret_cast<Handle<Value> *>(&event));
        free(ev);
      }
    }
  }
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

  if (Subscription::all_subscriptions.size() == 0) {
    // Initialize the async notifier and start watching it. This is what let
    // us know when the event_handler function has added an to the event queue.
    ev_async_init(&Subscription::eio_notifier, &Event);
    ev_async_start(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  }

  // Get a reference to our related Subscription object.
  Subscription *subscr = ObjectWrap::Unwrap<Subscription>(args.This());

  // Record the class and subclass to which we are subscribing.
  subscr->subscribed_class = strdup(*event_class);
  subscr->subscribed_subclasses.push_back(strdup(*event_subclass));

  // Create and check sysevent bind handle.
  subscr->shp = sysevent_bind_handle(&Subscription::event_handler);
  if (subscr->shp == NULL) {
    // XXX fix
    exit(1);
  }

  // Subscribe to events.
  Subscription::all_subscriptions.push_back(subscr);

  if (sysevent_subscribe_event(subscr->shp, *event_class,
        (const char **)&(subscr->subscribed_subclasses[0]), 1) != 0) {
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

  // Free class string
  free(subscr->subscribed_class);

  // Free subclass strings
  std::vector<char *>::iterator iter = subscr->subscribed_subclasses.begin();
  for (; iter != subscr->subscribed_subclasses.end(); iter++) {
    free(*iter);
  }

  all_subscriptions.remove(subscr);

  if (Subscription::all_subscriptions.size() == 0) {
    ev_async_stop(EV_DEFAULT_UC_ &Subscription::eio_notifier);
  }

  subscr->Unref();
  ev_unref(EV_DEFAULT_UC);

  return Undefined();
}

