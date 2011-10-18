var bindings = require('../build/default/sysevent_bindings');

exports.subscribe = function (class, subclassList, callback) {
  var sub = new bindings.Subscription();
  sub.on('event', callback);
  sub.subscribe(class, subclassList);
  return sub;
}

exports.Subscription = bindings.Subscription;
