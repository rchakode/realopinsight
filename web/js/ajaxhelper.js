Cufon.now();
function startAutoPlay() {
  return setInterval(function() {
         jQuery('ul#myRoundabout').roundabout_animateToNextChild();
         }, 5000);
}

$(document).ready(function() {
  $('ul#myRoundabout').roundabout({
    btnNext: '.next',
    btnPrev: '.prev',
    startingChild: 1,
    duration: 800,
    easing: 'easeOutCubic',
    minOpacity: 1.0
  })
  .hover(
    function() {
            clearInterval(interval);
    },
    function() {
      interval = startAutoPlay();
    }
  );
 

  interval = startAutoPlay();
});

$.xmlrpc({
  url: 'http://labs.realopinsight.com/nucleus/xmlrpc/server.php',
  methodName: 'mt.getRecentPostTitles',
  params: ['1','ajax', 'A2Z3E4R5T6Y7U8I9O0P', 2],
  success: function(response, status, jqXHR) {
    var content = "";
    var count = response.length;
    for (i=0; i<count; i++) {
      content += '<li><a href="http://labs.realopinsight.com/index.php?itemid='
                              +response[i].postid+'" target="_blank">'+response[i].title+'</a></li>';
    }
    $("#blogupdates").html(content);
  },
  error: function(jqXHR, status, error) {
     console.log(error);
     console.log(jqXHR);
  }
});


var totaldlds = parseInt($("#sf_downloads").html()) + parseInt($('#obs-fedora').html()) + parseInt($('#obs-opensuse').html())+ parseInt($('#obs-debian').html());
$('#totaldlds').html("<b>" + totaldlds + '+</b> downloads from <b>'+ $('#sf_countries').html() + '+</b> countries!!!');

(function() {
  var po = document.createElement('script'); po.type = 'text/javascript'; po.async = true;
  po.src = 'https://apis.google.com/js/plusone.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(po, s);
})();

