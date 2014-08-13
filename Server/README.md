Requires Node.JS to run!


How the 'devices.json' file works

{
	// MAC address of communication device
	"MAC" : "ff:ff:ff:ff:ff:ff",
	// Path to application file
	"Application" : "path/to/program.[py,rb,class,js,sh,etc...]",
	// Program to run application file. './' means native execute
	"Interpreter" : "[.\/,python,java,ruby,node,bash,etc...]",
	// Description of device
	"Description" : "This is a sample document for an embedded device.",
	// Path to Radio Frequency Device
	"Device" : "/dev/rfcomm99"
},