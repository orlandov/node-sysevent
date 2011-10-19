## NAME

node-sysevent - Node.js interface to sysevent

## SYNOPSIS

    var sysevent = require('sysevent');

    sub.on('event', function (event) {
      console.log("Event received!");
      console.log("Class was " + event.class);
      console.log("Subclass was " + event.subclass);
      console.log("Publisher was " + event.pub);
      console.log("Publisher pid was " + event.pid);
      console.log("Publisher vendor was " + event.vendor);

      // stop watching for events
      sub.unsubscribe();
    });

    // watch for events
    sub.subscribe("herp", "derp");

    // post an event
    sysevent.post("herp", "derp", "ovtech industries", process.argv[1]);

## DESCRIPTION

sysevent is a low-level mechanism via which publishers and consumers can communicate within a machine. node-sysevent provides an asynchronous interface around the libsyevent library to access these facilities.

## TODO

- Allow user to specify a limited `attrs` object when `post()`ing an event. This will be converted into an nvlist_t object and passed to libsysevent.
- If an `attrs` nvlist_t object is set on an incoming event, convert it to a javascript object and attach to the `event` object given on the `event` callback.  
- Allow user to specify more than one subclass when subscribing.
- Crudely measured the `syevent_subscribe_events` call taking between 0.1 and 0.2 milliseconds to execute. Should punt into the libev threadpool.

## ENVIRONMENT

Should work whever libsysevent is installed and working, i.e. SmartOS, IllumOS, OpenSolaris, and Solaris. Let me know. ;-) 

## BUGS

Warning: some things are not quite implemented yet so use at your own risk.

## AKNOWLEDGEMENTS

* http://bravenewmethod.wordpress.com/2011/03/30/callbacks-from-threaded-node-js-c-extension/
* http://download.oracle.com/docs/cd/E19963-01/html/821-1470/sysevent-subscribe-event-3sysevent.html
* http://blogs.oracle.com/eschrock/entry/dtrace_sysevent_provider
* http://www.filibeto.org/sun/lib/solaris10-docs/E19253-01/816-5180/6mbbf02g5/index.html
* http://www.unix.com/man-page/OpenSolaris/3nvpair/nvlist_alloc/
* https://github.com/mcavage/node-zutil/blob/master/src/zutil_common.h

## AUTHOR

Orlando Vazquez < ovazquez@gmail.com >
