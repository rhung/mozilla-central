function ok() {
  opener.ok.apply(opener, arguments);
}

function is(a, b, msg) {
  opener.is.apply(opener, arguments);
}

function isnot(a, b, msg) {
  opener.is.apply(opener, arguments);
}

SimpleTest.finish = function () {
  opener.nextTest();
};

SimpleTest.waitForExplicitFinish = function() {};