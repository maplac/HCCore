"use strict";
function DeviceBME280(id) {
	DeviceGeneric.call(this, id, 'BME280');
	this.temperature = 0;
	this.pressure = 0;
	this.humidity = 0;
	this.voltage = 0;
	this.data = {
		temperature: [],
		pressure: [],
		humidity: [],
		time: []
	};
}
DeviceBME280.prototype = Object.create(DeviceGeneric.prototype);
DeviceBME280.prototype.constructor = DeviceBME280;

DeviceBME280.prototype.getHtmlDeviceInfo = function() {
	var code = '';
	code += DeviceGeneric.prototype.getHtmlDeviceInfo.call(this);
	code += '<br>';
	code += '<span id="device_panel_temperature" class="big_text">'+Math.round(this.temperature*100)/100 + ' &degC'+'</span>';
	code += '<span id="device_panel_humidity" class="big_text" style="padding-left: 50px">'+Math.round(this.humidity*10)/10 + ' %'+'</span>';
	code += '<span id="device_panel_pressure" class="big_text" style="padding-left: 50px">'+Math.round(this.pressure) + ' Pa'+'</span>';
	code += '<span id="device_panel_voltage" class="big_text" style="padding-left: 50px">'+Math.round(this.voltage*100)/100 + ' V'+'</span><br><br>';
	code += '<span><button id="button_day" class="button_switch button_switch_deselected" onclick="selectedDevice.onButtonPress(\'day\')"> One day</button> \
	<button id="button_week" class="button_switch button_switch_deselected" onclick="selectedDevice.onButtonPress(\'week\')">One week</button>\
	<button id="button_month" class="button_switch button_switch_deselected" onclick="selectedDevice.onButtonPress(\'month\')">One month</button>\
	<button id="button_year" class="button_switch button_switch_deselected" onclick="selectedDevice.onButtonPress(\'year\')">One year</button></span></br>';
	code += '<div id="graph_temperature" style="width:100%;height:300px;margin-top:10px"></div>';
	code += '<div id="graph_humidity" style="width:100%;height:300px;margin-top:10px"></div>';
	code += '<div id="graph_pressure" style="width:100%;height:300px;margin-top:10px"></div>';
	return code;
};

DeviceBME280.prototype.createDevicePanel = function(msg) {
	document.getElementById('device_panel').innerHTML = this.getHtmlDeviceInfo();
};

DeviceBME280.prototype.newDataReceived = function(msg) {
	this.temperature = msg.data.temperature;
	this.pressure = msg.data.pressure;
	this.humidity = msg.data.humidity;
	this.voltage = msg.data.voltage;
	this.lastConnected = msg.lastConnected;
	document.getElementById('device_panel_temperature').innerHTML = Math.round(this.temperature*100)/100 + " &degC";
	document.getElementById('device_panel_humidity').innerHTML = Math.round(this.humidity*10)/10 + " %";
	document.getElementById('device_panel_pressure').innerHTML = Math.round(this.pressure) + " Pa";
	document.getElementById('device_panel_voltage').innerHTML = Math.round(this.voltage*100)/100 + " V";
	document.getElementById('device_panel_last_connected').innerHTML = msg.lastConnected;
};

DeviceBME280.prototype.onButtonPress = function(type) {
	console.log("pressed: " + type);
	var msg = {};
	msg.desId = this.id;
	msg.srcId = clientId;
	msg.type = "pullDataBuffer";

	switch(type){
		case "day":
			document.getElementById('button_day').className = "button_switch button_switch_selected";
			document.getElementById('button_week').className = "button_switch button_switch_deselected";
			document.getElementById('button_month').className = "button_switch button_switch_deselected";
			document.getElementById('button_year').className = "button_switch button_switch_deselected";
			msg.subType = "day";
			break;
		case "week":
			document.getElementById('button_day').className = "button_switch button_switch_deselected";
			document.getElementById('button_week').className = "button_switch button_switch_selected";
			document.getElementById('button_month').className = "button_switch button_switch_deselected";
			document.getElementById('button_year').className = "button_switch button_switch_deselected";
			msg.subType = "week";
			break;
		case "month":
			document.getElementById('button_day').className = "button_switch button_switch_deselected";
			document.getElementById('button_week').className = "button_switch button_switch_deselected";
			document.getElementById('button_month').className = "button_switch button_switch_selected";
			document.getElementById('button_year').className = "button_switch button_switch_deselected";
			msg.subType = "month";
			break;
		case "year":
			document.getElementById('button_day').className = "button_switch button_switch_deselected";
			document.getElementById('button_week').className = "button_switch button_switch_deselected";
			document.getElementById('button_month').className = "button_switch button_switch_deselected";
			document.getElementById('button_year').className = "button_switch button_switch_selected";
			msg.subType = "year";
			break;
		default:
	}
	socket.emit('message', msg);
};

