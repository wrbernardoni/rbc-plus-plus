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

let changed = true;

function compare( a, b ) {
  if ( a[1] > b[1] ){
    return -1;
  }
  if ( a[1] < b[1] ){
    return 1;
  }
  return 0;
}

function sendRankings()
{
	if (!changed)
		return;

	changed = false;
	console.log("Sending rankings")
	let eloArr = []
	for (key in elo)
	{
		eloArr.push([key, elo[key]]);
	}

	eloArr.sort(compare);

	let reqSend = {}

	let content = "";

	for (let i = 0; i < eloArr.length; i++)
	{
		content += `${i+1}: ${eloArr[i][0]} (${(Math.round(eloArr[i][1] * 100) / 100).toFixed(2)})\n`
	}

	reqSend.content = content;

	request.post(
	{
	    url: webhooks.DiscordWebhook,
	    body: JSON.stringify(reqSend),
	    headers: {
	        'Content-Type': 'application/json'
	    }
	})
}

http.listen(3000, function() {
	function getIPAddress() { // Function from https://stackoverflow.com/a/15075395
	  var interfaces = require('os').networkInterfaces();
	  for (var devName in interfaces) {
	    var iface = interfaces[devName];

	    for (var i = 0; i < iface.length; i++) {
	      var alias = iface[i];
	      if (alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal)
	        return alias.address;
	    }
	  }
	  return '0.0.0.0';
	}

	let addr = getIPAddress();

	console.log(`Server booted and listening on ${addr}:3000`)
	if (webhooks.DiscordWebhook)
		setInterval(sendRankings,15*60*1000);
})


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
	ret += "<table class=\"center\"> <tr> <th> Bot </th> <th> W-D-L </th> "
	for (let i = 0; i < eloArr.length; i++)
	{
		ret += "<th>" + eloArr[i][0] + "</th>";
	}
	ret += "</tr>"
	for (let i = 0; i < eloArr.length; i++)
	{
		let wT = 0
		let lT = 0
		let dT = 0;
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
					dT += winloss[eloArr[i][0]][eloArr[j][0]].draws
				}
			}
		}

		ret += "<tr>"
		ret += "<th>" + eloArr[i][0] + "</th>";

		if ((wT != 0) || (lT != 0))
			ret += `<td>${wT}-${dT}-${lT} (${Math.round(100.0*wT/(wT + lT))}%)</td>`
		else
			ret += "<th></th>"
		for (let j = 0; j < eloArr.length; j++)
		{
			let w = null;
			let l = null;
			let d = null;
			
			if (winloss[eloArr[i][0]])
			{
				if (winloss[eloArr[i][0]][eloArr[j][0]])
				{
					w = winloss[eloArr[i][0]][eloArr[j][0]].wins
					l = winloss[eloArr[i][0]][eloArr[j][0]].losses
					d = winloss[eloArr[i][0]][eloArr[j][0]].draws
				}
			}


			if (w || l || d)
			{
				if (!w)
					w = 0;
				if (!l)
					l = 0;
				if (!d)
					d = 0;

				if ((w != 0) || (l != 0))
				{
					let wr = w/(w + l);
					let r = 0;
					let g = 0;
					let b = 0;

					if (wr < 0.5)
					{
						wr = 2 * wr;
						r = (1.0 - wr) * 221 + wr * 216
						g = (1.0 - wr) * 130 + wr * 216
						b = (1.0 - wr) * 130 + wr * 216
					}
					else
					{
						wr = 2 * (wr - 0.5)
						r = (1.0 - wr) * 216 + wr * 130
						g = (1.0 - wr) * 216 + wr * 221
						b = (1.0 - wr) * 216 + wr * 130
					}
					ret += `<td style=\"background-color:rgb(${r},${g},${b});text-align: center\">`
				}
				else
					ret += "<td style=\"text-align: center\">"

				ret += `${ w }-${d}-${ l }`
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
				losses : 0,
				draws : 0
			}
		}

		if (!winloss[loser][winner])
		{
			winloss[loser][winner] = {
				wins : 0,
				losses : 0,
				draws : 0
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
	else
	{
		if (!winloss[bot1][bot2])
		{
			winloss[bot1][bot2] = {
				wins : 0,
				losses : 0,
				draws : 0
			}
		}

		if (!winloss[bot2][bot1])
		{
			winloss[bot2][bot1] = {
				wins : 0,
				losses : 0,
				draws : 0
			}
		}

		winloss[bot1][bot2].draws += 1;
		winloss[bot2][bot1].draws += 1;
		console.log(`W:${bot1}(${bot1Elo1}->${bot1Elo2}) vs B:${bot2}(${bot2Elo1}->${bot2Elo2}), DRAW`);
	}

	fs.writeFileSync('./elo.json', JSON.stringify(elo));
	fs.writeFileSync('./winloss.json', JSON.stringify(winloss));
	res.send();
	changed = true;
})
