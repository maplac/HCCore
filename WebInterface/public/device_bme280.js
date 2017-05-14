"use strict";
function DeviceBME280(id) {
	DeviceGeneric.call(this, id, 'BME280');
	this.temperature = 0;
	this.pressure = 0;
	this.humidity = 0;
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
	code += '<span id="device_panel_temperature" class="big_text">'+this.temperature + ' &degC'+'</span>'
	code += '<span id="device_panel_humidity" class="big_text" style="padding-left: 50px">'+this.humidity + ' %'+'</span>'
	code += '<span id="device_panel_pressure" class="big_text" style="padding-left: 50px">'+this.pressure + ' Pa'+'</span><br>'
	code += '<div id="graph_temperature" style="width:100%;height:400px;margin-top:10px"></div>'
	code += '<div id="graph_humidity" style="width:100%;height:400px;margin-top:10px"></div>'
	code += '<div id="graph_pressure" style="width:100%;height:400px;margin-top:10px"></div>'
	return code;
}

DeviceBME280.prototype.createDevicePanel = function(msg) {
	document.getElementById('device_panel').innerHTML = this.getHtmlDeviceInfo();
}

DeviceBME280.prototype.newDataReceived = function(msg) {
	this.temperature = (msg.data.temperature)/10;
	this.humidity = msg.data.humidity;
	this.pressure = msg.data.pressure;
	this.lastConnected = msg.lastConnected;
	document.getElementById('device_panel_temperature').innerHTML = (msg.data.temperature)/10 + "&degC";
	document.getElementById('device_panel_humidity').innerHTML = msg.data.humidity + "%";
	document.getElementById('device_panel_pressure').innerHTML = msg.data.pressure + " Pa";
	document.getElementById('device_panel_last_connected').innerHTML = msg.lastConnected;
}

DeviceBME280.prototype.dataBufferReceived = function(msg) {
	this.data = msg.data;
	for(var i = 0; i < this.data.time.length; i++){
		this.data.temperature[i] = this.data.temperature[i] / 10;
	}
	// console.log(this.data.temperature);
	this.updateGraphs();
}

DeviceBME280.prototype.updateGraphs = function() {
	var max = Math.max(...this.data.temperature);
		var min = Math.min(...this.data.temperature);
		var mean = (max + min) / 2;
		var deviation = mean / 20;
		var yRange = [mean-deviation, mean+deviation];
		if(min < yRange[0]){ yRange[0] = min*0.95;}
		if(max > yRange[1]){ yRange[1] = max*1.05;}
		// console.log(min+" "+max+" "+mean+" "+deviation);
		var trace1 = {	
			x: this.data.time,
			y: this.data.temperature,
			name: 'temperature',
			type: 'scatter',
			mode: 'lines+markers',
			line: {
				//shape: 'spline',
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
				//title: 'temperature [�C]',
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
			}
			
			
		};
		Plotly.newPlot( document.getElementById('graph_temperature'), [trace1], layout1 );
		
		max = Math.max(...this.data.humidity);
		min = Math.min(...this.data.humidity);
		mean = (max + min) / 2;
		deviation = mean / 20;
		yRange = [mean-deviation, mean+deviation];
		if(min < yRange[0]){ yRange[0] = min*0.95;}
		if(max > yRange[1]){ yRange[1] = max*1.05;}
		var trace2 = {	
			x: this.data.time,
			y: this.data.humidity,
			name: 'humidity',
			type: 'scatter',
			mode: 'lines+markers',
			line: {
				//shape: 'spline',
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
				side: 'right',
				range: yRange
			}
			
		};
		Plotly.newPlot( document.getElementById('graph_humidity'), data2, layout2 );
		
		max = Math.max(...this.data.pressure);
		min = Math.min(...this.data.pressure);
		mean = (max + min) / 2;
		deviation = mean / 20;
		yRange = [mean-deviation, mean+deviation];
		if(min < yRange[0]){ yRange[0] = min*0.95;}
		if(max > yRange[1]){ yRange[1] = max*1.05;}
		var trace3 = {	
			x: this.data.time,
			y: this.data.pressure,
			name: 'pressure',
			type: 'scatter',
			mode: 'lines+markers',
			line: {
				//shape: 'spline',
				color: '#ff7f0e'
			}
		};
		var data3 = [trace3];
		var layout3 = {
			title: 'pressure [Pa]',
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
				side: 'right',
				range: yRange
			}
			
		};
		Plotly.newPlot( document.getElementById('graph_pressure'), data3, layout3 );
}

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
