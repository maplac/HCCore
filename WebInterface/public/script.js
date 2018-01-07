//"use strict";
var deviceList;
var clientId = -1;
var ID_BROADCAST = 255;

var socket = io.connect();
var device_list;
var device_panel;
var selected_device_in_list = -1;
var selectedDevice = new DeviceGeneric(-1, 'none');


window.onload = function () {

    device_list = document.getElementById('device_list');
    device_panel = document.getElementById('device_panel');

    socket.on('onNewClientConnects', function (data) {
        clientId = data.id;
        console.log('this client id = ' + clientId);
        var msg = {};
        msg.desId = ID_BROADCAST;
        msg.srcId = clientId;
        msg.type = "pullAllDevices";
        socket.emit('message', msg);
    });
    /*
     graph = document.getElementById('graph');
     graph2 = document.getElementById('graph2');
     tempe = document.getElementById('device_temperature');
     humi = document.getElementById('device_humidity');
     */
};

// when a user selects a device in the menu
function onClickDeviceSelect(button) {

    // parse device id from button name
    var splitted = button.id.split('_');
    var id = parseInt(splitted[1]);

    selectedDevice.id = id;

    // remove selected layout from previous device
    if (typeof selected_device_in_list !== 'undefined') {
        selected_device_in_list.className = "device_in_list unselected";
    }

    // set selected layout to the current device
    selected_device_in_list = button;
    selected_device_in_list.className = "device_in_list selected";

    var msg = {};
    msg.desId = id;
    msg.srcId = clientId;
    msg.type = "pullDevice";
    socket.emit('message', msg);

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
socket.on('message', function (msg) {
    // if (msg.desId !== clientId || msg.desId !== ID_BROADCAST){
    // return
    // }
    //console.log("Received message:");
    //console.log(msg);
    switch (msg.type) {
        case "pushAllDevices":
            var code = "";
            var d;
            for (var i = 0; i < msg.devices.length; i++) {
                d = msg.devices[i];
                code = code + '<li class="device_in_list unselected" id="device_' + d.id + '" onclick="onClickDeviceSelect(this)">' + d.name + '</li>';
            }
            device_list.innerHTML = code;
            if (msg.devices.length > 0) {
                var id = msg.devices[0].id;
                var button = {};
                button.id = 'button_' + id;
                onClickDeviceSelect(button);
            }
            break;
        case "pushDevice":
            var device = msg.device;
            if (device.id === selectedDevice.id) {
                switch (device.type) {
                    case "switchOnOff" :

                        break;
                    case "BME280":
                        selectedDevice = new DeviceBME280(device.id);
                        var msg = {};
                        msg.desId = selectedDevice.id;
                        msg.srcId = clientId;
                        msg.type = "pullDataBuffer";
                        socket.emit('message', msg);
                        break;
                    default :
                        //device_panel.innerHTML = DeviceDHT22.createDevice(selectedDevice);
                        console.log("msg pushDevice: unknown type");
                }
                selectedDevice.setParameters(device);
                selectedDevice.createDevicePanel();
            }
            break;
        case "pushNewData":
            if (msg.srcId === selectedDevice.id) {
                selectedDevice.newDataReceived(msg);
                if (selectedDevice.selectedView === "day") {
                    var msg = {};
                    msg.desId = selectedDevice.id;
                    msg.srcId = clientId;
                    msg.type = "pullDataBuffer";
                    msg.subType = "day";
                    socket.emit('message', msg);
                }
            }
            break;
        case "pushDataBuffer":
            if (msg.srcId === selectedDevice.id) {
                selectedDevice.dataBufferReceived(msg);
            }
            break;
        case "test":
            /*
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
             */
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



