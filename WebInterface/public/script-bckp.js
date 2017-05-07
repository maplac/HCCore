
var deviceList;
var clientId = -1;
var selectedDeviceId = -1;
var selectedDevice;
var ID_BROADCAST = 255;

var socket = io.connect();
var device_list;
var device_panel;
var selected_device_in_list = -1;
var graph;
var graph2;
var tempe;
var humi;

window.onload = function() {
	
	device_list=document.getElementById('device_list');
	device_panel=document.getElementById('device_panel');
	
	socket.on('onNewClientConnects', function (data) {
		clientId = data.id;
		console.log('this client id = ' + clientId);
		var msg = {};
		msg.desId = ID_BROADCAST;
		msg.srcId = clientId;
		msg.type = "pullAllDevices";
		socket.emit('message', msg);
	});
	graph = document.getElementById('graph');
	graph2 = document.getElementById('graph2');
	tempe = document.getElementById('device_temperature');
	humi = document.getElementById('device_humidity');
};

// when a user selects a device in the menu
function onClickDeviceSelect(button){
	
	// parse device id from button name
	var splitted = button.id.split('_');
	var id = parseInt(splitted[1]);
	selectedDeviceId = id;
	var device;
	
	// find the correct device in the device list
	for(i=0; i<deviceList.length; i++){
		if(deviceList[i].id === selectedDeviceId){
			device=deviceList[i];
			break;
		}
	}
	
	// remove selected layout from previous device
	if (typeof selected_device_in_list !== 'undefined') {
		selected_device_in_list.className= "device_in_list unselected";
	}
	
	// set selected layout to the current device
	selected_device_in_list=button;
	selected_device_in_list.className = "device_in_list selected";
	
	switch(device.type){
		case "switchOnOff" : 
			device_panel.innerHTML = DeviceSwitchOnOff.createDevice(device);
			break;
		// case "DHT220":
			
			// break;
		default : 
			device_panel.innerHTML = "not implemented";
	}
	
	
	// requests complete informations about the device
	// var msg = {};
	// msg.desId = id;
	// msg.srcId = clientId;
	// msg.type = "getDevice";
	// console.log("Send message:");
	// console.log(msg);
	// socket.emit('message', msg);
}

