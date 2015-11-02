$('#id-on').click(function(){
    console.log("On");
    $.get('/main/on/', function(data) {
	console.log(data);
    });
});

$('#id-off').click(function(){
    console.log("Off");
    $.get('/main/off/', function(data) {
	console.log(data);
    });
});

$('#id-automatic').click(function(){
    console.log("Automatic");
    $.get('/main/automatic/', function(data) {
	console.log(data);
    });
});


function updateLabels() {
    $.get('/main/status/', function(data) {
	console.log(data);
	$('#id-time').html(data.time)
	$('#id-condition').html(data.mode)
    });
}

setInterval(updateLabels, 5000);
