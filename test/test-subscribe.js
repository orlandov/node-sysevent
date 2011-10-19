require.paths.push(__dirname+'/../lib');
var sysevent = require('sysevent');

exports["test subscribing to all events"]
= function (test) {
  var sub = new sysevent.Subscription();

  sub.on('event', function (event) {
    console.log("Got a sysevent:");
    console.dir(event);
    test.equal(event.class, 'xxx', 'class name should match');
    test.equal(event.subclass, 'yyy', 'subclass name should match');
    test.equal(event.vendor, 'joyent', 'vendor name should match');
    test.equal(event.pid, process.pid, 'process pid name should match');
    test.equal(event.pub, process.argv[1], 'publisher name should match');
    sub.unsubscribe();
    test.done();
  });

  sub.subscribe("xxx", "yyy");

  var event = { key1: 'value' };
  sysevent.post("xxx", "yyy", "joyent", process.argv[1], event)
}
