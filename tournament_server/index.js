var express  = require('express')
var app = express()
var http = require('http').createServer(app);

var bodyParser = require('body-parser');
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

http.listen(3000, function() {
	console.log("Server booted and listening on :3000")
})

app.get('/', function(req, res) {
	res.send("Hello world!");
})
