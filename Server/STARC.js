"use strict";

var util = require("util");
var fs = require('fs');
var clc = require('cli-color');
var exec = require('child_process').exec;

String.prototype.contains = function(substr) {
	return (this.indexOf(substr) != -1);
};

function objExists(obj) {
	return (typeof obj != "undefined") ? true : false;
}

function Encrypter() {
	this.ikey = "TEST";
	this.skey = "TEST";
	this.ekey = "TEST";

	this.ASCII_BYPASS = 32;
	this.STREAM_SIZE = 256-this.ASCII_BYPASS;

	this.stream = {
		initial: this.generateStream("TEST")
	};
	//console.log(this.stream["initial"]);
}

Encrypter.prototype.generateStream = function(key) {
	var buff = 0;
	var keylen = key.length;
	var S = [];
	var T = [];
	var AB = this.ASCII_BYPASS;
	var SS = this.STREAM_SIZE;
	for(var i = 0; i < SS; ++i) {
		S[i] = i+AB;
		T[i] = key[(i+AB) % keylen];
	}
	var j = 0;
	for(var i = 0; i < SS; ++i) {
		j = (j + S[i] + T[i].charCodeAt()) % (SS);
		buff = S[i];
		S[i] = S[j];
		S[j] = buff;
	}
	return S;
}

Encrypter.prototype.crypt = function(imsg, type) {
	var buff = 0; var i = 0; var j = 0;
	var crypter = 0;
	if(!objExists(type)) { type = "initial"; }
	var SS = this.STREAM_SIZE;
	var S = this.stream[type].slice();
	var T = this.stream[type].slice();
	var new_message = "";
	var msg = imsg.slice();
	for (var k = 0; k < msg.length; ++k) {
		i = (i + 1) % (SS);
		j = (j + S[i]) % (SS);
		buff = T[i];
		T[i] = T[j];
		T[j] = buff;
		crypter = T[(T[i] + T[j]) % (SS)];
		var nchar = String.fromCharCode(msg[k].charCodeAt() ^ crypter);
		new_message += nchar;
	};
	return new_message;
};

var orange = clc.xterm(130);
var lightBlue = clc.xterm(110);
var CommunicationState = {
	DISCONNECTED: clc.red("DISCONNECTED"),
	ATTEMPTING_CONNECTION: orange("ATTEMPTING_CONNECTION"),
	CONNECTED: clc.green("CONNECTED"),
	ATTEMPTING_HANDSHAKE: clc.magenta("ATTEMPTING_HANDSHAKE"),
	ENCRYPTED_CONNECTION: clc.blue("ENCRYPTED_CONNECTION")
};
var CTRL = {
	STX		: '\x02', // Start of Transmission
	ETX		: '\x03', // End of Transmission
	ENQ		: '\x05', // Enquiry
	ACK		: '\x06', // Acknowledge
	CMD		: '\x12', // Send/Recieve CMD
	LOG 	: '\x14', // LOG message
	NACK	: '\x15', // Negative Ack
	US		: '\x1F'  // Unit Seperator
}

