var express  = require('express')
var app = express()
var http = require('http').createServer(app);

var bodyParser = require('body-parser');
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

const fs = require('fs');

const K = 100;

if (!fs.existsSync("./elo.json"))
{
	let newElo = { };
	fs.writeFileSync('./elo.json', JSON.stringify(newElo));
}

let rawdata = fs.readFileSync('./elo.json');
let elo = JSON.parse(rawdata);

http.listen(3000, function() {
	console.log("Server booted and listening on :3000")
})

function compare( a, b ) {
  if ( a[1] > b[1] ){
    return -1;
  }
  if ( a[1] < b[1] ){
    return 1;
  }
  return 0;
}

app.get('/', function(req, res) {
	let eloArr = []
	for (key in elo)
	{
		eloArr.push([key, elo[key]]);
	}

	eloArr.sort(compare);

	let ret = `
	<html>
	<head>
	<style>
	table, th, td {
	  border: 1px solid black;
	}
	table.center {
	  margin-left: auto;
	  margin-right: auto;
	}
	</style>
	<meta http-equiv="refresh" content="30" >
	</head>
	<body>
	<table class="center">
	  <tr>
	    <th>Bot Name</th>
	    <th>Elo</th>
	  </tr>
	`;
	for (let i = 0; i < eloArr.length; i++)
	{
		ret += "<tr> <td>" + eloArr[i][0] + "</td><td>" + (Math.round(eloArr[i][1] * 100) / 100).toFixed(2); + "</td></tr>";
	}

	ret += "</table> </body>"
	res.send(ret);
})

app.post("/", function(req, res)
{
	let bot1 = req.body.white;
	let bot2 = req.body.black;
	let winner = req.body.winner;
	if (!elo[bot1])
	{
		elo[bot1] = 1000;
	}

	if (!elo[bot2])
	{
		elo[bot2] = 1000;
	}

	if (winner != "")
	{
		let loser = bot1;
		if (winner == bot1)
		{
			loser = bot2;
		}

		let pWin = (1.0 / (1.0 + Math.pow(10, ((elo[winner] - elo[loser])/400))));
		console.log(`W:${bot1}(${(Math.round(elo[bot1] * 100) / 100).toFixed(2)}) vs B:${bot2}(${(Math.round(elo[bot2] * 100) / 100).toFixed(2)}), winner: ${winner}`);
		elo[winner] += K * (1.0 - pWin);
		elo[loser] -= K * (1.0 - pWin);
		console.log(`W:${bot1}(${(Math.round(elo[bot1] * 100) / 100).toFixed(2)}) vs B:${bot2}(${(Math.round(elo[bot2] * 100) / 100).toFixed(2)}), new elo`);
		console.log();
	}

	fs.writeFileSync('./elo.json', JSON.stringify(elo));
	res.send();
})
