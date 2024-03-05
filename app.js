const express = require('express');
const { exec } = require('child_process');
const app = express();
const port = 3000;

app.get('/simulate', (req, res) => {
    // Get input from query parameters, for example: ?input=yourInput
    const input = req.query.input;

    // Call your C++ program, assuming it's called cache_simulator
    exec(`./cache_simulator ${input}`, (error, stdout, stderr) => {
        if (error) {
            console.error(`Execution error: ${error}`);
            return res.status(500).send('Internal Server Error');
        }
        res.send(`Simulator output: ${stdout}`);
    });
});

app.listen(port, () => {
    console.log(`Server running at http://localhost:${port}`);
});
