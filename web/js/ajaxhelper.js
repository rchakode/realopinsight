// For slide show
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
      // oh no, it's the cops!
      clearInterval(interval);
    },
    function() {
    // false alarm: PARTY!
      interval = startAutoPlay();
    }
  );
 
  // let's get this party started
  interval = startAutoPlay();
});

// For Blogs
$.xmlrpc({
  url: 'http://blog.realopinsight.com/nucleus/xmlrpc/server.php',
  methodName: 'mt.getRecentPostTitles',
  params: ['1','ajax', 'A2Z3E4R5T6Y7U8I9O0P', 2],
  success: function(response, status, jqXHR) {
    var content = "";
    var count = response.length;
    for (i=0; i<count; i++) {
      content += '<li><a href="http://blog.realopinsight.com/index.php?itemid='
                              +response[i].postid+'" target="_blank">'+response[i].title+'</a></li>';
    }
    $("#blogupdates").html(content);
  },
  error: function(jqXHR, status, error) {
     console.log(error);
     console.log(jqXHR);
  }
});

// For stats
$.getJSON('sfstats/data/total.json',
  function(data) {
	var totaldlds = parseInt(data.total) + parseInt($('#obs-fedora').html()) + parseInt($('#obs-opensuse').html());
	$('#totaldlds').html("<b>" + totaldlds + '+</b> downloads from <b>'+ data.countries.length + '+</b> countries!!!');
    	$('#start_date').html(data.start_date.substr(0, 10));
    	$('#end_date').html(data.stats_updated);
  });

// For G+ button
(function() {
  var po = document.createElement('script'); po.type = 'text/javascript'; po.async = true;
  po.src = 'https://apis.google.com/js/plusone.js';
  var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(po, s);
})();

