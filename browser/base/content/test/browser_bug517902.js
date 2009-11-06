/* Make sure that "View Image Info" loads the correct image data */

function test() {
  waitForExplicitFinish();

  gBrowser.selectedTab = gBrowser.addTab();
  
  gBrowser.selectedBrowser.addEventListener("load", function () {
    gBrowser.selectedBrowser.removeEventListener("load", arguments.callee, true);

    var doc = gBrowser.contentDocument;
    var testImg = doc.getElementById("test-image");
    var pageInfo = BrowserPageInfo(doc, "mediaTab", testImg);

    pageInfo.addEventListener("load", function () {
      pageInfo.onFinished.push(function () {
        executeSoon(function () {
          var pageInfoImg = pageInfo.document.getElementById("thepreviewimage");

          is(pageInfoImg.src, testImg.src, "selected image has the correct source");
          is(pageInfoImg.width, testImg.width, "selected image has the correct width");
          is(pageInfoImg.height, testImg.height, "selected image has the correct height");

          pageInfo.close();
          gBrowser.removeCurrentTab();
          finish();
        });
      });
    }, true);
  }, true);

  content.location =
    "data:text/html," +
    "<img src='about:logo?a' height=200 width=250>" +
    "<img src='about:logo?b' height=200 width=250 alt=1>" +
    "<img src='about:logo?b' height=100 width=150 alt=2 id='test-image'>";
}
