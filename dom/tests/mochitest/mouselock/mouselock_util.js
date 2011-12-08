function ok() {
  opener.ok.apply(opener, arguments);
  //opener.ok(condition, msg);
}

function is(a, b, msg) {
  opener.is(a, b, msg);
}

function isnot(a, b, msg) {
  opener.isnot(a, b, msg);
}