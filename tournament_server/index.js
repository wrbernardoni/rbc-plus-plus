var express  = require('express')
var app = express()
var http = require('http').createServer(app);
var request = require('request');

var bodyParser = require('body-parser');
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

const fs = require('fs');

const K = 100;

require('console-stamp')(console, '[HH:MM:ss.l]');

if (!fs.existsSync("./elo.json"))
{
	let newElo = { };
	fs.writeFileSync('./elo.json', JSON.stringify(newElo));
}

if (!fs.existsSync("./winloss.json"))
{
	let newElo = { };
	fs.writeFileSync('./winloss.json', JSON.stringify(newElo));
}

if (!fs.existsSync("./webhooks.json"))
{
	let newElo = { };
	fs.writeFileSync('./webhooks.json', JSON.stringify(newElo));
}

let rawdata = fs.readFileSync('./elo.json');
let elo = JSON.parse(rawdata);

let webhooks = JSON.parse(fs.readFileSync('./webhooks.json'));
let winloss = JSON.parse(fs.readFileSync('./winloss.json'));

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
	  border: 1px solid #2e2626;
	  background-color:#dde3e9;
	}
	table.center {
	  margin-left: auto;
	  margin-right: auto;
	}
	</style>
	<meta http-equiv="refresh" content="60" >
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

	ret += "</table> <br><br>"

	ret += "<div style=\"overflow:auto\">"
	ret += "<table class=\"center\"> <tr> <th> Bot </th> <th> W-L </th> "
	for (let i = 0; i < eloArr.length; i++)
	{
		ret += "<th>" + eloArr[i][0] + "</th>";
	}
	ret += "</tr>"
	for (let i = 0; i < eloArr.length; i++)
	{
		let wT = 0
		let lT = 0
		for (let j = 0; j < eloArr.length; j++)
		{
			if (winloss[eloArr[i][0]])
			{
				if (winloss[eloArr[i][0]][eloArr[j][0]])
				{
					let w = winloss[eloArr[i][0]][eloArr[j][0]].wins
					let l = winloss[eloArr[i][0]][eloArr[j][0]].losses
					wT += w;
					lT += l;
				}
			}
		}

		ret += "<tr>"
		ret += "<th>" + eloArr[i][0] + "</th>";

		if ((wT != 0) || (lT != 0))
			ret += `<td>${wT}-${lT} (${Math.round(100.0*wT/(wT + lT))}%)</td>`
		else
			ret += "<th></th>"
		for (let j = 0; j < eloArr.length; j++)
		{
			let w = null;
			let l = null;
			
			if (winloss[eloArr[i][0]])
			{
				if (winloss[eloArr[i][0]][eloArr[j][0]])
				{
					w = winloss[eloArr[i][0]][eloArr[j][0]].wins
					l = winloss[eloArr[i][0]][eloArr[j][0]].losses
				}
			}


			if (w || l)
			{
				if (!w)
					w = 0;
				if (!l)
					l = 0;

				if (w > l)
					ret += "<td style=\"background-color:#c5e2c0;text-align: center\">"
				else if (w < l)
					ret += "<td style=\"background-color:#e2c0c0;text-align: center\">"
				else
					ret += "<td style=\"text-align: center\">"

				ret += `${ w }-${ l }`
			}
			else
			{
				ret += "<td style=\"background-color:#a79e9e\">"
			}
			ret += "</td>"
		}
		ret += "</tr>"
	}
	ret += "</table></div>";

	ret += "</body>"
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

		if (!winloss[winner])
		{
			winloss[winner] = {}
		}

		if (!winloss[loser])
		{
			winloss[loser] = {}
		}

		if (!winloss[winner][loser])
		{
			winloss[winner][loser] = {
				wins : 0,
				losses : 0
			}
		}

		if (!winloss[loser][winner])
		{
			winloss[loser][winner] = {
				wins : 0,
				losses : 0
			}
		}

		winloss[winner][loser].wins += 1;
		winloss[loser][winner].losses += 1;

		let pWin = (1.0 / (1.0 + Math.pow(10, ((elo[loser] - elo[winner])/400))));
		let delta = K * (1.0 - pWin);
		bot1Elo1 = (Math.round(elo[bot1] * 100) / 100).toFixed(2)
		bot2Elo1 = (Math.round(elo[bot2] * 100) / 100).toFixed(2)
		elo[winner] += delta;
		elo[loser] -= delta;
		bot1Elo2 = (Math.round(elo[bot1] * 100) / 100).toFixed(2)
		bot2Elo2 = (Math.round(elo[bot2] * 100) / 100).toFixed(2)

		console.log(`W:${bot1}(${bot1Elo1}->${bot1Elo2}) vs B:${bot2}(${bot2Elo1}->${bot2Elo2}), winner: ${winner}, delta:${(Math.round(delta * 100) / 100).toFixed(2)}`);
		//console.log();

		if (webhooks.DiscordWebhook)
		{
			let c = 	16777215;
			if (winner == bot2)
				c = 0
			let reqSend = {
					embeds : [
						{
							fields : [
							{
								name : "White",
								value : `${bot1} | (${bot1Elo1})`,
								inline : true
							},
							{
								name : "Black",
								value : `${bot2} | (${bot2Elo1})`,
								inline : true
							},
							{
								name : `Winner (delta: ${(Math.round(delta * 100) / 100).toFixed(2)})`,
								value: `${winner}`
							},
							{
								name : `*${bot1}*`,
								value : `${bot1Elo2}`,
								inline : true
							},
							{
								name : `*${bot2}*`,
								value : `${bot2Elo2}`,
								inline : true
							},
							],
							color : c
						}
					]
			}
			request.post(
			{
			    url: webhooks.DiscordWebhook,
			    body: JSON.stringify(reqSend),
			    headers: {
			        'Content-Type': 'application/json'
			    }
			})
		}
	}	

	fs.writeFileSync('./elo.json', JSON.stringify(elo));
	fs.writeFileSync('./winloss.json', JSON.stringify(winloss));
	res.send();
})