// when packet with message arrives
socket.on('message', function(msg){
	//console.log("Received message:");
	//console.log(msg);
	switch(msg.type){
		case "pushAllDevices":
			deviceList = msg.devices;
			var code = "";
			var d;
			for(i=0; i<deviceList.length; i++){
				d=deviceList[i];
				code=code+'<li class="device_in_list unselected" id="device_'+d.id+'" onclick="onClickDeviceSelect(this)">'+d.name+'</li>';
			}
			device_list.innerHTML = code;
			break;
		case "pushDevice":
			var device = msg.device;
			for(i=0; i<deviceList.length; i++){
				if (deviceList[i].id === device.id){
					deviceList[i] = device;
				}
				if (device.id === selectedDeviceId){
					switch(device.type){
						case "switchOnOff" : 
							device_panel.innerHTML = DeviceSwitchOnOff.createDevice(device);
							break;
						// case "DHT220":
							
							// break;
						default : 
							device_panel.innerHTML = "not implemented";
					}
				}
			}
			break;
		case "test":
			var trace1 = {	
				x: msg.time,
				y: msg.temperature,
				name: 'temperature',
				type: 'scatter',
				mode: 'lines+markers',
				line: {
					shape: 'spline',
					color: '#ff7f0e'
				},
			};
			var layout1 = {
				title: 'temperature [°C]',
				titlefont: {
					color: '#ff7f0e',
					size: 20
				},
				showlegend: false,
				paper_bgcolor:'rgba(0,0,0,0)',
				plot_bgcolor:'rgba(0,0,0,0)',
				domain: [0, 0.9],
				xaxis: {
					//title: 'time',
					showgrid: false,
					zeroline: false,
					//domain: [0, 0.7],
					tickfont: {
					  family: 'Arial',
					  size: 20,
					  color: 'rgb(78, 177, 186)'
					}
				},
				yaxis: {
					//title: 'temperature [°C]',
					//showline: false
					gridcolor: 'rgb(78, 177, 186)',
					zeroline: false,
					titlefont: {
						color: '#ff7f0e',
						size: 20
					},
					tickfont: {
						color: 'rgb(78, 177, 186)',//'#1f77b4',
						size: 20
					},
					//overlaying: 'y',
					//anchor: 'x',
					side: 'right'
				}
				
				
			};
			Plotly.newPlot( graph, [trace1], layout1 );
			var trace2 = {	
				x: msg.time,
				y: msg.humidity,
				name: 'humidity',
				type: 'scatter',
				mode: 'lines+markers',
				line: {
					shape: 'spline',
					color: '#ff7f0e'
				}
			};
			var data2 = [trace2];
			var layout2 = {
				title: 'humidity [%]',
				titlefont: {
					color: '#ff7f0e',
					size: 20
				},
				showlegend: false,
				paper_bgcolor:'rgba(0,0,0,0)',
				plot_bgcolor:'rgba(0,0,0,0)',
				domain: [0, 0.9],
				xaxis: {
					//title: 'time',
					showgrid: false,
					zeroline: false,
					
					tickfont: {
					  family: 'Arial',
					  size: 20,
					  color: 'rgb(78, 177, 186)'
					}
				},

				
				yaxis: {
					//title: 'humidity [%]',
					//showgrid: false,
					zeroline: false,
					gridcolor: 'rgb(78, 177, 186)',
					titlefont: {
						color: '#ff7f0e',
						size: 20
					},
					tickfont: {
						color: 'rgb(78, 177, 186)',
						size: 20
					},
					//overlaying: 'y',
					//anchor: 'free',
					//position: 0.85,
					side: 'right'
				}
				
			};
			Plotly.newPlot( graph2, data2, layout2 );
			tempe.textContent = msg.temperature[msg.temperature.length-1] + ' °C'
			humi.textContent = msg.humidity[msg.humidity.length-1]+ ' %'
			break;
		default:
		
	}
	/*
	switch(msg.type){
		case "getAllDevices" :
			deviceList = msg.devices;
			var code = "";
			var d;
			for(i=0; i<deviceList.length; i++){
				d=deviceList[i];
				code=code+'<li class="device_in_list unselected" id="device_'+d.id+'" onclick="onClickDeviceSelect(this)">'+d.name+'</li>';
			}
			device_list.innerHTML = code;
			//onClickDeviceSelect(device_list.firstElementChild);
			break;
		case "getDevice" :
			if(msg.device.id !== selectedDeviceId){
				console.log("Device id doesn't match. Message ignored.")
				return;
			}
			selectedDevice=msg.device;
			device_panel_body.innerHTML = createDevicePanelBody(msg.device);
			break;
		case "changeParameters" :

			updateDevice(msg);
		default:
	}
	*/
	
});


function updateDevice(msg){
	
	var targetDevice;
	for(i=0; i<deviceList.length; i++){
		if(deviceList[i].id === msg.srcId){
			targetDevice=deviceList[i];
			break;
		}
	}
	
	// TODO if necessary update gui device list
	
	if (msg.srcId !== selectedDeviceId){
		console.log("Update is not for selected device");
		//return;
	}
	
	// console.log(Object.getOwnPropertyNames(selectedDevice))
	switch(selectedDevice.type){
		case "switchOnOff" : 
			DeviceSwitchOnOff.updateParameters(msg.parameters);
			break;
		default : 
		
	}
	
	// if(switchOn1 === 1){
		// switchOn1 = 0;
		// document.getElementById('button_on_off').setAttribute("class", "button_onoff on");;
	// }else{
		// switchOn1 = 1;
		// document.getElementById('button_on_off').setAttribute("class", "button_onoff off");;
	// }
}

// function createDevicePanelHead(device){
	
// }

function createDevicePanelBody(device){
	var code = '';
	switch(device.type){
		case "switchOnOff" : 
			code += DeviceSwitchOnOff.createDevice(device);
			break;
		case "server" :
			break;
		case "DHT22" :
			break;
		default:
			code = code + '<p>unknown device type</p>'
	}
	return code;
}



function createDeviceDHT22(device){
	var code = "";
	code += '<span>Status: </span><span id="device_panel_status">'+device.status+'</span><br>';
	code += '<span>Last connected: </span><span id="device_panel_last_connected">'+device.lastConnected+'</span><br>';
	return code;
}

var switchOn1 = 0;


function onClick(){
	
}



