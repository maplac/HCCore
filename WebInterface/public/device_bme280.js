
var DeviceBME280 = {
	
	device : {},
	
	createDevice : function (selectedDevice){
		device = selectedDevice;
		var code = "";
		code += '<h1 id="device_panel_name"></h1>';
		code += '<span>ID: </span><span id="device_panel_id">'+device.id+'</span><br>';
		code += '<span>Name: </span><span id="device_panel_name">'+device.name+'</span><br>';
		code += '<span>Type: </span><span id="device_panel_type">'+device.type+'</span><br>';
		code += '<span>Description: </span><span id="device_panel_description">'+device.description+'</span><br>';
		code += '<span>Status: </span><span id="device_panel_status">'+device.status+'</span><br>';
		code += '<span>Last connected: </span><span id="device_panel_last_connected">'+device.lastConnected+'</span><br>';
		code += '<br>';
		code += '<span id="device_temperature" class="big_text">'+device.temperature + ' &degC'+'</span>'
		code += '<span id="device_humidity" class="big_text" style="padding-left: 50px">'+device.humidity + ' %'+'</span>'
		code += '<span id="device_pressure" class="big_text" style="padding-left: 50px">'+device.pressure + ' Pa'+'</span><br>'
		code += '<div id="graph_temperature" style="width:100%;height:400px;margin-top:10px"></div>'
		code += '<div id="graph_humidity" style="width:100%;height:400px;margin-top:10px"></div>'
		code += '<div id="graph_pressure" style="width:100%;height:400px;margin-top:10px"></div>'
		device.temperature = [];
		device.humidity = [];
		device.pressure = [];
		device.time = [];
		return code;
	},
	
	newDataReceived : function (msg){
		document.getElementById('device_temperature').innerHTML = (msg.data.temperature)/10 + "&degC";
		document.getElementById('device_humidity').innerHTML = msg.data.humidity + "%";
		document.getElementById('device_pressure').innerHTML = msg.data.pressure + " Pa";
		document.getElementById('device_panel_last_connected').innerHTML = msg.lastConnected;
		
		if (device.time.length > 15){
			device.time.shift();
			device.temperature.shift();
			device.humidity.shift();
			device.pressure.shift();
		}
		device.time.push(msg.data.time);
		device.temperature.push((msg.data.temperature)/10);
		device.humidity.push(msg.data.humidity);
		device.pressure.push(msg.data.pressure);
		//console.log(device.temperature);
		DeviceBME280.updateGraphs();
	},
	
	updateGraphs : function (){
		var trace1 = {	
			x: device.time,
			y: device.temperature,
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
				side: 'right'
			}
			
			
		};
		Plotly.newPlot( document.getElementById('graph_temperature'), [trace1], layout1 );
		
		var trace2 = {	
			x: device.time,
			y: device.humidity,
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
				side: 'right'
			}
			
		};
		Plotly.newPlot( document.getElementById('graph_humidity'), data2, layout2 );
		
		var trace3 = {	
			x: device.time,
			y: device.pressure,
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
				side: 'right'
			}
			
		};
		Plotly.newPlot( document.getElementById('graph_pressure'), data3, layout3 );
		
	}
}