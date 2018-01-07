"use strict";
// base class for all devices
function DeviceGeneric(id, type) {
	this.id = id;
	this.type = type;
	this.name = 'none';
	this.description = 'none';
	this.status = 'none';
	this.lastConnected = 'none';
        this.selectedView = 'none';
}
DeviceGeneric.prototype = Object.create(null);

DeviceGeneric.prototype.getHtmlDeviceInfo = function () {
	var code = '';
	code += '<h1 id="device_panel_name"></h1>';
	code += '<span>ID: </span><span id="device_panel_id">'+this.id+'</span><br>';
	code += '<span>Name: </span><span id="device_panel_name">'+this.name+'</span><br>';
	code += '<span>Type: </span><span id="device_panel_type">'+this.type+'</span><br>';
	code += '<span>Description: </span><span id="device_panel_description">'+this.description+'</span><br>';
	code += '<span>Status: </span><span id="device_panel_status">'+this.status+'</span><br>';
	code += '<span>Last connected: </span><span id="device_panel_last_connected">'+this.lastConnected+'</span><br>';
	code += '<br>';
	return code;
};


DeviceGeneric.prototype.createDevicePanel = function(msg) {
	document.getElementById('device_panel').innerHTML = this.getHtmlDeviceInfo();
}

DeviceGeneric.prototype.newDataReceived = function(msg) {
	// only some devices require this function
}

DeviceGeneric.prototype.dataBufferReceived = function(msg) {
	// only some devices require this function
}

DeviceGeneric.prototype.setParameters = function(device) {
	this.name = device.name;
	this.description = device.description;
	this.status = device.status;
	this.lastConnected = device.lastConnected;
}

DeviceGeneric.prototype.onButtonPress = function(type) {
}