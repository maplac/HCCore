
var DeviceSwitchOnOff = {
	
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
		code += '<button id="button_on_off" class="button_onoff ';
		if(device.value === 1){
			code +='on';
		}else{
			code +='off';
		}
		code += '" type="button" onclick="DeviceSwitchOnOff.onClickButtonOnOff()">Toggle</button></br>';
		return code;
	},
	
	
	onClickButtonOnOff : function (){
		
		var msg = {};
		msg.desId = device.id;
		msg.srcId = clientId;
		msg.type = "setParameter";
		msg.parameter = {};
		msg.parameter.value = device.value == 1 ? 0 : 1;
		console.log("Send message:");
		console.log(msg);
		socket.emit('message', msg);
	},
	
	updateParameters : function (newParam){
		var parameters = Object.getOwnPropertyNames(newParam);
		for(i=0; i<parameters.length; i++){
			var param = parameters[i];
			selectedDevice[param]=newParam[param];
			if(param == "value"){
				if(selectedDevice.value === 0){
					document.getElementById('button_on_off').setAttribute("class", "button_onoff off");
				}else{
					document.getElementById('button_on_off').setAttribute("class", "button_onoff on");
				}
				
			}else{
				document.getElementById('device_panel_' + param).textContent = newParam[param];
			}
		}
	}
}