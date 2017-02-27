var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

connection.onopen = function () {  connection.send('Connect ' + new Date()); }; 

connection.onerror = function (error) {    console.log('WebSocket Error ', error);};

connection.onmessage = function (e) {  console.log('Server: ', e.data); recieveState(e.data); };

function sendAnimation() {
	var animation = 'anim:';
	animation += parseInt($('animation').value);
	connection.send(animation);
}

function sendSpeed() {
	var speed = parseInt($('speed').value);
	$('speeddisp').innerHTML = speed;
	console.log("speed:" + speed);
	$('speeddisp').innerHTML = speed;
	connection.send("speed:" + speed);
}

function sendColor() {
	var color = $('color').value;
	console.log(color);
	console.log("color:" + color);
	connection.send("color:" + color);	
}

function recieveState(data) {
	if(data.indexOf('STATE:') >= 0) {
		
		var r = getValFor('r:', data);
		var g = getValFor('g:', data);
		var b = getValFor('b:', data);
		
		$('color').value = rgbToHex(r, g, b);

		var speed = getValFor('speed:', data);
		$('speed').value = speed;
		$('speeddisp').innnerHTML = speed;

		var anim = getValFor('anim:', data);
		$('animation').value = anim;
		renderAnimationOptions(getValFor('anims:', data).split(','), anim);
		// if($('animation option:selected').text() == "ColorFill") {
		// 	$('colorselection').removeClass('hidden');
		// }
		// else{
		// 	$('colorselection').addClass('hidden');;
		// }
	}
}


function renderAnimationOptions(options, current) {
	var opts = "";
	for(var i in options) {
		var sel = "";
		if(i == current) {
			sel = " selected=\"selected\"";
		}
		opts += "<option value=\"" + i + "\" " + sel + ">" + options[i] + "</option>";
	}
	$('animation').innerHTML = opts;
}

function getValFor(prop, str) {
	var idx = str.indexOf(prop);
	if(idx >= 0) {		
		var end = str.indexOf(';', idx);
		return str.substring(idx+prop.length, end);
	}
	return false;
}

function componentToHex(c) {
	console.log(c);
	c = parseInt(c);
    var hex = c.toString(16);
    console.log(hex);
    return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(r, g, b) {
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function $(id) {
	return document.getElementById(id);
}