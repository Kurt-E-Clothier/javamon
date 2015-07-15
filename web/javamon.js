(function(){
	  
	// Constants for the variable coffee height attribute
	const DEFAULT_HEIGHT_PCT = "95%";
	const MAX_LEVEL = 100;
	const HEIGHT_CHANGE = 5;
	const MIN_HEIGHT = 0;
	const CRITICAL_HEIGHT = 20;
	
	// Constants for PubNub ms
	const FILLING = 0;
	const READY = 1;

	const CHANNEL = 'javamon';
	const ONLINE_ICON = "images/coffee-online.png";
	const OFFLINE_ICON = "images/coffee-offline.png";

	// Variables
	var pubnub = PUBNUB.init({
		//publish_key: 'pub-c-6646d310-d65b-4c41-ae2c-760476c77524',
	    //subscribe_key: 'sub-c-84d9d286-08bd-11e5-9ffb-0619f8945a4f'
		publish_key: 'demo',
		subscribe_key: 'demo'
	});
	var output = pubnub.$('chat');
	var level = DEFAULT_HEIGHT_PCT;
	var value = 0;
	var icon = document.getElementById("icon");
	var stateTxt = document.getElementById("state");
	var levelTxt = document.getElementById("level");
	var fillState = FILLING;	// is the pot filling or ready?

	function print(s){
		output.innerHTML = output.innerHTML + '<br>' + s;
		output.scrollTop = output.scrollHeight;
	}

	// Subscribe to channel
	pubnub.subscribe({
		'channel'  : CHANNEL,
		// Handle messages to the channel
		'callback' : function(m) {
			// Turn back into pixel value and update display
			print(m);
			value = parseInt(m);
			if(value == MAX_LEVEL){
				levelTxt.innerHTML = "Full!";
			}
			else if(value > CRITICAL_HEIGHT){
				levelTxt.innerHTML = "Level: " + value.toString() + '%';
			}
			else {
				levelTxt.innerHTML = "Critically low!";
			}
			level = (100-value) + "%";
			document.getElementById('coffee').style.height = level;
		},
		// Handle presence events
		'presence' : function(m) {
			if(m['uuid'] == 'JavaMonPi'){
				if(m['action'] == 'join'){
					icon.src = ONLINE_ICON;
					stateTxt.innerHTML = "Active Monitoring";
				}
				else if(m['action'] == 'leave' || m['action'] == 'timeout'){
					icon.src = OFFLINE_ICON;
					stateTxt.innerHTML = "Offline";
					levelTxt.innerHTML = "";
					document.getElementById('coffee').style.height = "5px";
				}
			}
			else {
				print(m['uuid'] + ': ' + m['action']);
			}			
		}
	});
})();







// JavaScript Document

$(document).ready(function(){
	//smooth scroll
	$('a[href*=#]:not([href=#])').click(function() {
		if (location.pathname.replace(/^\//,'') == this.pathname.replace(/^\//,'') && location.hostname == this.hostname) {
			var target = $(this.hash);
			target = target.length ? target : $('[name=' + this.hash.slice(1) +']');
		if (target.length) {
			$('html,body').animate({
			scrollTop: target.offset().top
		}, 1000);
		return false;
		}
		}
	});
});


$(window).resize(function(){ 
	var nav = $( "nav ul" ); 
    var w = $(window).width();  
    if(w > 700 && nav.is(':hidden')) {  
        nav.show(); 
    }  else if (w < 700) {
    	$(".logo img").hide();
    }
}); 

$( "#mobilemenu" ).click(function() {
	$( "#mobilemenu" ).toggleClass( "rotatedmenu" )
  $( "nav ul" ).slideToggle( function() {
    // Animation complete.
  });

});

$( "nav ul li a" ).click(function() {
	$( "#mobilemenu" ).toggleClass( "rotatedmenu" )
	var w = $(window).width();
    if(w < 700) {
	  $( "nav ul" ).slideToggle( "slow", function() {
	    // Animation complete.
	  });
    } else {
        return false;
    }
});

$( ".mobilelogo" ).click(function() {
	$( "#mobilemenu" ).toggleClass( "rotatedmenu" )
	var w = $(window).width();
    if(w < 700) {
	  $( "nav ul" ).slideToggle( "slow", function() {
	    // Animation complete.
	  });
    } else {
        return false;
    }
});

$( ".middlelogo" ).click(function() {
	$( "#mobilemenu" ).toggleClass( "rotatedmenu" )
	var w = $(window).width();
    if(w < 700) {
	  $( "nav ul" ).slideToggle( "slow", function() {
	    // Animation complete.
	  });
    } else {
        return false;
    }
});

$(window).scroll(function() {
	var w = $(window).width();
    if(w > 700) {
		  if ($(this).scrollTop() > 100) {
		    $('nav').css("position", "fixed");
        $('nav ul li').addClass("scrolled");
		  }
		  else {
		    $('nav').css("position", "relative");
        $('nav ul li').removeClass("scrolled");
		  }
	} else {
        return false;
    }
});
