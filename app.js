const express = require('express');
const { spawn } = require('child_process');
const app = express();
const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 8080 });
app.use(express.static('public'));
app.use(express.urlencoded({ extended: true }));
app.set('view engine', 'ejs');
app.use(express.json());
const port = 80;



app.get('/', (req, res) => {
    res.render('index');
});

let cacheSimulatorProcess;
let output = '';

function initCacheSimulatorProcess() {
    if (!cacheSimulatorProcess) {
        cacheSimulatorProcess = spawn('./cacheSimulator/testCacheSim');
        
        cacheSimulatorProcess.stdout.on('data', (data) => {
            console.log(`C++ program output: ${data.toString()}`);
            output = data.toString();
        });
        
        cacheSimulatorProcess.stderr.on('data', (data) => {
            console.error(`C++ program error: ${data.toString()}`);
        });
        
        cacheSimulatorProcess.on('close', (code) => {
            console.log(`C++ program exited with code ${code}`);
            cacheSimulatorProcess = null; 
        });
    }
}

initCacheSimulatorProcess();

wss.on('connection', function connection(ws) {
    console.log('Client connected');
  
    cacheSimulatorProcess.stdout.on('data', (data) => {
      ws.send(data.toString()); 
    });
  
    cacheSimulatorProcess.on('close', (code) => {
      ws.send('C++ program ended'); 
      ws.send(output);
    });
    
    ws.on('message', function incoming(message) {
      console.log('received: %s', message);
      cacheSimulatorProcess.stdin.write(message + "\n"); 
    });
  });

app.post('/process', (req, res) => {
    const { input } = req.body;
    
    if (cacheSimulatorProcess) {
        cacheSimulatorProcess.stdin.write(input + "\n");
        res.json({ message: "Input sent to C++ program" });
    } else {
        res.status(500).json({ error: "C++ program is not running" });
        initCacheSimulatorProcess(); 
    }
});

app.listen(port,'0.0.0.0', () => {
    console.log(`Server listening at http://0.0.0.0:${port}`);
});