function EmbeddedDevice(dev) {
	if(typeof dev != "object") {
		return {};
	}
	var parent = this;
	this.connection = CommunicationState.DISCONNECTED;
	this.mac = lightBlue(dev["MAC"]);
	this.device = dev["Device"];
	this.interpreter = dev["Interpreter"];
	this.application = dev["Application"];

	this.checkInterval = false;

	this.ReadStream = undefined;
	this.WriteStream = undefined;
	this.serialbuffer = "";
	this.serialtext = "";

	this.timeout = 15000;
	this.retryTime = 60000;

	//this.encrypter = new Encrypter();

	this.onDataTransmission = function (data) {
		//console.log(data);
		if(parent.connection == CommunicationState.ATTEMPTING_CONNECTION &&
			data.contains(CTRL.ACK)) {
			parent.connecting = false;
			parent.connection = CommunicationState.CONNECTED;
			console.log(parent.mac+" is "+parent.connection);
			return;
		}

		parent.serialbuffer += data;

		if(parent.serialbuffer.contains(CTRL.ETX)) {
			//var encrypted_text = parent.serialbuffer.replace(CTRL.ETX, "");
			var re = new RegExp("["+CTRL.ETX+CTRL.STX+"]", 'g');
			parent.serialtext = parent.serialbuffer.replace(re, "");
			//parent.serialtext = parent.encrypter.crypt(encrypted_text);
			if(parent.serialtext.contains(CTRL.LOG)) {
				re = new RegExp("["+CTRL.LOG+"]", 'g');
				parent.serialtext = parent.serialtext.replace(re, "");
 				console.log("[LOG] "+parent.mac+"  "+parent.serialtext);
			} else if(parent.serialtext.contains(CTRL.CMD)) {
				re = new RegExp("["+CTRL.CMD+"]", 'g');
				parent.serialtext = parent.serialtext.replace(re, "");
				console.log("[CMD] "+parent.mac+"  "+parent.serialtext);
				var command = parent.interpreter+" ";
					command += parent.application+" ";
					command += "\""+parent.serialtext+"\"";
				var child = exec(command, function(error, stdout, stderr) {
					console.log(command+" -> "+stdout);
					parent.send(stdout);
					parent.check = false;
				});
			} else { }
			parent.serialbuffer = "";
		}
	};
	this.onClose = function () {
		console.log(parent.mac+" Connection Closed");
		parent.connection = CommunicationState.DISCONNECTED;
		parent.WriteStream.end();
		parent.connect();
	};
	this.onWError = function (err) {
		console.log(parent.mac+" Connection Write Error, Host may be down. Connection Closed.");
		parent.connection = CommunicationState.DISCONNECTED;
		parent.WriteStream.end();
	};
	this.onRError = function (err) {
		console.log(parent.mac+" Connection Read Error, Host may be down. Connection Closed.");
		parent.connection = CommunicationState.DISCONNECTED;
		parent.WriteStream.end();
		parent.connect();
	};

	this.connect();
}
EmbeddedDevice.prototype.send = function(msg) {
	this.WriteStream.write(CTRL.STX+msg+CTRL.ETX);
};
EmbeddedDevice.prototype.connect = function() {
	var parent = this;

	if(this.connecting == true) { return; }

	this.connection = CommunicationState.ATTEMPTING_CONNECTION;
	this.connecting = true;

	if(objExists(this.ReadStream)) {
		delete this.ReadStream;
		delete this.WriteStream;
		console.log("Attempting to reconnect to "+this.mac);
	} else {
		console.log("Attempting to connect to "+this.mac);
	}

	this.WriteStream = fs.createWriteStream(this.device);
	this.WriteStream.on('error', this.onWError);

	this.ReadStream = fs.createReadStream(this.device);
	this.ReadStream.setEncoding('ascii');
	this.ReadStream.on('data', this.onDataTransmission);
	this.ReadStream.on('close', this.onClose);
	this.ReadStream.on('error', this.onRError);

	this.send(CTRL.ENQ);

	setTimeout(function() {
		parent.connecting = false;
		if(parent.connection == CommunicationState.DISCONNECTED) {
			delete parent.ReadStream;
			delete parent.WriteStream;
			console.log(parent.mac+" is "+CommunicationState.DISCONNECTED);
			console.log("\tTrying again in 1 minute.");
			parent.connection = CommunicationState.DISCONNECTED;
			setTimeout(function() { parent.connect(); }, parent.retryTime);
		}
	}, this.timeout);
};

var embedded = [];

(function main() {
/*	var enc = new Encrypter();
	var msg = "[VOL]25";
	var emsg = enc.crypt(msg);
	var dmsg = enc.crypt(emsg);

	console.log(msg);
	console.log(emsg);
	console.log(dmsg);*/

	fs.readFile('devices.json', 'utf8', function (err, data) {
		if (err) { return console.log(err); }
		try {
			//data = JSON.minify(data);
			var devices = JSON.parse(data);
			// Start at one to skip dummy object
			for (var i = 0; i < devices.length; i++) {
				embedded[i] = new EmbeddedDevice(devices[i]);
				break;
			};
		} catch(e) {
			console.log(e);
			console.log(clc.red("Loading of 'devices.json' failed"));
			console.log(clc.red("OR"));
			console.log(clc.red("The JSON struction is not syntactically correct."));
			process.exit(1);
		}
	});
})();