var SerialPort  = require('serialport2').SerialPort;
var portName;

if (process.platform == 'win32')
  portName = 'COM4';
else
  portName = '/dev/ttyUSB0';

var sp = new SerialPort(); // instantiate the serial port.

sp.open(portName, { // portName is instatiated to be COM3, replace as necessary
   baudRate: 9600, // this is synced to what was set for the Arduino Code
   dataBits: 8, // this is the default for Arduino serial communication
   parity: 'none', // this is the default for Arduino serial communication
   stopBits: 1, // this is the default for Arduino serial communication
   flowControl: false // this is the default for Arduino serial communication
});

var io = require('socket.io').listen(6969); // server listens for socket.io communication at port 8000
io.set('log level', 1); // disables debugging. this is optional. you may remove it if desired.

var clickb1 = 0;
var clickb2 = 0;
var clickb3 = 0;

// Iniciamos la conexión.
io.sockets.on('connection', function (socket) {
  socket.emit('connected');
  //Permanecemos a la escucha del evento clickb1
  socket.on('clickb1', function(){
    if (clickb1) {
      sp.write("1l\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    } else {
      sp.write("1h\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    }
    clickb1 = !clickb1;
    //Emitimos el evento que dira que hemos recibido el del boton1
    io.sockets.emit('sclickb1', clickb1);
  });
  //Permanecemos a la escucha del evento clickb2
  socket.on('clickb2', function(){
    if (clickb2) {
      sp.write("2l\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    } else {
      sp.write("2h\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    }
    clickb2 = !clickb2;
    //Emitimos el evento que dira que hemos recibido el del boton1
    io.sockets.emit('sclickb2', clickb2);
  });
  //Permanecemos a la escucha del evento clickb3
  socket.on('clickb3', function(){
    if (clickb3) {
      sp.write("3l\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    } else {
      sp.write("3h\r", function (err, bytesWritten) {
        console.log('bytes written:', bytesWritten);
      });
    }
    clickb3 = !clickb3;
    //Emitimos el evento que dira que hemos recibido el del boton1
    io.sockets.emit('sclickb3', clickb3);
  });
  // Si socket.io recibe el evento message desde el cliente browser Navegador Web 
  // this call back will be executed.
  socket.on('message', function (msg) {
    console.log(msg);
  });
  // Si un web browser se desconecta from Socket.IO then this callback is called.
  socket.on('disconnect', function () {
    console.log('disconnected');
  });
});

var cleanData = ''; // En esta variable se almacena la trama completa JSON
var readData = '';  // En esta variable se almacena el buffer de entrada 
var serialData = '';
sp.on('data', function (data) { // call back when data is received
    readData += data.toString(); // append data to buffer
    // Si los caracteres '{' y '\r' se encuentran en el buffer se aislara lo que hay en el medio
    // como datos limpios. A continuación, se desactiva el buffer.
    if (readData.indexOf('\r') >= 0 && readData.indexOf('{') >= 0) {
        cleanData = readData.substring(readData.indexOf('{') + 0, readData.indexOf('\r'));
        serialData = JSON.parse(cleanData);
        console.log(cleanData);
        readData = '';
        io.sockets.emit('message1', serialData.x);
        io.sockets.emit('message2', serialData.y);
        io.sockets.emit('message3', serialData.z);
        serialData = '';
	}
});