DeviceBME280.prototype.dataBufferReceived = function(msg) {
	this.data = msg.data;
	for(var i = 0; i < this.data.time.length; i++){
		this.data.temperature[i] = this.data.temperature[i] / 100;
		//this.data.pressure[i] = this.data.pressure[i];
		this.data.humidity[i] = this.data.humidity[i]/100;
	}
	// console.log(this.data.temperature);
	this.updateGraphs();
};

DeviceBME280.prototype.setParameters = function(device) {
	DeviceGeneric.prototype.setParameters.call(this, device);
	this.temperature = device.temperature;
	this.pressure = device.pressure;
	this.humidity = device.humidity;
	this.voltage = device.voltage;
};

DeviceBME280.prototype.updateGraphs = function() {
	var max = Math.max(...this.data.temperature);
	var min = Math.min(...this.data.temperature);
	var mean = (max + min) / 2;
	var deviation = mean / 20;
	var yRange = [mean-deviation, mean+deviation];
	if(min < yRange[0]){ yRange[0] = min*0.95;}
	if(max > yRange[1]){ yRange[1] = max*1.05;}
	// console.log(min+" "+max+" "+mean+" "+deviation);
	var trace = {	
		x: this.data.time,
		y: this.data.temperature,
		// name: 'temperature',
		type: 'scatter',
		mode: 'lines+markers',
		line: {
			color: '#ff7f0e'
		},
	};
	var layout = {
		/*title: 'temperature [°C]',
		titlefont: {
			color: '#ff7f0e',
			size: 20
		},
		*/
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
			title: 'Temperature [°C]',
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
			side: 'right',
			range: yRange
		},
		autosize: true,
		margin: {
			autoexpand: false,
			l: 50,
			r: 100,
			t: 10,
			b: 60,
		}
		
	};
	Plotly.newPlot( document.getElementById('graph_temperature'), [trace], layout );
	
	max = Math.max(...this.data.humidity);
	min = Math.min(...this.data.humidity);
	mean = (max + min) / 2;
	deviation = mean / 20;
	yRange = [mean-deviation, mean+deviation];
	if(min < yRange[0]){ yRange[0] = min*0.95;}
	if(max > yRange[1]){ yRange[1] = max*1.05;}

	trace.y = this.data.humidity;
	layout.yaxis.title = 'Humidity [%]';
	layout.yaxis.range = yRange;
	Plotly.newPlot( document.getElementById('graph_humidity'), [trace], layout );
	
	max = Math.max(...this.data.pressure);
	min = Math.min(...this.data.pressure);
	mean = (max + min) / 2;
	deviation = mean / 20;
	yRange = [mean-deviation, mean+deviation];
	if(min < yRange[0]){ yRange[0] = min*0.95;}
	if(max > yRange[1]){ yRange[1] = max*1.05;}
	
	trace.y = this.data.pressure;
	layout.yaxis.title = 'Pressure [Pa]';
	layout.yaxis.range = yRange;
	Plotly.newPlot( document.getElementById('graph_pressure'), [trace], layout );
};

/*
window.onload = function() {
	// var device2 = new DeviceGeneric(88,"typ");
	// var device2 = new DeviceBME280(88);
	// console.log("id is " + device.id + "\n");
	// console.log("id2 is " + device2.id + "\n");
	//console.log(device.getHtmlDeviceInfo());
	// device.showDevice();
	device.createDevicePanel();
}
*/
