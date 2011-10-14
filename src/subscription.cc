#include "subscription.h"

using namespace v8;
using namespace node;

void Subscription::Init(v8::Handle<Object> target) {
  printf("subscription init\n");
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(t);                          
  constructor_template->Inherit(EventEmitter::constructor_template);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);                   
  constructor_template->SetClassName(String::NewSymbol("Subscription"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "subscribe", Subscribe);
                                                                                        
  target->Set(v8::String::NewSymbol("Subscription"),
    constructor_template->GetFunction());
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
  return Undefined();
}

Persistent<FunctionTemplate> Subscription::constructor_template;
