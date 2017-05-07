
/* Required packages
├── express@4.14.0
├── plotly.js@1.21.3
├── socket.io@1.4.8
└── zmq@2.15.3
*/

var DEVICE_ID = 200;
var BROADCAST_ID = 255;

express = require('express');  //web server
app = express();
server = require('http').createServer(app);
io = require('socket.io').listen(server);	//web socket server
zmq = require("zmq");  
socketZmqIn = zmq.socket("sub"); 
socketZmqOut = zmq.socket("pub"); 

server.listen(80); //start the webserver
app.use(express.static('public')); //tell the server that ./public/ contains the static webpages

socketZmqIn.connect('tcp://127.0.0.1:20001',function(err) {
	if (err) {
		console.log(err);
		return;
	}
});
socketZmqIn.subscribe('');
// bindSync blocks until it binds
socketZmqOut.bindSync('tcp://*:20000',function(err){
	if (err) {
		console.log(err);
		return;
	}
});
/*
setTimeout(function() {
	var msg = {};
	msg.desId = BROADCAST_ID;
	msg.srcId = DEVICE_ID;
	msg.type = "getAllDevices";
	socketZmqOut.send(JSON.stringify(msg));
}, 1000);
*/
function ConnectedClient(socket, id){
	this.id = id;
	this.socket = socket;
	// this.activeDevice = -1;
}

var clientList = [];

io.sockets.on('connect', function(client) {
	var clientId = getFreeId();
	console.log("pripojeni"); 
	//console.log("pred: "+clientsToString(clientList));
	clientList.push(new ConnectedClient(client,clientId));
	console.log("po: "+clientsToString(clientList));	
	
	client.emit('onNewClientConnects', {id: clientId});
	
    client.on('disconnect', function() {
		console.log("odpojeni");
		//console.log("pred: "+clientsToString(clientList));
		clientList.splice(getIndexInClientList(client), 1);
		console.log("po: "+clientsToString(clientList));
    });
	
	client.on('message', function(msg){
		console.log("ws in: "+JSON.stringify(msg));
		socketZmqOut.send(JSON.stringify(msg));
	});
});

function getClientInClientList(client){
	for(var i=0; i < clientList.length; i++){
		if(clientList[i].socket === client){
			return clientList[i];
		}
	}
}
function getIndexInClientList(client){
	for(var i=0; i < clientList.length; i++){
		if(clientList[i].socket === client){
			return i;
		}
	}
	return -1;
}

function getFreeId(){
	for(var i=1000; i < 9000; i++){
		var isFree = 1;
		for(var j=0; j < clientList.length; j++){
			if(clientList[j].id===i){
				isFree=0;
				break;
			}
		}
		if(isFree===1){
			return i;
		}
	}
	return 9000;
}

function clientsToString(c){
	var str = "";
	for(var i=0; i < c.length; i++){
		if(c[i]===null){
			str = str + "0" + ", ";
		}else{
			str = str + c[i].id + ", ";
		}
	}
	return str;
}


socketZmqIn.on("message", function (msgRaw) {  
	var msg = JSON.parse(msgRaw);
	var desId = parseInt(msg.desId);
	console.log("zmq in: "+JSON.stringify(msg));
	
	// if the message is broadcast send it to all client
	if(msg.desId == BROADCAST_ID){
		for(var j=0; j < clientList.length; j++){
			clientList[j].socket.emit("message", msg);
		}
	}else{
		for(var j=0; j < clientList.length; j++){
			if(clientList[j].id === desId){
				clientList[j].socket.emit("message", msg);
				break;
			}
		}
	}
});


function exit() {
	// free all resources
	console.log(" Exit");
	socketZmqIn.close();
	socketZmqOut.close();
	process.exit();
}
process.on('SIGINT', exit);


