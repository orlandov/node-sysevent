var sysevent = require('sysevent');

var sub1 = sysevent.subscribe("foo", "bar", function (event) {
  console.log("Got an event");
});

/*
sub1.on('event', function (class, subclass, attrs) {
  console.log("There was an event");
});

sub1.subscribe('class1', ['subclass1', 'subclass2']);
*/

//sysevent.subscribe('class1', ['subclass1', 'subclass2'], function (event) {
//  console.dir(event);
//});
