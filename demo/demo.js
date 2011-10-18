var sysevent = require('sysevent');

var sub = new sysevent.Subscription();

sub.on('event', function () {
  console.log("GOt a sysevent");
});

sub.subscribe("testclass", "subclass1");
